#pragma once

#include "ExceptionHandler.h"
#include "function.h"
#include <functional>
#include "JSRef.h"
#include "references.h"
#include "Registry.h"
#include "ResourceHandler.h"
#include <string>
#include <tuple>
#include "util.h"
#include <vector>

#ifdef HAS_REF_QUALIFIERS
# undef HAS_REF_QUALIFIERS
# undef REF_QUAL_LVALUE
#endif

#if defined(__clang__)
# if __has_feature(cxx_reference_qualified_functions)
#  define HAS_REF_QUALIFIERS
# endif
#elif defined(__GNUC__)
# define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
# if GCC_VERSION >= 40801
#  define HAS_REF_QUALIFIERS
# endif
#elif defined(_MSC_VER)
# if _MSC_VER >= 1900 // since MSVS-14 CTP1
#  define HAS_REF_QUALIFIERS
# endif
#endif

#if defined(HAS_REF_QUALIFIERS)
# define REF_QUAL_LVALUE &
#else
# define REF_QUAL_LVALUE
#endif

namespace seljs {
class State;
class Selector {
    friend class State;
private:
    duk_context *_state;
    Registry *_registry;
    ExceptionHandler *_exception_handler;
    std::string _name;

	// Root reference, if not global object
	JSRef _root;

    // Traverses the structure up to this element
    std::vector<JSRef> _traversal;

    // Key of the value to act upon.
    JSRef _key;

    std::vector<JSRef> _functor_arguments;

    // Functor is activated when the () operator is invoked.
    mutable  MovingFlag _functor_active;

    Selector(duk_context *s, Registry &r, ExceptionHandler &eh, const std::string &name,
             std::vector<JSRef> traversal, JSRef key)
        : _state(s), _registry(&r), _exception_handler(&eh), _name(name), _traversal(traversal),
          _key(key) {}

    Selector(duk_context *s, Registry &r, ExceptionHandler &eh, const std::string& name)
        : _state(s), _registry(&r), _exception_handler(&eh), _name(name),
          _key(make_Ref(s, name)) {}

	Selector(duk_context *s, Registry &r, ExceptionHandler &eh, JSRef root)
		: _state(s), _registry(&r), _exception_handler(&eh), _name(), 
		_key(), _root(root) {}

    void _get(JSRef r) const {
        r.Push(_state);
		duk_get_prop(_state, -2);
        duk_remove(_state, -2);
    }

    // Pushes this element to the stack
    void _get() const {
		if (_key.isRef())
			_get(_key);
    }

    // Sets this element from a function that pushes a value to the
    // stack.
    template<typename PushFunction>
    void _put(PushFunction fun) const {
        _key.Push(_state);
        fun();
		duk_put_prop(_state, -3);
        duk_pop(_state);
    }

    void _check_create_table() const {
        ResetStackOnScopeExit save(_state);
        _traverse();
        _get();
        if (duk_is_object(_state, -1) == 0 ) { // not table
            duk_pop(_state); // flush the stack
            auto put = [this]() {
                duk_push_object(_state);
            };
            _traverse();
            _put(put);
        }
    }

    void _traverse() const {
		if (_root.isRef())
			_root.Push(_state);
		else
			duk_push_global_object(_state);
        for (auto &key : _traversal) {
			_get(key);
        }
    }

    template <typename Fun>
    void _evaluate_store(Fun&& push) const {
        ResetStackOnScopeExit save(_state);
        _traverse();
        _put(std::forward<Fun>(push));
    }

    void _evaluate_retrieve(int num_results) const {
        _traverse();
        _get();
        _evaluate_function_call(num_results);
    }

    void _evaluate_function_call(int num_ret) const {
        if(!_functor_active) return;
        _functor_active = false;
        // call js function with error handler
        for(auto const & arg : _functor_arguments) {
            arg.Push(_state);
        }
        auto const statusCode =
            duk_pcall(_state, _functor_arguments.size());

        if (statusCode != DUK_EXEC_SUCCESS) {
			_exception_handler->Handle_top_of_stack(statusCode, _state);
        }
    }
public:

    Selector(const Selector &) = default;
    Selector(Selector &&) = default;
    Selector & operator=(const Selector &) = default;
    Selector & operator=(Selector &&) = default;

    ~Selector() noexcept(false) {
        // If there is a functor is not empty, execute it and collect no args
        if (_functor_active) {
            ResetStackOnScopeExit save(_state);
            _traverse();
            _get();
            if (std::uncaught_exception())
            {
                try {
                    _evaluate_function_call(0);
                } catch (...) {
                    // We are already unwinding, ignore further exceptions.
                    // As of C++17 consider std::uncaught_exceptions()
                }
            } else {
                _evaluate_function_call(0);
            }
        }
    }

    // Allow automatic casting when used in comparisons
    bool operator==(Selector &other) = delete;

    template <typename... Args>
    const Selector operator()(Args&&... args) const {
        Selector copy{*this};
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
        const auto eh = _exception_handler;
        copy._functor_arguments = make_Refs(_state, std::forward<Args>(args)...);
        copy._functor_active = true;
        return copy;
    }

    template <typename L>
    void operator=(L lambda) const {
        _evaluate_store([this, lambda]() {
            _registry->Register(lambda);
        });
    }

    void operator=(bool b) const {
        _evaluate_store([this, b]() {
            detail::_push(_state, b);
        });
    }

    void operator=(int i) const {
        _evaluate_store([this, i]() {
            detail::_push(_state, i);
        });
    }

    void operator=(unsigned int i) const {
        _evaluate_store([this, i]() {
            detail::_push(_state, i);
        });
    }

    void operator=(duk_double_t n) const {
        _evaluate_store([this, n]() {
            detail::_push(_state, n);
        });
    }

    void operator=(const std::string &s) const {
        _evaluate_store([this, s]() {
            detail::_push(_state, s);
        });
    }

    template <typename Ret, typename... Args>
    void operator=(std::function<Ret(Args...)> fun) {
        _evaluate_store([this, fun]() {
            _registry->Register(fun);
        });
    }

    template<typename T>
    void operator=(Reference<T> const & ref) {
        _evaluate_store([this, &ref]() {
            detail::_push(_state, ref);
        });
    }

    template<typename T>
    void operator=(Pointer<T> const & ptr) {
        _evaluate_store([this, &ptr]() {
            detail::_push(_state, ptr);
        });
    }

    template <typename Ret, typename... Args>
    void operator=(Ret (*fun)(Args...)) {
        _evaluate_store([this, fun]() {
            _registry->Register(fun);
        });
    }

    void operator=(const char *s) const {
        _evaluate_store([this, s]() {
            detail::_push(_state, s);
        });
    }

    template <typename T, typename... Funs>
    void SetObj(T &t, Funs... funs) {
        auto fun_tuple = std::make_tuple(std::forward<Funs>(funs)...);
        _evaluate_store([this, &t, &fun_tuple]() {
            _registry->Register(t, fun_tuple);
        });
    }

    template <typename T, typename... Args, typename... Funs>
    void SetClass(Funs... funs) {
        auto fun_tuple = std::make_tuple(std::forward<Funs>(funs)...);
        _evaluate_store([this, &fun_tuple]() {
            typename detail::_indices_builder<sizeof...(Funs)>::type d;
            _registry->RegisterClass<T, Args...>(_name, fun_tuple, d);
        });
    }

	template <typename T, typename Constructor, typename Destructor, typename... Funs>
	void SetClassCustom(Funs... funs) {
		auto fun_tuple = std::make_tuple(std::forward<Funs>(funs)...);
		_evaluate_store([this, &fun_tuple]() {
			typename detail::_indices_builder<sizeof...(Funs)>::type d;
			_registry->RegisterClassCustom<T, Constructor, Destructor>(_name, fun_tuple, d);
		});
	}

	template <typename T>
	void SetClassObj(T &t) {
		_evaluate_store([this, &t]() {
			_registry->RegisterClassObj(t);
		});
	}

    template <typename... Ret>
    std::tuple<Ret...> GetTuple() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(sizeof...(Ret));
        return detail::_get_n<Ret...>(_state);
    }

	JSRef asRef() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return std::move(JSRef(_state, duv_ref(_state)));
	}

	operator JSRef&() const {
		throw std::logic_error("Cannot use implicit operator for JSRef, use asRef()");
	}

    template<
        typename T,
        typename = typename std::enable_if<
            !detail::is_primitive<typename std::decay<T>::type>::value
        >::type
    >
    operator T&() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<T&>{}, _state);
    }

    template <typename T>
    operator T*() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<T*>{}, _state);
    }

    template <typename T>
    operator Reference<T>() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<Reference<T>>{}, _state);
    }

    template <typename T>
    operator Pointer<T>() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<Pointer<T>>{}, _state);
    }

	std::string type() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return std::string(duv_typename(_state, duk_get_type(_state, -1)));
	}

    operator bool() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<bool>{}, _state);
    }

	operator char() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return detail::_pop(detail::_id<char>{}, _state);
	}

	operator unsigned char() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return detail::_pop(detail::_id<unsigned char>{}, _state);
	}

    operator int() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<int>{}, _state);
    }

    operator unsigned int() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<unsigned int>{}, _state);
    }

    operator duk_double_t() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<duk_double_t>{}, _state);
    }

    operator std::string() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<std::string>{}, _state);
    }

    template <typename R, typename... Args>
    operator seljs::function<R(Args...)>() {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        auto ret = detail::_pop(detail::_id<seljs::function<R(Args...)>>{},
                                _state);
        ret._enable_exception_handler(_exception_handler);
        return ret;
    }

	bool isUndefined() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_undefined(_state, -1) != 0;
	}

	bool isNull() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_null(_state, -1) != 0;
	}

	bool isNullOrUndefined() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_null_or_undefined(_state, -1) != 0;
	}

	bool isCallable() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_callable(_state, -1) != 0;
	}

	bool isObject() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_object(_state, -1) != 0;
	}

	bool isArray() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_array(_state, -1) != 0;
	}

	bool isFunction() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_function(_state, -1) != 0;
	}

	bool hasKey(const std::string& name) const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_is_object(_state, -1) && duk_has_prop_string(_state, -1, name.c_str());
	}

	int length() const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		return duk_get_length(_state, -1);
	}

	std::vector<std::string> keyList(duk_uint_t enum_flags = 0) const {
		ResetStackOnScopeExit save(_state);
		_evaluate_retrieve(1);
		std::vector<std::string> ret;
		duk_enum(_state, -1, enum_flags);
		while (duk_next(_state, -1, 0)) {
			ret.push_back(std::string(duk_get_string(_state, -1)));
			duk_pop(_state);
		}
		duk_pop(_state); // enum
		return ret;
	}

    // Chaining operators. If the selector is an rvalue, modify in
    // place. Otherwise, create a new Selector and return it.
#ifdef HAS_REF_QUALIFIERS
    Selector&& operator[](const std::string& name) && {
        _name += std::string(".") + name;
        _check_create_table();
		if (_key.isRef())
			_traversal.push_back(_key);
        _key = make_Ref(_state, name);
        return std::move(*this);
    }
    Selector&& operator[](const char* name) && {
        return std::move(*this)[std::string{name}];
    }
    Selector&& operator[](const int index) && {
        _name += std::string(".") + std::to_string(index);
        _check_create_table();
		if (_key.isRef())
			_traversal.push_back(_key);
        _key = make_Ref(_state, index);
        return std::move(*this);
    }
#endif // HAS_REF_QUALIFIERS
    Selector operator[](const std::string& name) const REF_QUAL_LVALUE {
        auto n = _name + "." + name;
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_key);
        return Selector{_state, *_registry, *_exception_handler, n, traversal, make_Ref(_state, name)};
    }
    Selector operator[](const char* name) const REF_QUAL_LVALUE {
        return (*this)[std::string{name}];
    }
    Selector operator[](const int index) const REF_QUAL_LVALUE {
        auto name = _name + "." + std::to_string(index);
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_key);
        return Selector{_state, *_registry, *_exception_handler, name, traversal, make_Ref(_state, index)};
    }

    friend bool operator==(const Selector &, const char *);

    friend bool operator==(const char *, const Selector &);

private:
    std::string ToString() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<std::string>{}, _state);
    }
};

inline bool operator==(const Selector &s, const char *c) {
    return std::string{c} == s.ToString();
}

inline bool operator==(const char *c, const Selector &s) {
    return std::string{c} == s.ToString();
}

template <typename T>
inline bool operator==(const Selector &s, T&& t) {
    return T(s) == t;
}

template <typename T>
inline bool operator==(T &&t, const Selector &s) {
    return T(s) == t;
}

}

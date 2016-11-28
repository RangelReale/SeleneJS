#pragma once

#include "ExceptionTypes.h"
#include <string>
#include "traits.h"
#include <type_traits>
#include "MetatableRegistry.h"

#include <duktape.h>

/* The purpose of this header is to handle pushing and retrieving
 * primitives from the stack
 */

namespace seljs {

namespace detail {

template <typename T>
struct is_primitive {
    static constexpr bool value = false;
};
template <>
struct is_primitive<int> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<unsigned int> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<bool> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<duk_double_t> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<std::string> {
    static constexpr bool value = true;
};

template<typename T>
using decay_primitive =
    typename std::conditional<
        is_primitive<typename std::decay<T>::type>::value,
        typename std::decay<T>::type,
        T
    >::type;

/* getters */
template <typename T>
inline T* _get(_id<T*>, duk_context *l, const int index) {
    if(MetatableRegistry::IsType(l, typeid(T), index)) {
		T* ret = (T*)duv_get_obj_ptr(l, index);
		return ret;
    }
    return nullptr;
}

template <typename T>
inline T& _get(_id<T&>, duk_context *l, const int index) {
    if(!MetatableRegistry::IsType(l, typeid(T), index)) {
        throw TypeError{
            MetatableRegistry::GetTypeName(l, typeid(T)),
            MetatableRegistry::GetTypeName(l, index)
        };
    }

    T *ptr = (T*)duv_get_obj_ptr(l, index);
    if(ptr == nullptr) {
        throw TypeError{MetatableRegistry::GetTypeName(l, typeid(T))};
    }
    return *ptr;
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value, T
>::type
_get(_id<T>, duk_context *l, const int index) {
    return _get(_id<T&>{}, l, index);
}

inline bool _get(_id<bool>, duk_context *l, const int index) {
    return duk_to_boolean(l, index) != 0;
}

inline int _get(_id<int>, duk_context *l, const int index) {
    return static_cast<int>(duk_to_int(l, index));
}

inline unsigned int _get(_id<unsigned int>, duk_context *l, const int index) {
    return static_cast<unsigned>(duk_to_uint(l, index));
}

inline duk_double_t _get(_id<duk_double_t>, duk_context *l, const int index) {
    return duk_to_number(l, index);
}

inline std::string _get(_id<std::string>, duk_context *l, const int index) {
    size_t size;
    const char *buff = duk_to_lstring(l, index, &size);
    return std::string{buff, size};
}

using _js_check_get = void (*)(duk_context *l, int index);
// Throw this on conversion errors to prevent long jumps caused in Lua from
// bypassing destructors. The outermost function can then call checkd_get(index)
// in a context where a long jump is safe.
// This way we let Lua generate the error message and use proper stack
// unwinding.
struct GetParameterFromJSTypeError {
    _js_check_get checked_get;
    int index;
};

template <typename T>
inline T* _check_get(_id<T*>, duk_context *l, const int index) {
    MetatableRegistry::CheckType(l, typeid(T), index);
	return (T*)duv_require_obj_ptr(l, index);
}

template <typename T>
inline T& _check_get(_id<T&>, duk_context *l, const int index) {
    static_assert(!is_primitive<T>::value,
                  "Reference types must not be primitives.");

    T *ptr = _check_get(_id<T*>{}, l, index);

    if(ptr == nullptr) {
        throw GetUserdataParameterFromJSTypeError{
            MetatableRegistry::GetTypeName(l, typeid(T)),
            index
        };
    }

    return *ptr;
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value, T
>::type
_check_get(_id<T>, duk_context *l, const int index) {
    return _check_get(_id<T&>{}, l, index);
}

template <typename T>
inline T _check_get(_id<T&&>, duk_context *l, const int index) {
    return _check_get(_id<T>{}, l, index);
}


inline int _check_get(_id<int>, duk_context *l, const int index) {
	//return static_cast<int>(duk_to_int(l, index));
	return static_cast<int>(duk_require_int(l, index));
}

inline unsigned int _check_get(_id<unsigned int>, duk_context *l, const int index) {
	//return static_cast<unsigned int>(duk_to_uint(l, index));
	return static_cast<unsigned int>(duk_require_uint(l, index));
}

inline duk_double_t _check_get(_id<duk_double_t>, duk_context *l, const int index) {
	//return static_cast<duk_double_t>(duk_to_number(l, index));
	return static_cast<duk_double_t>(duk_require_number(l, index));
}

inline bool _check_get(_id<bool>, duk_context *l, const int index) {
    //return duk_to_boolean(l, index) != 0;
	return duk_require_boolean(l, index) != 0;
}

inline std::string _check_get(_id<std::string>, duk_context *l, const int index) {
    size_t size = 0;
	/*
    char const * buff = duk_to_lstring(l, index, &size);
    if(buff == nullptr) {
        throw GetParameterFromJSTypeError{
            [](duk_context *l, int index){duk_safe_to_string(l, index);},
            index
        };
    }
	*/
	char const * buff = duk_require_lstring(l, index, &size);
	return std::string{buff, size};
}

// Worker type-trait struct to _get_n
// Getting multiple elements returns a tuple
template <typename... Ts>
struct _get_n_impl {
    using type =  std::tuple<Ts...>;

    template <std::size_t... N>
    static type worker(duk_context *l,
                       _indices<N...>) {
        return std::make_tuple(_get(_id<Ts>{}, l, N + 1)...);
    }

    static type apply(duk_context *l) {
        return worker(l, typename _indices_builder<sizeof...(Ts)>::type());
    }
};

// Getting nothing returns void
template <>
struct _get_n_impl<> {
    using type = void;
    static type apply(duk_context *) {}
};

// Getting one element returns an unboxed value
template <typename T>
struct _get_n_impl<T> {
    using type = T;
    static type apply(duk_context *l) {
        return _get(_id<T>{}, l, -1);
    }
};

template <typename... T>
typename _get_n_impl<T...>::type _get_n(duk_context *l) {
    return _get_n_impl<T...>::apply(l);
}

template <typename T>
T _pop(_id<T> t, duk_context *l) {
    T ret =  _get(t, l, -1);
    duk_pop(l);
    return ret;
}

/* Setters */

inline void _push(duk_context *) {}

template <typename T>
inline void _push(duk_context *l, T* t) {
  if(t == nullptr) {
    duk_push_null(l);
  }
  else {
	duv_push_obj_ptr(l, t);
    MetatableRegistry::SetMetatable(l, typeid(T));
  }
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value
>::type
_push(duk_context *l, T& t) {
	duv_push_obj_ptr(l, &t);
	MetatableRegistry::SetMetatable(l, typeid(T));
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value
    && std::is_rvalue_reference<T&&>::value
>::type
_push(duk_context *l, T&& t) {
    if(!MetatableRegistry::IsRegisteredType(l, typeid(t)))
    {
        throw CopyUnregisteredType(typeid(t));
    }

	void *addr = static_cast<void*>(new T(std::forward<T>(t)));
	duv_push_obj_ptr(l, addr);
	MetatableRegistry::SetMetatable(l, typeid(T));
}

inline void _push(duk_context *l, bool b) {
    duk_push_boolean(l, b);
}

inline void _push(duk_context *l, int i) {
    duk_push_int(l, i);
}

inline void _push(duk_context *l, unsigned int u) {
    duk_push_uint(l, u);
}

inline void _push(duk_context *l, duk_double_t f) {
    duk_push_number(l, f);
}

inline void _push(duk_context *l, const std::string &s) {
    duk_push_lstring(l, s.c_str(), s.size());
}

inline void _push(duk_context *l, const char *s) {
    duk_push_string(l, s);
}

template <typename T>
inline void _set(duk_context *l, T &&value, const int index) {
    _push(l, std::forward<T>(value));
    duk_replace(l, index);
}

inline void _push_n(duk_context *) {}

template <typename T, typename... Rest>
inline void _push_n(duk_context *l, T &&value, Rest&&... rest) {
    _push(l, std::forward<T>(value));
    _push_n(l, std::forward<Rest>(rest)...);
}

template <typename... T, std::size_t... N>
inline void _push_dispatcher(duk_context *l,
                             const std::tuple<T...> &values,
                             _indices<N...>) {
    _push_n(l, std::get<N>(values)...);
}

inline void _push(duk_context *, std::tuple<>) {}

template <typename... T>
inline void _push(duk_context *l, const std::tuple<T...> &values) {
    constexpr int num_values = sizeof...(T);
    _push_dispatcher(l, values,
                     typename _indices_builder<num_values>::type());
}

template <typename... T>
inline void _push(duk_context *l, std::tuple<T...> &&values) {
    _push(l, const_cast<const std::tuple<T...> &>(values));
}

}
}

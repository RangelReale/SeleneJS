#pragma once

#include "BaseFun.h"
#include <string>
#include "lmetatable.h"

namespace seljs {

template <int N, typename T, typename Ret, typename... Args>
class ClassFun : public BaseFun {
private:
    using _fun_type = std::function<Ret(T*, Args...)>;
    _fun_type _fun;
    std::string _name;
    std::string _metatable_name;

    T *_get(duk_context *state) {
		// get obj from "this" bindind
		T *ret = (T *)duv_get_obj_ptr(state, -2);
        return ret;
    }

public:
    ClassFun(duk_context *l,
             const std::string &name,
             const std::string &metatable_name,
             Ret(*fun)(Args...))
        : ClassFun(l, name, _fun_type{fun}) {}

    ClassFun(duk_context *l,
             const std::string &name,
             const std::string &metatable_name,
             _fun_type fun)
        : _fun(fun), _name(name), _metatable_name(metatable_name) {
		duv_push_c_function_ptr(l, &detail::_js_dispatcher, DUK_VARARGS, (void *)static_cast<BaseFun *>(this));
		duk_put_prop_string(l, -2, name.c_str());
    }

    int Apply(duk_context *l) {
		// "this" is at index -2
        std::tuple<T*> t = std::make_tuple(_get(l));
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        std::tuple<T*, Args...> pack = std::tuple_cat(t, args);
        detail::_push(l, detail::_lift(_fun, pack));
        return N;
    }
};

template <typename T, typename... Args>
class ClassFun<0, T, void, Args...> : public BaseFun {
private:
    using _fun_type = std::function<void(T*, Args...)>;
    _fun_type _fun;
    std::string _name;
    std::string _metatable_name;

    T *_get(duk_context *state) {
		// get obj from "this" binding
		T *ret = (T *)duv_get_obj_ptr(state, -2);
		return ret;
    }

public:
    ClassFun(duk_context *l,
             const std::string &name,
             const std::string &metatable_name,
             void(*fun)(Args...))
        : ClassFun(l, name, metatable_name, _fun_type{fun}) {}

    ClassFun(duk_context *l,
             const std::string &name,
             const std::string &metatable_name,
             _fun_type fun)
        : _fun(fun), _name(name), _metatable_name(metatable_name) {
		duv_push_c_function_ptr(l, &detail::_js_dispatcher, DUK_VARARGS, (void *)static_cast<BaseFun *>(this));
		duk_put_prop_string(l, -2, name.c_str());
    }

    int Apply(duk_context *l) {
		// "this" is at index -2
		std::tuple<T*> t = std::make_tuple(_get(l));
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        std::tuple<T*, Args...> pack = std::tuple_cat(t, args);
        detail::_lift(_fun, pack);
        return 0;
    }
};
}

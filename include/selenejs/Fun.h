#pragma once

#include "BaseFun.h"
#include "primitives.h"
#include <string>
#include "metatable.h"

namespace seljs {
template <int N, typename Ret, typename... Args>
class Fun : public BaseFun {
private:
    using _fun_type = std::function<Ret(detail::decay_primitive<Args>...)>;
    _fun_type _fun;

public:
    Fun(duk_context *&l,
        _fun_type fun) : _fun(fun) {
		duv_push_c_function_ptr(l, &detail::_js_dispatcher, DUK_VARARGS, (void *)static_cast<BaseFun *>(this));
    }

    // Each application of a function receives a new Lua context so
    // this argument is necessary.
    int Apply(duk_context *l) override {
        std::tuple<detail::decay_primitive<Args>...> args =
            detail::_get_args<detail::decay_primitive<Args>...>(l);
        detail::_push(l, detail::_lift(_fun, args));
        return N;
    }

};

template <typename... Args>
class Fun<0, void, Args...> : public BaseFun {
private:
    using _fun_type = std::function<void(detail::decay_primitive<Args>...)>;
    _fun_type _fun;

public:
    Fun(duk_context *&l,
        _fun_type fun) : _fun(fun) {
		duv_push_c_function_ptr(l, &detail::_js_dispatcher, DUK_VARARGS, (void *)static_cast<BaseFun *>(this));
    }

    // Each application of a function receives a new Lua context so
    // this argument is necessary.
    int Apply(duk_context *l) {
        std::tuple<detail::decay_primitive<Args>...> args =
            detail::_get_args<detail::decay_primitive<Args>...>(l);
        detail::_lift(_fun, args);
        return 0;
    }
};
}

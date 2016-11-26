#pragma once

#include "function.h"
#include <exception>
#include "ExceptionHandler.h"
#include <functional>
#include "primitives.h"
#include <tuple>
#include "util.h"
#include "metatable.h"

namespace seljs {
struct BaseFun {
    virtual ~BaseFun() {}
    virtual int Apply(duk_context *state) = 0;
};

namespace detail {

inline duk_ret_t _js_dispatcher(duk_context *l) {
	duk_push_current_function(l);
    BaseFun *fun = (BaseFun *)duv_get_c_function_ptr(l, -1);
    _js_check_get raiseParameterConversionError = nullptr;
    const char * wrong_meta_table = nullptr;
    int erroneousParameterIndex = 0;
    try {
        return fun->Apply(l);
    } catch (GetParameterFromJSTypeError & e) {
        raiseParameterConversionError = e.checked_get;
        erroneousParameterIndex = e.index;
    } catch (GetUserdataParameterFromJSTypeError & e) {
        wrong_meta_table = duk_push_lstring(
            l, e.metatable_name.c_str(), e.metatable_name.length());
        erroneousParameterIndex = e.index;
    } catch (std::exception & e) {
        duk_push_string(l, e.what());
        Traceback(l);
        //store_current_exception(l, duk_to_string(l, -1));
    } catch (...) {
        duk_push_string(l, "<Unknown exception>");
        Traceback(l);
        //store_current_exception(l, duk_to_string(l, -1));
    }

    if(raiseParameterConversionError) {
        raiseParameterConversionError(l, erroneousParameterIndex);
    }
    else if(wrong_meta_table) {
		// POSSIBLE PROBLEM
        duvL_checkudata(l, erroneousParameterIndex, wrong_meta_table);
    }

	return DUK_RET_ERROR;
}

template <typename Ret, typename... Args, std::size_t... N>
inline Ret _lift(std::function<Ret(Args...)> fun,
                 std::tuple<Args...> args,
                 _indices<N...>) {
    return fun(std::get<N>(args)...);
}

template <typename Ret, typename... Args>
inline Ret _lift(std::function<Ret(Args...)> fun,
                 std::tuple<Args...> args) {
    return _lift(fun, args, typename _indices_builder<sizeof...(Args)>::type());
}


template <typename... T, std::size_t... N>
inline std::tuple<T...> _get_args(duk_context *state, _indices<N...>) {
    return std::tuple<T...>{_check_get(_id<T>{}, state, N)...};
}

template <typename... T>
inline std::tuple<T...> _get_args(duk_context *state) {
    constexpr std::size_t num_args = sizeof...(T);
    return _get_args<T...>(state, typename _indices_builder<num_args>::type());
}
}
}

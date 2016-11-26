#pragma once

#include "ExceptionHandler.h"
#include <iostream>
#include <utility>
#include "metatable.h"

#include <duktape.h>

namespace seljs {
inline std::ostream &operator<<(std::ostream &os, duk_context *l) {
    int top = duk_get_top(l);
    for (int i = 1; i <= top; ++i) {
        int t = duk_get_type(l, i);
        switch(t) {
        case DUK_TYPE_STRING:
            os << duk_to_string(l, i);
            break;
        case DUK_TYPE_BOOLEAN:
            os << (duk_to_boolean(l, i)!=0 ? "true" : "false");
            break;
        case DUK_TYPE_NUMBER:
            os << duk_to_number(l, i);
            break;
        default:
            os << duv_typename(l, t);
            break;
        }
        os << " ";
    }
    return os;
}

inline void _print() {
    std::cout << std::endl;
}

template <typename T, typename... Ts>
inline void _print(T arg, Ts... args) {
    std::cout << arg << ", ";
    _print(args...);
}

inline bool check(duk_context *L, int code) {
    if (code == 0) {
        return true;
    } else {
        std::cout << duk_safe_to_string(L, -1) << std::endl;
        return false;
    }
}

inline int Traceback(duk_context *L) {
    // Make nil and values not convertible to string human readable.
	/*
    const char* msg = "<not set>";
    if (!lua_isnil(L, -1)) {
        msg = lua_tostring(L, -1);
        if (!msg)
            msg = "<error object>";
    }
    lua_pushstring(L, msg);

    // call debug.traceback
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_pushvalue(L, -3);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
	*/
    return 1;
}

inline duk_ret_t ErrorHandler(duk_context *L) {
    if(test_stored_exception(L) != nullptr) {
        return 1;
    }

    return Traceback(L);
}

inline int SetErrorHandler(duk_context *L) {
	duk_push_c_function(L, &ErrorHandler, 1);
    return duk_get_top(L);
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}

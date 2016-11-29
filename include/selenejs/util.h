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

// gets message and possible traceback from Error object
inline std::string ErrorMessage(duk_context *L, duk_idx_t index) {
	std::string ret;
	// TODO: check if index is really an error object
	// gets the "stack" property of the Error object
	duk_get_prop_string(L, index, "stack");
	if (duk_is_string(L, -1)) {
		size_t size;
		const char *buff = duk_get_lstring(L, -1, &size);
		ret = std::string{ buff, size };
		duk_pop(L);
	}
	else 
	{
		duk_pop(L);
		// gets message from object using ToString()
		ret = duk_safe_to_string(L, index);
	}
	return ret;
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

inline duk_idx_t duv_push_c_function_ptr(duk_context *ctx, duk_c_function func, duk_idx_t nargs, void *ptr)
{
	duk_idx_t ret = duk_push_c_function(ctx, func, nargs);
	// push object into hidden property
	duk_push_pointer(ctx, ptr);
	// put into function property
	duk_put_prop_string(ctx, -2, "\xFF" "_func_obj");
	return ret;
}

inline void * duv_get_c_function_ptr(duk_context *ctx, duk_idx_t index)
{
	duk_get_prop_string(ctx, index, "\xFF" "_func_obj");
	void* ptr = duk_get_pointer(ctx, -1);
	duk_pop(ctx);
	return ptr;
}

inline void duv_push_obj_ptr(duk_context *ctx, void *ptr)
{
	duk_push_object(ctx);
	duk_push_pointer(ctx, ptr);
	duk_put_prop_string(ctx, -2, "\xFF" "_obj");
}

inline void *duv_get_obj_ptr(duk_context *ctx, duk_idx_t index)
{
	duk_get_prop_string(ctx, index, "\xFF" "_obj");
	void* ptr = duk_get_pointer(ctx, -1);
	duk_pop(ctx);
	return ptr;
}

inline void *duv_require_obj_ptr(duk_context *ctx, duk_idx_t index)
{
	duk_get_prop_string(ctx, index, "\xFF" "_obj");
	void* ptr = duk_require_pointer(ctx, -1);
	duk_pop(ctx);
	return ptr;
}

}

#pragma once

#include <duktape.h>

namespace seljs {

void duv_setmetatable(duk_context *ctx, duk_idx_t index)
{
	duk_put_prop_string(ctx, index, "\xFF" "_metatable");
}

int duv_getmetatable(duk_context *ctx, duk_idx_t index)
{
	if (duk_get_prop_string(ctx, index, "\xFF" "_metatable") == 0) {
		duk_pop(ctx); // undefined
		return 0;
	}
	return 1;
}

const char *duv_typename(duk_context *ctx, duk_int_t tp)
{
	switch (tp)
	{
	case DUK_TYPE_NONE: return "NONE";
	case DUK_TYPE_UNDEFINED: return "UNDEFINED";
	case DUK_TYPE_NULL: return "NULL";
	case DUK_TYPE_BOOLEAN: return "BOOLEAN";
	case DUK_TYPE_NUMBER: return "NUMBER";
	case DUK_TYPE_STRING: return "STRING";
	case DUK_TYPE_OBJECT: return "OBJECT";
	case DUK_TYPE_BUFFER: return "BUFFER";
	case DUK_TYPE_POINTER: return "POINTER";
	case DUK_TYPE_LIGHTFUNC: return "LIGHTFUNC";
	}
	return "NONE";
}

duk_idx_t duv_push_c_function_ptr(duk_context *ctx, duk_c_function func, duk_idx_t nargs, void *ptr)
{
	duk_idx_t ret = duk_push_c_function(ctx, func, nargs);
	// push object into hidden property
	duk_push_pointer(ctx, ptr);
	// put into function property
	duk_put_prop_string(ctx, -2, "\xFF" "_func_obj");
	return ret;
}

void * duv_get_c_function_ptr(duk_context *ctx, duk_idx_t index)
{
	duk_get_prop_string(ctx, index, "\xFF" "_func_obj");
	void* ptr = duk_get_pointer(ctx, -1);
	duk_pop(ctx);
	return ptr;
}

}
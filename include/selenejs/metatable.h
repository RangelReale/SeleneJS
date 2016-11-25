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


}
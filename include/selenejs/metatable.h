#pragma once

#include <duktape.h>

namespace seljs {

void duv_setmetatable(duk_context *ctx, duk_idx_t index)
{
	duk_set_prototype(ctx, index);
}

int duv_getmetatable(duk_context *ctx, duk_idx_t index)
{
	if (!duk_is_object(ctx, index))
		return 0;

	duk_get_prototype(ctx, index);
	int ret = duk_is_undefined(ctx, -1)?0:1;
	if (ret == 0) 
		duk_pop(ctx);
	return ret;
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
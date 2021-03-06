#pragma once

#include <duktape.h>

#include "metatable.h"

namespace seljs {

inline duk_int_t duvL_newmetatable(duk_context *ctx, const char *tname) {
	duk_push_heap_stash(ctx);
	duk_get_prop_string(ctx, -1, tname);
	if (duk_is_undefined(ctx, -1) == 0) {
		duk_remove(ctx, -2); // remove stash
		return 0;
	}
	duk_pop(ctx); // value
	duk_push_object(ctx);
	duk_dup(ctx, -1); // put one on the hash, return the other
	duk_put_prop_string(ctx, -3, tname);
	duk_remove(ctx, -2); // stash
	return 1;
}

inline void duvL_getmetatable(duk_context *ctx, const char *tname)
{
	duk_push_heap_stash(ctx);
	duk_get_prop_string(ctx, -1, tname);
	duk_remove(ctx, -2);
}

inline void duvL_setmetatable(duk_context *ctx, const char *tname)
{
	duvL_getmetatable(ctx, tname);
	duv_setmetatable(ctx, -2);
}

inline void *duvL_checkudata(duk_context *ctx, duk_idx_t index, const char *tname)
{
	return duk_to_pointer(ctx, index);
}

}
#pragma once

#include <duktape.h>

namespace seljs {

inline duk_int_t duv_newmetatable(duk_context *ctx, const char *tname) {
	duk_push_heap_stash(ctx);
	duk_get_prop_string(ctx, -1, tname);
	if (duk_is_undefined(ctx, -1) != 0) {
		duk_remove(ctx, -2); // remove stash
		return 0;
	}
	duk_pop(ctx); // value
	duk_push_object(ctx);
	duk_dup(ctx, -1);
	duk_put_prop_string(ctx, -3, tname);
	duk_remove(ctx, -3); // stash
	return 1;
}

}
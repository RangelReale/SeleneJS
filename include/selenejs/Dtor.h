#pragma once

#include "BaseFun.h"
#include "lmetatable.h"

namespace seljs {

template <typename T>
class Dtor : public BaseFun {
private:
    std::string _metatable_name;
public:
    Dtor(duk_context *l,
         const std::string &metatable_name)
        : _metatable_name(metatable_name) {
		duv_push_c_function_ptr(l, &detail::_js_dispatcher, DUK_VARARGS, (void *)static_cast<BaseFun *>(this));
		duk_set_finalizer(l, -2);
    }

    int Apply(duk_context *l) {
		T *t = (T *)duv_get_obj_ptr(l, 0);
        t->~T();
        return 0;
    }
};

template <typename T>
class DtorNull : public BaseFun {
private:
public:
	DtorNull(duk_context *l,
		const std::string &metatable_name)
	{
	}

	int Apply(duk_context *l) {
		return 0;
	}
};
}

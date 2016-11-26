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
        T *t = (T *)duvL_checkudata(l, 1, _metatable_name.c_str());
        t->~T();
        return 0;
    }
};
}

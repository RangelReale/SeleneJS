#pragma once

#include "BaseFun.h"
#include "lmetatable.h"

namespace seljs {

template <typename T, typename... Args>
class Ctor : public BaseFun {
private:
    using _ctor_type = std::function<void(duk_context *, Args...)>;
    _ctor_type _ctor;

public:
    Ctor(duk_context *l,
         const std::string &metatable_name,
		 const std::string &class_name)
         : _ctor([metatable_name](duk_context *state, Args... args) {
			void *addr = static_cast<void*>(new T(args...));
			duv_push_obj_ptr(state, addr);
            duvL_setmetatable(state, metatable_name.c_str());
           }) {
		duv_push_c_function_ptr(l, &detail::_js_dispatcher, DUK_VARARGS, (void *)static_cast<BaseFun *>(this));
		duk_put_prop_string(l, -2, class_name.c_str()); // use class name as function name
    }

    int Apply(duk_context *l) {
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        auto pack = std::tuple_cat(std::make_tuple(l), args);
        detail::_lift(_ctor, pack);
        // The constructor will leave a single userdata entry on the stack
        return 1;
    }
};

template <typename T>
class CtorNull : public BaseFun {
private:

public:
	CtorNull(duk_context *l,
		const std::string &metatable_name,
		const std::string &class_name)
	{
	}

	int Apply(duk_context *l) {
		return 0;
	}
};
}

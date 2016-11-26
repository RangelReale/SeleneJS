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
			duk_push_object(state);
            void *addr = duk_push_fixed_buffer(state, sizeof(T));
            new(addr) T(args...);
			duk_put_prop_string(state, -2, "\xFF" "_obj");
            duvL_setmetatable(state, metatable_name.c_str()); // TODO
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
}

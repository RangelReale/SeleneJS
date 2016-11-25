#pragma once
#include <iostream>
#include <memory>
#include <typeinfo>
#include <unordered_map>

#include <duktape.h>
#include "refs.h"
#include "lmetatable.h"
#include "metatable.h"

namespace seljs {

namespace detail {
struct GetUserdataParameterFromJSTypeError {
    std::string metatable_name;
    int index;
};
}

namespace MetatableRegistry {
using TypeID = std::reference_wrapper<const std::type_info>;
namespace detail {

static inline void _create_table_in_registry(duk_context *state, const std::string & name) {
	duk_push_heap_stash(state);
	duk_push_lstring(state, name.c_str(), name.size());
	duk_push_object(state);
	duk_put_prop(state, -3);
	duk_pop(state); // stash
}

static inline void _push_names_table(duk_context *state) {
	duk_push_heap_stash(state);
	duk_push_string(state, "selenejs_metatable_names");
	duk_get_prop(state, -2);
	duk_remove(state, -2); // stash
}

static inline void _push_meta_table(duk_context *state) {
	duk_push_heap_stash(state);
	duk_push_string(state, "selenejs_metatables");
	duk_get_prop(state, -2);
	duk_remove(state, -2); // stash
}

static inline void _push_typeinfo(duk_context *state, TypeID type) {
	duk_push_pointer(state, const_cast<std::type_info*>(&type.get()));
}

static inline void _get_metatable(duk_context *state, TypeID type) {
    detail::_push_meta_table(state);
    detail::_push_typeinfo(state, type);
	duk_get_prop(state, -2);
	duk_remove(state, -2);
}

}

static inline void Create(duk_context *state) {
	duv_ref_setup(state);
    detail::_create_table_in_registry(state, "selene_metatable_names");
    detail::_create_table_in_registry(state, "selene_metatables");
}

static inline void PushNewMetatable(duk_context *state, TypeID type, const std::string& name) {
    detail::_push_names_table(state);

    detail::_push_typeinfo(state, type);
	duk_push_lstring(state, name.c_str(), name.size());
	duk_put_prop(state, -3);

    duk_pop_n(state, 1);


    duvL_newmetatable(state, name.c_str()); // Actual result.


    detail::_push_meta_table(state);

    detail::_push_typeinfo(state, type);
    duk_dup(state, -3);
    duk_put_prop(state, -3);

    duk_pop(state);
}

static inline bool SetMetatable(duk_context *state, TypeID type) {
    detail::_get_metatable(state, type);

	if (duk_is_object(state, -1)) {
        duv_setmetatable(state, -2);
        return true;
    }

    duk_pop(state);
    return false;
}

static inline bool IsRegisteredType(duk_context *state, TypeID type) {
    detail::_push_names_table(state);
    detail::_push_typeinfo(state, type);
    duk_get_prop(state, -2);

    bool registered = duk_is_string(state, -1) != 0;
    duk_pop_2(state);
    return registered;
}

static inline std::string GetTypeName(duk_context *state, TypeID type) {
    std::string name("unregistered type");

    detail::_push_names_table(state);
    detail::_push_typeinfo(state, type);
    duk_get_prop(state, -2);

    if(duk_is_string(state, -1) != 0) {
        size_t len = 0;
        char const * str = duk_to_lstring(state, -1, &len);
        name.assign(str, len);
    }

    duk_pop_2(state);
    return name;
}

static inline std::string GetTypeName(duk_context *state, int index) {
    std::string name;

    if(duv_getmetatable(state, index) != 0) {
		duk_push_string(state, "__name");
		duk_get_prop(state, -2);

        if(duk_is_string(state, -1) != 0) {
            size_t len = 0;
            char const * str = duk_to_lstring(state, -1, &len);
            name.assign(str, len);
        }

        duk_pop_2(state);
    }

    if(name.empty()) {
        name = duv_typename(state, duk_get_type(state, index));
    }

    return name;
}

static inline bool IsType(duk_context *state, TypeID type, const int index) {
    bool equal = true;

    if(duv_getmetatable(state, index)) {
        detail::_get_metatable(state, type);
        equal = duk_is_object(state, -1)!=0 && duk_equals(state, -1, -2)!=0; // POSSIBLE PROBLEM
        duk_pop_2(state);
    } else {
        detail::_get_metatable(state, type);
        equal = duk_is_object(state, -1)==0;
        duk_pop(state);
    }

    return equal;
}

static inline void CheckType(duk_context *state, TypeID type, const int index) {
    if(!IsType(state, type, index)) {
        throw seljs::detail::GetUserdataParameterFromJSTypeError{
            GetTypeName(state, type),
            index
        };
    }
}

}

}

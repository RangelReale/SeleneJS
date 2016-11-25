#pragma once
#include <iostream>
#include <memory>
#include <typeinfo>
#include <unordered_map>

#include <duktape.h>
#include "refs.h"

namespace seljs {

namespace detail {
struct GetUserdataParameterFromLuaTypeError {
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


    luaL_newmetatable(state, name.c_str()); // Actual result.


    detail::_push_meta_table(state);

    detail::_push_typeinfo(state, type);
    lua_pushvalue(state, -3);
    lua_settable(state, -3);

    lua_pop(state, 1);
}

static inline bool SetMetatable(lua_State *state, TypeID type) {
    detail::_get_metatable(state, type);

    if(lua_istable(state, -1)) {
        lua_setmetatable(state, -2);
        return true;
    }

    lua_pop(state, 1);
    return false;
}

static inline bool IsRegisteredType(lua_State *state, TypeID type) {
    detail::_push_names_table(state);
    detail::_push_typeinfo(state, type);
    lua_gettable(state, -2);

    bool registered = lua_isstring(state, -1);
    lua_pop(state, 2);
    return registered;
}

static inline std::string GetTypeName(lua_State *state, TypeID type) {
    std::string name("unregistered type");

    detail::_push_names_table(state);
    detail::_push_typeinfo(state, type);
    lua_gettable(state, -2);

    if(lua_isstring(state, -1)) {
        size_t len = 0;
        char const * str = lua_tolstring(state, -1, &len);
        name.assign(str, len);
    }

    lua_pop(state, 2);
    return name;
}

static inline std::string GetTypeName(lua_State *state, int index) {
    std::string name;

    if(lua_getmetatable(state, index)) {
        lua_pushliteral(state, "__name");
        lua_gettable(state, -2);

        if(lua_isstring(state, -1)) {
            size_t len = 0;
            char const * str = lua_tolstring(state, -1, &len);
            name.assign(str, len);
        }

        lua_pop(state, 2);
    }

    if(name.empty()) {
        name = lua_typename(state, lua_type(state, index));
    }

    return name;
}

static inline bool IsType(lua_State *state, TypeID type, const int index) {
    bool equal = true;

    if(lua_getmetatable(state, index)) {
        detail::_get_metatable(state, type);
        equal = lua_istable(state, -1) && lua_rawequal(state, -1, -2);
        lua_pop(state, 2);
    } else {
        detail::_get_metatable(state, type);
        equal = !lua_istable(state, -1);
        lua_pop(state, 1);
    }

    return equal;
}

static inline void CheckType(lua_State *state, TypeID type, const int index) {
    if(!IsType(state, type, index)) {
        throw sel::detail::GetUserdataParameterFromLuaTypeError{
            GetTypeName(state, type),
            index
        };
    }
}

}

}

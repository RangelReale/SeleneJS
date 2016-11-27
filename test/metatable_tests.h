#pragma once

#include <selenejs.h>

struct Qux {
    int baz() { return 4; }
    int qux = 3;
};

static Qux qux;

Qux *GetQuxPtr() { return &qux; }
Qux &GetQuxRef() { return qux; }

bool test_metatable_registry_ptr(seljs::State &state) {
    state["get_instance"] = &GetQuxPtr;
    state["Qux"].SetClass<Qux>("baz", &Qux::baz);
    state.Load("../test/test_metatable.js");
    return state["call_method"]() == 4;
}

bool test_metatable_registry_ref(seljs::State &state) {
    state["get_instance"] = &GetQuxRef;
    state["Qux"].SetClass<Qux>("baz", &Qux::baz);
    state.Load("../test/test_metatable.js");
    return state["call_method"]() == 4;
}

bool test_metatable_ptr_member(seljs::State &state) {
    state["get_instance"] = &GetQuxPtr;
    state["Qux"].SetClass<Qux>("baz", &Qux::baz, "qux", &Qux::qux);
    state.Load("../test/test_metatable.js");
    return state["access_member"]() == 3;
}

bool test_metatable_ref_member(seljs::State &state) {
    state["get_instance"] = &GetQuxRef;
    state["Qux"].SetClass<Qux>("baz", &Qux::baz, "qux", &Qux::qux);
    state.Load("../test/test_metatable.js");
    return state["access_member"]() == 3;
}

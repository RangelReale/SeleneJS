#pragma once

#include "common/lifetime.h"
#include <selenejs.h>

bool test_select_global(seljs::State &state) {
    state.Load("../test/test.js");
    int answer = state["my_global"];
    return answer == 4;
}

bool test_select_field(seljs::State &state) {
    state.Load("../test/test.js");
    duk_double_t answer = state["my_table"]["key"];
    return answer == duk_double_t(6.4);
}

bool test_select_index(seljs::State &state) {
    state.Load("../test/test.js");
    std::string answer = state["my_table"][3];
    return answer == "hi";
}

bool test_select_nested_field(seljs::State &state) {
    state.Load("../test/test.js");
    std::string answer = state["my_table"]["nested"]["foo"];
    return answer == "bar";
}

bool test_select_nested_index(seljs::State &state) {
    state.Load("../test/test.js");
    int answer = state["my_table"]["nested"][2];
    return answer == -3;
}

bool test_select_equality(seljs::State &state) {
    state.Load("../test/test.js");
    return state["my_table"]["nested"][2] == -3;
}

bool test_select_cast(seljs::State &state) {
    state.Load("../test/test.js");
    return int(state["global1"]) == state["global2"];
}

bool test_set_global(seljs::State &state) {
    state.Load("../test/test.js");
    auto lua_dummy_global = state["dummy_global"];
    lua_dummy_global = 32;
    return state["dummy_global"] == 32;
}

bool test_set_field(seljs::State &state) {
    state.Load("../test/test.js");
    state["my_table"]["dummy_key"] = "testing";
    return state["my_table"]["dummy_key"] == "testing";
}

bool test_set_index(seljs::State &state) {
    state.Load("../test/test.js");
    state["my_table"][10] = 3;
    return state["my_table"][10] == 3;
}

bool test_set_nested_field(seljs::State &state) {
    state.Load("../test/test.js");
    state["my_table"]["nested"]["asdf"] = true;
    return state["my_table"]["nested"]["asdf"];
}

bool test_set_nested_index(seljs::State &state) {
    state.Load("../test/test.js");
    state["my_table"]["nested"][1] = 2;
    return state["my_table"]["nested"][1] == 2;
}

bool test_create_table_field(seljs::State &state) {
    state["new_table"]["test"] = 4;
    return state["new_table"]["test"] == 4;
}

bool test_create_table_index(seljs::State &state) {
    state["new_table"][3] = 4;
    return state["new_table"][3] == 4;
}

bool test_cache_selector_field_assignment(seljs::State &state) {
    seljs::Selector s = state["new_table"][3];
    s = 4;
    return state["new_table"][3] == 4;
}

bool test_cache_selector_field_access(seljs::State &state) {
    state["new_table"][3] = 4;
    seljs::Selector s = state["new_table"][3];
    return s == 4;
}

bool test_cache_selector_function(seljs::State &state) {
    state.Load("../test/test.js");
    seljs::Selector s = state["set_global"];
    s();
    return state["global1"] == 8;
}

bool test_function_should_run_once(seljs::State &state) {
    state.Load("../test/test.js");
    auto should_run_once = state["should_run_once"];
    should_run_once();
    return state["should_be_one"] == 1;
}

bool test_function_result_is_alive_ptr(seljs::State &state) {
    using namespace test_lifetime;
    state["Obj"].SetClass<InstanceCounter>();
	state("function createObj() { return new Obj.Obj(); }");
    int const instanceCountBeforeCreation = InstanceCounter::instances;

    seljs::Pointer<InstanceCounter> pointer = state["createObj"]();
    state.ForceGC();

    return InstanceCounter::instances == instanceCountBeforeCreation + 1;
}

bool test_function_result_is_alive_ref(seljs::State &state) {
    using namespace test_lifetime;
    state["Obj"].SetClass<InstanceCounter>();
	state("function createObj() { return new Obj.Obj(); }");
    int const instanceCountBeforeCreation = InstanceCounter::instances;

    seljs::Reference<InstanceCounter> reference = state["createObj"]();
    state.ForceGC();

    return InstanceCounter::instances == instanceCountBeforeCreation + 1;
}

bool test_get_and_set_Reference_keeps_identity(seljs::State &state) {
    using namespace test_lifetime;
    state["Obj"].SetClass<InstanceCounter>();
	state("objA = new Obj.Obj();");

    seljs::Reference<InstanceCounter> objA_ref = state["objA"];
    state["objB"] = objA_ref;
    seljs::Reference<InstanceCounter> objB_ref = state["objB"];

	state("function areVerySame() { return objA == objB; }");
    return state["areVerySame"]() && (&objA_ref.get() == &objB_ref.get());
}

bool test_get_and_set_Pointer_keeps_identity(seljs::State &state) {
    using namespace test_lifetime;
    state["Obj"].SetClass<InstanceCounter>();
	state("objA = new Obj.Obj();");

    seljs::Pointer<InstanceCounter> objA_ptr = state["objA"];
    state["objB"] = objA_ptr;
    seljs::Pointer<InstanceCounter> objB_ptr = state["objB"];

	state("function areVerySame() { return objA == objB; }");
    return state["areVerySame"]() && (objA_ptr == objB_ptr);
}

struct SelectorBar {};
struct SelectorFoo {
    int x;
    SelectorFoo(int num) : x(num) {}
    int getX() {
        return x;
    }
};

bool test_selector_call_with_registered_class(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>("get", &SelectorFoo::getX);
	state("function getXFromFoo(foo) { return foo.get(); }");
    SelectorFoo foo{4};
    return state["getXFromFoo"](foo) == 4;
}

bool test_selector_call_with_registered_class_ptr(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>("get", &SelectorFoo::getX);
	state("function getXFromFoo(foo) { return foo.get(); }");
    SelectorFoo foo{4};
    return state["getXFromFoo"](&foo) == 4;
}

bool test_selector_call_with_wrong_type_ptr(seljs::State &state) {
    auto acceptFoo = [] (SelectorFoo *) {};
    state["Foo"].SetClass<SelectorFoo, int>();
    state["Bar"].SetClass<SelectorBar>();
    state["acceptFoo"] = acceptFoo;
	state("bar = new Bar.Bar();");

    bool error_encounted = false;
    state.HandleExceptionsWith([&error_encounted](int, std::string, std::exception_ptr) {
        error_encounted = true;
    });
	state("acceptFoo(bar);");

    return error_encounted;
}

bool test_selector_call_with_wrong_type_ref(seljs::State &state) {
    auto acceptFoo = [] (SelectorFoo &) {};
    state["Foo"].SetClass<SelectorFoo, int>();
    state["Bar"].SetClass<SelectorBar>();
    state["acceptFoo"] = acceptFoo;
	state("bar = new Bar.Bar();");

    bool error_encounted = false;
    state.HandleExceptionsWith([&error_encounted](int, std::string, std::exception_ptr) {
        error_encounted = true;
    });
	state("acceptFoo(bar);");

    return error_encounted;
}

bool test_selector_call_with_nullptr_ref(seljs::State &state) {
    auto acceptFoo = [] (SelectorFoo &) {};
    state["Foo"].SetClass<SelectorFoo, int>();
    state["acceptFoo"] = acceptFoo;

    bool error_encounted = false;
    state.HandleExceptionsWith([&error_encounted](int, std::string, std::exception_ptr) {
        error_encounted = true;
    });
	state("acceptFoo(nil);");

    return error_encounted;
}

bool test_selector_get_nullptr_ref(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>();
	state("bar = null;");
    bool error_encounted = false;

    try{
        SelectorFoo & foo = state["bar"];
    } catch(seljs::TypeError &) {
        error_encounted = true;
    }

    return error_encounted;
}

bool test_selector_get_wrong_ref(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>();
    state["Bar"].SetClass<SelectorBar>();
	state("bar = new Bar.Bar();");
    bool error_encounted = false;

    try{
        SelectorFoo & foo = state["bar"];
    } catch(seljs::TypeError &) {
        error_encounted = true;
    }

    return error_encounted;
}

bool test_selector_get_wrong_ref_to_string(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>();
	state("bar = \"Not a Foo\";");
    bool expected_message = false;

    try{
        SelectorFoo & foo = state["bar"];
    } catch(seljs::TypeError & e) {
        expected_message = std::string(e.what()).find("got STRING") != std::string::npos;
    }

    return expected_message;
}

bool test_selector_get_wrong_ref_to_table(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>();
	state("bar = {};");
    bool expected_message = false;

    try{
        SelectorFoo & foo = state["bar"];
    } catch(seljs::TypeError & e) {
        expected_message = std::string(e.what()).find("got OBJECT") != std::string::npos;
    }

    return expected_message;
}

bool test_selector_get_wrong_ref_to_unregistered(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>();
	state("foo = new Foo.Foo(4);");
    bool expected_message = false;

    try{
        SelectorBar & bar = state["foo"];
    } catch(seljs::TypeError & e) {
        expected_message = std::string(e.what()).find("unregistered type expected") != std::string::npos;
    }

    return expected_message;
}

bool test_selector_get_wrong_ptr(seljs::State &state) {
    state["Foo"].SetClass<SelectorFoo, int>();
    state["Bar"].SetClass<SelectorBar>();
	state("bar = new Bar.Bar();");
    SelectorFoo * foo = state["bar"];
    return foo == nullptr;
}

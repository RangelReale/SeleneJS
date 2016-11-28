#pragma once

#include "common/lifetime.h"
#include <iostream>
#include <selenejs.h>

int take_fun_arg(seljs::function<int(int, int)> fun, int a, int b) {
    return fun(a, b);
}

struct Mutator {
    Mutator() {}
    Mutator(seljs::function<void(int)> fun) {
        fun(-4);
    }
    seljs::function<void()> Foobar(bool which,
                                 seljs::function<void()> foo,
                                 seljs::function<void()> bar) {
        return which ? foo : bar;
    }
};

bool test_function_reference(seljs::State &state) {
    state["take_fun_arg"] = &take_fun_arg;
    state.Load("../test/test_ref.js");
    bool check1 = state["pass_add"](3, 5) == 8;
    bool check2 = state["pass_sub"](4, 2) == 2;
    return check1 && check2;
}

bool test_function_in_constructor(seljs::State &state) {
    state["Mutator"].SetClass<Mutator, seljs::function<void(int)>>();
    state.Load("../test/test_ref.js");
    bool check1 = state["a"] == 4;
	state("mutator = new Mutator.Mutator(mutate_a);");
    bool check2 = state["a"] == -4;
    return check1 && check2;
}

bool test_pass_function_to_lua(seljs::State &state) {
    state["Mutator"].SetClass<Mutator>("foobar", &Mutator::Foobar);
    state.Load("../test/test_ref.js");
	state("mutator = new Mutator.Mutator();");
	state("mutator.foobar(true, foo, bar)();");
    bool check1 = state["test"] == "foo";
	state("mutator.foobar(false, foo, bar)();");
    bool check2 = state["test"] == "bar";
    return check1 && check2;
}

bool test_call_returned_lua_function(seljs::State &state) {
    state.Load("../test/test_ref.js");
    seljs::function<int(int, int)> lua_add = state["add"];
    return lua_add(2, 4) == 6;
}

bool test_call_result_is_alive_ptr(seljs::State &state) {
    using namespace test_lifetime;
    state["Obj"].SetClass<InstanceCounter>();
	state("function createObj() { return new Obj.Obj(); }");
    seljs::function<seljs::Pointer<InstanceCounter>()> createObj = state["createObj"];
    int const instanceCountBeforeCreation = InstanceCounter::instances;

    seljs::Pointer<InstanceCounter> pointer = createObj();
    state.ForceGC();

    return InstanceCounter::instances == instanceCountBeforeCreation + 1;
}

bool test_call_result_is_alive_ref(seljs::State &state) {
    using namespace test_lifetime;
    state["Obj"].SetClass<InstanceCounter>();
	state("function createObj() { return new Obj.Obj(); }");
    seljs::function<seljs::Reference<InstanceCounter>()> createObj = state["createObj"];
    int const instanceCountBeforeCreation = InstanceCounter::instances;

    seljs::Reference<InstanceCounter> ref = createObj();
    state.ForceGC();

    return InstanceCounter::instances == instanceCountBeforeCreation + 1;
}

struct FunctionFoo {
    int x;
    FunctionFoo(int num) : x(num) {}
    int getX() {
        return x;
    }
};
struct FunctionBar {};

bool test_function_call_with_registered_class(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>("get", &FunctionFoo::getX);
	state("function getX(foo) { return foo.get(); }");
    seljs::function<int(FunctionFoo &)> getX = state["getX"];
    FunctionFoo foo{4};
    return getX(foo) == 4;
}

bool test_function_call_with_registered_class_ptr(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>("get", &FunctionFoo::getX);
	state("function getX(foo) { return foo.get(); }");
    seljs::function<int(FunctionFoo *)> getX = state["getX"];
    FunctionFoo foo{4};
    return getX(&foo) == 4;
}

bool test_function_call_with_registered_class_val(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>("get", &FunctionFoo::getX);
	state("function store(foo) { globalFoo = foo; }");
	state("function getX() { return globalFoo.get(); }");

    seljs::function<void(FunctionFoo)> store = state["store"];
    seljs::function<int()> getX = state["getX"];
    store(FunctionFoo{4});

    return getX() == 4;
}

bool test_function_call_with_registered_class_val_lifetime(seljs::State &state) {
    using namespace test_lifetime;
    state["Foo"].SetClass<InstanceCounter>();
	state("function store(foo) { globalFoo = foo; }");
    seljs::function<void(InstanceCounter)> store = state["store"];

    int instanceCountBefore = InstanceCounter::instances;
    store(InstanceCounter{});

    return InstanceCounter::instances == instanceCountBefore + 1;
}

bool test_function_call_with_nullptr_ref(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>();
	state("function makeNil() { return null; }");
    seljs::function<FunctionFoo &()> getFoo = state["makeNil"];
    bool error_encounted = false;

    try {
        FunctionFoo & foo = getFoo();
    } catch(seljs::TypeError &) {
        error_encounted = true;
    }

    return error_encounted;
}

bool test_function_call_with_wrong_ref(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>();
    state["Bar"].SetClass<FunctionBar>();
	state("function makeBar() { return new Bar.Bar(); }");
    seljs::function<FunctionFoo &()> getFoo = state["makeBar"];
    bool error_encounted = false;

    try {
        FunctionFoo & foo = getFoo();
    } catch(seljs::TypeError &) {
        error_encounted = true;
    }

    return error_encounted;
}

bool test_function_call_with_wrong_ptr(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>();
    state["Bar"].SetClass<FunctionBar>();
	state("function makeBar() { return new Bar.Bar(); }");
    seljs::function<FunctionFoo *()> getFoo = state["makeBar"];
    return nullptr == getFoo();
}

bool test_function_get_registered_class_by_value(seljs::State &state) {
    state["Foo"].SetClass<FunctionFoo, int>();
	state("function getFoo() { return new Foo.Foo(4); }");
    seljs::function<FunctionFoo()> getFoo = state["getFoo"];

    FunctionFoo foo = getFoo();

    return foo.getX() == 4;
}

#pragma once

#include "common/lifetime.h"
#include <memory>
#include <selenejs.h>
#include <string>

int my_add(int a, int b) {
    return a + b;
}

void no_return() {
}

bool test_function_no_args(seljs::State &state) {
    state.Load("../test/test.js");
    state["foo"]();
    return true;
}

bool test_add(seljs::State &state) {
    state.Load("../test/test.js");
    return state["add"](5, 2) == 7;
}

bool test_call_field(seljs::State &state) {
    state.Load("../test/test.js");
    int answer = state["mytable"]["foo"]();
    return answer == 4;
}

bool test_call_c_function(seljs::State &state) {
    state.Load("../test/test.js");
    state["cadd"] = std::function<int(int, int)>(my_add);
    int answer = state["cadd"](3, 6);
    return answer == 9;
}

bool test_call_c_fun_from_lua(seljs::State &state) {
    state.Load("../test/test.js");
    state["cadd"] = std::function<int(int, int)>(my_add);
    int answer = state["execute"]();
    return answer == 11;
}

bool test_no_return(seljs::State &state) {
    state["no_return"] = &no_return;
    state["no_return"]();
    return true;
}

bool test_call_std_fun(seljs::State &state) {
    state.Load("../test/test.js");
    std::function<int(int, int)> mult = [](int x, int y){ return x * y; };
    state["cmultiply"] = mult;
    int answer = state["cmultiply"](5, 6);
    return answer == 30;
}

bool test_call_lambda(seljs::State &state) {
    state.Load("../test/test.js");
    state["cmultiply"] = [](int x, int y){ return x * y; };
    int answer = state["cmultiply"](5, 6);
    return answer == 30;
}

bool test_call_normal_c_fun(seljs::State &state) {
    state.Load("../test/test.js");
    state["cadd"] = &my_add;
    const int answer = state["cadd"](4, 20);
    return answer == 24;
}

bool test_call_normal_c_fun_many_times(seljs::State &state) {
    // Ensures there isn't any strange overflow problem or lingering
    // state
    state.Load("../test/test.js");
    state["cadd"] = &my_add;
    bool result = true;
    for (int i = 0; i < 25; ++i) {
        const int answer = state["cadd"](4, 20);
        result = result && (answer == 24);
    }
    return result;
}

bool test_call_functor(seljs::State &state) {
    struct the_answer {
        int answer = 42;
        int operator()() {
            return answer;
        }
    };
    the_answer functor;
    state.Load("../test/test.js");
    state["c_the_answer"] = std::function<int()>(functor);
    int answer = state["c_the_answer"]();
    return answer == 42;

}

bool test_embedded_nulls(seljs::State &state) {
    state.Load("../test/test.js");
    const std::string result = state["embedded_nulls"]();
    return result.size() == 4;
}

struct Special {
    int foo = 3;
};

static Special special;

Special* return_special_pointer() { return &special; }

bool test_pointer_return(seljs::State &state) {
    state["return_special_pointer"] = &return_special_pointer;
    return state["return_special_pointer"]() == &special;
}

Special& return_special_reference() { return special; }

bool test_reference_return(seljs::State &state) {
    state["return_special_reference"] = &return_special_reference;
    Special &ref = state["return_special_reference"]();
    return &ref == &special;
}

test_lifetime::InstanceCounter return_value() { return {}; }

bool test_return_value(seljs::State &state) {
    using namespace test_lifetime;
    state["MyClass"].SetClass<InstanceCounter>();
    state["return_value"] = &return_value;
    int const instanceCountBeforeCreation = InstanceCounter::instances;

	state("globalValue = return_value();");

    return InstanceCounter::instances == instanceCountBeforeCreation + 1;
}

bool test_return_unregistered_type(seljs::State &state) {
    using namespace test_lifetime;
    state["return_value"] = &return_value;
    int const instanceCountBeforeCreation = InstanceCounter::instances;

    bool error_encounted = false;
    state.HandleExceptionsWith([&error_encounted](int, std::string msg, std::exception_ptr) {
        error_encounted = true;
    });

	state("globalValue = return_value();");

    return error_encounted;
}

bool test_value_parameter(seljs::State &state) {
    using namespace test_lifetime;
    state["MyClass"].SetClass<InstanceCounter>();
	state("function acceptValue(value) { valCopy = value; }");
    int const instanceCountBefore = InstanceCounter::instances;

    state["acceptValue"](InstanceCounter{});

    return InstanceCounter::instances == instanceCountBefore + 1;
}

bool test_wrong_value_parameter(seljs::State &state) {
    using namespace test_lifetime;
    state["MyClass"].SetClass<InstanceCounter>();
	state("function acceptValue(value) { valCopy = value; }");
    int const instanceCountBefore = InstanceCounter::instances;

    try {
        state["acceptValue"](Special{});
    } catch(seljs::CopyUnregisteredType & e)
    {
        return e.getType().get() == typeid(Special);
    }

    return false;
}

bool test_value_parameter_keeps_type_info(seljs::State &state) {
    using namespace test_lifetime;
    state["MyClass"].SetClass<Special>();
	state("function acceptValue(value) { valCopy = value; }");
    state["acceptValue"](Special{});

    Special * foo = state["valCopy"];

    return foo != nullptr;
}

bool test_callback_with_value(seljs::State &state) {
    using namespace test_lifetime;
    state["MyClass"].SetClass<InstanceCounter>();
	state("val = new MyClass.MyClass();");

    std::unique_ptr<InstanceCounter> copy;
    state["accept"] = [&copy](InstanceCounter counter) {
        copy.reset(new InstanceCounter(std::move(counter)));
    };

    int const instanceCountBeforeCall = InstanceCounter::instances;
    state("accept(val);");

    return InstanceCounter::instances == instanceCountBeforeCall + 1;
}

bool test_nullptr_to_nil(seljs::State &state) {
    state["getNullptr"] = []() -> void* {
        return nullptr;
    };
	state("x = getNullptr();");
	state("result = x == null;");
    return static_cast<bool>(state["result"]);
}

bool test_get_primitive_by_value(seljs::State & state) {
    state.Load("../test/test.js");
    return static_cast<int>(state["global1"]) == 5;
}

bool test_get_primitive_by_const_ref(seljs::State & state) {
    state.Load("../test/test.js");
    return static_cast<const int &>(state["global1"]) == 5;
}

bool test_get_primitive_by_rvalue_ref(seljs::State & state) {
    state.Load("../test/test.js");
    return static_cast<int &&>(state["global1"]) == 5;
}

bool test_call_with_primitive_by_value(seljs::State & state) {
    bool success = false;
    auto const accept_int_by_value = [&success](int x) {success = x == 5;};
    state["test"] = accept_int_by_value;
    state["test"](5);
    return success;
}

bool test_call_with_primitive_by_const_ref(seljs::State & state) {
    bool success = false;
    auto const accept_int_by_const_ref =
        [&success](const int & x) {success = x == 5;};
    state["test"] = accept_int_by_const_ref;
    state["test"](5);
    return success;
}

bool test_call_with_primitive_by_rvalue_ref(seljs::State & state) {
    bool success = false;
    auto const accept_int_by_rvalue_ref =
        [&success](int && x) {success = x == 5;};
    state["test"] = accept_int_by_rvalue_ref;
    state["test"](5);
    return success;
}

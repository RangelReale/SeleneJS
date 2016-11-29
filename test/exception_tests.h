#pragma once

#include <exception>
#include <duktape.h>
#include <selenejs.h>

bool test_catch_exception_from_callback_within_lua(seljs::State &state) {
    state.Load("../test/test_exceptions.js");
    state["throw_logic_error"] =
        []() {throw std::logic_error("Message from C++.");};
	std::string msg = state["call_protected"]("throw_logic_error");
	bool ok = state["ok"];
	return !ok && msg.find("Message from C++.") != std::string::npos;
}

bool test_catch_unknwon_exception_from_callback_within_lua(seljs::State &state) {
    state.Load("../test/test_exceptions.js");
    state["throw_int"] =
        []() {throw 0;};
	std::string msg = state["call_protected"]("throw_int");
	bool ok = state["ok"];
    return !ok && msg.find("invalid c++ exception") != std::string::npos;
}

bool test_call_exception_handler_for_exception_from_lua(seljs::State &state) {
    state.Load("../test/test_exceptions.js");
    int jsStatusCode = 1;
    std::string message;
    state.HandleExceptionsWith([&jsStatusCode, &message](int s, std::string msg, std::exception_ptr exception) {
        jsStatusCode = s, message = std::move(msg);
    });
    state["raise"]("Message from JS.");
    return jsStatusCode == DUK_EXEC_ERROR
        && message.find("Message from JS.") != std::string::npos;
}

bool test_call_exception_handler_for_exception_from_callback(seljs::State &state) {
    int jsStatusCode = 1;
    std::string message;
    state.HandleExceptionsWith([&jsStatusCode, &message](int s, std::string msg, std::exception_ptr exception) {
		jsStatusCode = s, message = std::move(msg);
    });
    state["throw_logic_error"] =
        []() {throw std::logic_error("Message from C++.");};
    state["throw_logic_error"]();
    return jsStatusCode == DUK_EXEC_ERROR
        && message.find("Message from C++.") != std::string::npos;
}

bool test_call_exception_handler_while_using_sel_function(seljs::State &state) {
    state.Load("../test/test_exceptions.js");
    int jsStatusCode = 1;
    std::string message;
    state.HandleExceptionsWith([&jsStatusCode, &message](int s, std::string msg, std::exception_ptr exception) {
		jsStatusCode = s, message = std::move(msg);
    });
    seljs::function<void(std::string)> raiseFromLua = state["raise"];
    raiseFromLua("Message from JS.");
    return jsStatusCode == DUK_EXEC_ERROR
        && message.find("Message from JS.") != std::string::npos;
}

bool test_rethrow_exception_for_exception_from_callback(seljs::State &state) {
    state.HandleExceptionsWith([](int s, std::string msg, std::exception_ptr exception) {
        if(exception) {
            std::rethrow_exception(exception);
        }
    });
    state["throw_logic_error"] =
        []() {throw std::logic_error("Arbitrary message.");};
    try {
        state["throw_logic_error"]();
    } catch(std::logic_error & e) {
        return std::string(e.what()).find("Arbitrary message.") != std::string::npos;
    }
    return false;
}

bool test_rethrow_using_sel_function(seljs::State & state) {
    state.HandleExceptionsWith([](int s, std::string msg, std::exception_ptr exception) {
        if(exception) {
            std::rethrow_exception(exception);
        }
    });
    state["throw_logic_error"] =
        []() {throw std::logic_error("Arbitrary message.");};
    seljs::function<void(void)> cause_exception = state["throw_logic_error"];
    try {
        cause_exception();
    } catch(std::logic_error & e) {
        return std::string(e.what()).find("Arbitrary message.") != std::string::npos;
    }
    return false;
}

bool test_throw_on_exception_using_Load(seljs::State &state) {
    state.HandleExceptionsWith([](int s, std::string msg, std::exception_ptr exception) {
        throw std::logic_error(msg);
    });
    try {
        state.Load("non_existing_file");
    } catch (std::logic_error & e) {
        return std::string(e.what()).find("no sourcecode") != std::string::npos;
    }
    return false;
}

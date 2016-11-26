#pragma once

#include "ExceptionHandler.h"
#include <iostream>
#include <memory>
#include <string>
#include "Registry.h"
#include "Selector.h"
#include <tuple>
#include "util.h"
#include <vector>

//// Selector.h
#include "ResourceHandler.h"

namespace seljs {
class State {
private:
    duk_context *_l;
    bool _l_owner;
    std::unique_ptr<Registry> _registry;
    std::unique_ptr<ExceptionHandler> _exception_handler;

public:
    State() : _l(nullptr), _l_owner(true), _exception_handler(new ExceptionHandler) {
        _l = duk_create_heap_default();
        if (_l == nullptr) throw 0;
        _registry.reset(new Registry(_l));
        HandleExceptionsPrintingToStdOut();
    }
    State(duk_context *l) : _l(l), _l_owner(false), _exception_handler(new ExceptionHandler) {
        _registry.reset(new Registry(_l));
        HandleExceptionsPrintingToStdOut();
    }
    State(const State &other) = delete;
    State &operator=(const State &other) = delete;
    State(State &&other)
        : _l(other._l),
          _l_owner(other._l_owner),
          _registry(std::move(other._registry)) {
        other._l = nullptr;
    }
    State &operator=(State &&other) {
        if (&other == this) return *this;
        _l = other._l;
        _l_owner = other._l_owner;
        _registry = std::move(other._registry);
        other._l = nullptr;
        return *this;
    }
    ~State() {
        if (_l != nullptr && _l_owner) {
            ForceGC();
			duk_destroy_heap(_l);
        }
        _l = nullptr;
    }

    int Size() const {
		return duk_get_top(_l);
    }

    bool Load(const std::string &file) {
        ResetStackOnScopeExit savedStack(_l);
		if (duk_peval_file(_l, file.c_str()) == 0) {
            return true;
        }

		/*
        const char *msg = lua_tostring(_l, -1);
        _exception_handler->Handle(status, msg ? msg : file + ": dofile failed");
		*/
        return false;
    }

    void HandleExceptionsPrintingToStdOut() {
        *_exception_handler = ExceptionHandler([](int, std::string msg, std::exception_ptr){_print(msg);});
    }

    void HandleExceptionsWith(ExceptionHandler::function handler) {
        *_exception_handler = ExceptionHandler(std::move(handler));
    }

public:
    Selector operator[](const char *name) {
        return Selector(_l, *_registry, *_exception_handler, name);
    }

    bool operator()(const char *code) {
        ResetStackOnScopeExit savedStack(_l);
		if (duk_peval_string(_l, code) != 0) {
			throw TypeError{ duk_safe_to_string(_l, -1) }; // TEMP
			//_exception_handler->Handle_top_of_stack(status, _l);
            return false;
        }
        return true;
    }
    void ForceGC() {
        duk_gc(_l, 0);
    }

    void InteractiveDebug() {
		duk_peval_string(_l, "debug.debug()");
    }

    friend std::ostream &operator<<(std::ostream &os, const State &state);
};

inline std::ostream &operator<<(std::ostream &os, const State &state) {
    os << "seljs::State - " << state._l;
    return os;
}
}

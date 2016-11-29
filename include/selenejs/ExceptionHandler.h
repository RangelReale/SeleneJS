#pragma once
#include <functional>
#include "primitives.h"
#include <string>
#include "util.h"
#include "lmetatable.h"

#include <duktape.h>

namespace seljs {

struct stored_exception {
    std::string what;
    std::exception_ptr exception;
};

inline std::string const * _stored_exception_metatable_name() {
    static std::string const name = "selenejs_stored_exception";
    return &name;
}

inline duk_ret_t _delete_stored_exception(duk_context * l) {
	duk_get_prop_string(l, -1, "\xFF" "_exception");
	if (duk_is_pointer(l, -1)) {
		void * user_data = duk_get_pointer(l, -1);
		static_cast<stored_exception *>(user_data)->~stored_exception();
	}
	duk_pop(l);
	return 0;
}

inline duk_ret_t _push_stored_exceptions_what(duk_context * l) {
	duk_get_prop_string(l, -1, "\xFF" "_exception");
    void * user_data = duk_get_pointer(l, -1);
    std::string const & what = static_cast<stored_exception *>(user_data)->what;
    detail::_push(l, what);
	duk_pop(l);
    return 1;
}

inline void _register_stored_exception_metatable(duk_context * l) {
    duvL_newmetatable(l, _stored_exception_metatable_name()->c_str());
    duk_push_c_function(l, _delete_stored_exception, 1);
	duk_set_finalizer(l, -2);
	duk_push_c_function(l, _push_stored_exceptions_what, 1); // TODO
	duk_put_prop_string(l, -2, "toString");
}

inline void store_current_exception(duk_context * l, char const * what) {
	stored_exception* user_data = new stored_exception{ what, std::current_exception() };
    duk_push_pointer(l, static_cast<void*>(user_data));
	duk_put_prop_string(l, -2, "\xFF" "_exception");

    duvL_getmetatable(l, _stored_exception_metatable_name()->c_str());
    if(duk_is_undefined(l, -1) != 0) {
        duk_pop(l);
        _register_stored_exception_metatable(l);
    }

    duv_setmetatable(l, -2);
}

inline stored_exception * test_stored_exception(duk_context *l) {
	if (duk_is_object(l, -1)) {
		duk_get_prop_string(l, -1, "\xFF" "_exception");
		if(duk_is_pointer(l, -1)) {
			// TODO: check if the prototype is "selenejs_stored_exception"
			void * user_data = duk_get_pointer(l, -1);
			duk_pop(l);
			if(user_data != nullptr) {
				return static_cast<stored_exception *>(user_data);
			}
		}
		duk_pop(l);
	}
	return nullptr;
}

inline bool push_stored_exceptions_what(duk_context * l) {
    stored_exception * stored = test_stored_exception(l);
    if(stored != nullptr) {
        detail::_push(l, static_cast<const std::string &>(stored->what));
        return true;
    }
    return false;
}

inline std::exception_ptr extract_stored_exception(duk_context *l) {
    stored_exception * stored = test_stored_exception(l);
    if(stored != nullptr) {
        return stored->exception;
    }
    return nullptr;
}

inline void fatal_function(duk_context *ctx, duk_errcode_t code, const char *msg)
{
	store_current_exception(ctx, msg);
	// TODO: should stop the applcation
}

class ExceptionHandler {
public:
    using function = std::function<void(int,std::string,std::exception_ptr)>;

private:
    function _handler;

public:
    ExceptionHandler() = default;

    explicit ExceptionHandler(function && handler) : _handler(handler) {}

	void Handle(int jsStatusCode, std::string message, std::exception_ptr exception = nullptr) {
        if(_handler) {
            _handler(jsStatusCode, std::move(message), std::move(exception));
        }
    }

    void Handle_top_of_stack(int jsStatusCode, duk_context *L) {
        stored_exception * stored = test_stored_exception(L);
        if(stored) {
            Handle(
                jsStatusCode,
                stored->what,
                stored->exception);
        } else {
            Handle(
                jsStatusCode,
				ErrorMessage(L, -1));
        }
    }

};
}

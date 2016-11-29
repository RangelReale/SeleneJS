#pragma once

#include <memory>
#include <vector>
#include "primitives.h"
#include "ResourceHandler.h"
#include "refs.h"

#include <duktape.h>

namespace seljs {
namespace detail {
class JSRefDeleter {
private:
    duk_context *_state;
public:
    JSRefDeleter(duk_context *state) : _state{state} {}
    void operator()(int *ref) const {
        duv_unref(_state, *ref);
        delete ref;
    }
};
}
class JSRef {
private:
    std::shared_ptr<int> _ref;
public:
    JSRef(duk_context *state, int ref)
        : _ref(new int{ref}, detail::JSRefDeleter{state}) {}

    JSRef(duk_context *state)
        : _ref()
        {}

	JSRef()
		: _ref()
	{}

	bool isRef() const {
		return _ref.operator bool();
	}

    void Push(duk_context *state) const {
		if (_ref)
			duv_push_ref(state, *_ref);
    }
};

template <typename T>
JSRef make_Ref(duk_context * state, T&& t) {
    detail::_push(state, std::forward<T>(t));
    return JSRef(state, duv_ref(state));
}

namespace detail {
    inline void append_ref_recursive(duk_context *, std::vector<JSRef> &) {}

    template <typename Head, typename... Tail>
    void append_ref_recursive(duk_context * state, std::vector<JSRef> & refs, Head&& head, Tail&&... tail) {
        refs.push_back(make_Ref(state, std::forward<Head>(head)));

        append_ref_recursive(state, refs, std::forward<Tail>(tail)...);
    }
}

template <typename... Args>
std::vector<JSRef> make_Refs(duk_context * state, Args&&... args) {
    std::vector<JSRef> refs;
    refs.reserve(sizeof...(Args));

    detail::append_ref_recursive(state, refs, std::forward<Args>(args)...);
    return refs;
}
}

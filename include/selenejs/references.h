#pragma once

#include <cstddef>
#include "JSRef.h"
#include <utility>

namespace seljs {

template<typename T>
class Reference {
    JSRef _lifetime;
    T *_obj;
public:
    Reference(T &obj, JSRef lifetime)
      : _lifetime(std::move(lifetime))
      , _obj(&obj) {}

    T& get() const {
        return *_obj;
    }

    operator T&() const {
        return get();
    }

    void _push(duk_context *l) const {
        _lifetime.Push(l);
    }
};

template<typename T>
class Pointer {
    JSRef _lifetime;
    T *_obj;
public:
    Pointer(T *obj, JSRef lifetime)
      : _lifetime(std::move(lifetime))
      , _obj(obj)
    {}

    Pointer(JSRef lifetime)
      : Pointer(nullptr, std::move(lifetime))
    {}

    T* get() const {
        return _obj;
    }

    T* operator->() const {
        return _obj;
    }

    T& operator*() const {
        return *_obj;
    }

    operator bool() const {
        return _obj;
    }

    bool operator!() const {
        return !_obj;
    }

    friend bool operator==(std::nullptr_t, Pointer<T> const & ptr) {
        return nullptr == ptr._obj;
    }

    friend bool operator==(Pointer<T> const & ptr, std::nullptr_t) {
        return nullptr == ptr._obj;
    }

    friend bool operator!=(std::nullptr_t, Pointer<T> const & ptr) {
        return !(nullptr == ptr);
    }

    friend bool operator!=(Pointer<T> const & ptr, std::nullptr_t) {
        return !(nullptr == ptr);
    }

    friend bool operator==(Pointer<T> const & ptrA, Pointer<T> const & ptrB) {
        return ptrA.get() == ptrB.get();
    }

    friend bool operator!=(Pointer<T> const & ptrA, Pointer<T> const & ptrB) {
        return ptrA.get() != ptrB.get();
    }

    void _push(duk_context *l) const {
        _lifetime.Push(l);
    }
};

namespace detail {

template<typename T>
struct is_primitive<seljs::Reference<T>> {
    static constexpr bool value = true;
};

template <typename T>
inline seljs::Reference<T> _check_get(_id<seljs::Reference<T>>,
                                    duk_context *l, const int index) {
    T& result = _check_get(_id<T&>{}, l, index);
    duk_dup(l, index);
    JSRef lifetime(l, duv_ref(l));
    return {result, lifetime};
}

template <typename T>
inline seljs::Reference<T> _get(_id<seljs::Reference<T>>,
                              duk_context *l, const int index) {
    T& result = _get(_id<T&>{}, l, index);
    duk_dup(l, index);
    JSRef lifetime(l, duv_ref(l));
    return {result, lifetime};
}

template<typename T>
inline void _push(duk_context *l, seljs::Reference<T> const & ref) {
    ref._push(l);
}


template<typename T>
struct is_primitive<seljs::Pointer<T>> {
    static constexpr bool value = true;
};

template <typename T>
inline seljs::Pointer<T> _check_get(_id<seljs::Pointer<T>>,
                                    duk_context *l, const int index) {
    auto result = _check_get(_id<T*>{}, l, index);
    if(result) {
        duk_dup(l, index);
        JSRef lifetime(l, duv_ref(l));
        return {result, lifetime};
    } else {
        return {JSRef(l)};
    }
}

template <typename T>
inline seljs::Pointer<T> _get(_id<seljs::Pointer<T>>,
                              duk_context *l, const int index) {
    auto result = _get(_id<T*>{}, l, index);
    if(result) {
        duk_dup(l, index);
        JSRef lifetime(l, duv_ref(l));
        return {result, lifetime};
    } else {
        return {JSRef(l)};
    }
}

template<typename T>
inline void _push(duk_context *l, seljs::Pointer<T> const & ptr) {
    ptr._push(l);
}

}
}

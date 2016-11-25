#pragma once

namespace seljs {

class MovingFlag {
    bool flag = false;

public:
    MovingFlag() = default;

    MovingFlag(MovingFlag const &) = default;

    MovingFlag & operator=(MovingFlag const &) = default;

    MovingFlag(MovingFlag && that) noexcept
        : flag(that.flag) {
        that = false;
    }

    MovingFlag & operator=(MovingFlag && that) noexcept {
        this->flag = that.flag;
        that = false;
        return *this;
    }

    operator bool() const {
        return flag;
    }

    MovingFlag & operator=(bool x) {
        flag = x;
        return *this;
    }
};

class ResetStackOnScopeExit {
    duk_context * _stack;
    int _saved_top_index;

public:
    explicit ResetStackOnScopeExit(duk_context * stack)
        : _stack(stack),
          _saved_top_index(duk_get_top(_stack))
    {}

    ~ResetStackOnScopeExit() {
        if (_stack) {
            duk_set_top(_stack, _saved_top_index);
        }
    }

    ResetStackOnScopeExit(ResetStackOnScopeExit const & ) = delete;
    ResetStackOnScopeExit(ResetStackOnScopeExit       &&) = delete;
    ResetStackOnScopeExit & operator=(ResetStackOnScopeExit const & ) = delete;
    ResetStackOnScopeExit & operator=(ResetStackOnScopeExit       &&) = delete;
};
}

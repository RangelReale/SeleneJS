#pragma once

//#include "ObjFun.h"
#include "MetatableRegistry.h"
#include <functional>
#include <memory>
#include <string>
#include "util.h"
#include <utility>
#include <vector>

namespace seljs {
struct BaseClassObj {
    virtual ~BaseClassObj() {}
};
template <typename T>
class ClassObj : public BaseClassObj {
private:

public:
    ClassObj(duk_context *state, T *t) {
		duv_push_obj_ptr(state, t);
		MetatableRegistry::SetMetatable(state, typeid(T));
    }
};
}

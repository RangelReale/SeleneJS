#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1900 // before MSVS-14 CTP1
#define constexpr const
#endif

#include "selenejs/State.h"
#include "selenejs/Tuple.h"

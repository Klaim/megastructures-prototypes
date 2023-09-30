#pragma once

#include <version>

#ifdef __cpp_lib_unreachable
#   include <utility>
#else

namespace std
{
    [[noreturn]] inline void unreachable()
    {
        // Uses compiler specific extensions if possible.
        // Even if no extension is used, undefined behavior is still raised by
        // an empty function body and the noreturn attribute.
    #ifdef __GNUC__ // GCC, Clang, ICC
        __builtin_unreachable();
    #elifdef _MSC_VER // MSVC
        __assume(false);
    #endif
    }
}

#endif
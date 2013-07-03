#ifndef Core_Utils_H
#define Core_Utils_H

#include "Platform.h"
#include "TemplateUtils.h"

#define SG_NO_COPY_OPERATOR(TYPE) \
    TYPE & operator=(TYPE const&) = delete;

#define SG_NON_COPYABLE(TYPE) \
    TYPE(TYPE const&) = delete; \
    SG_NO_COPY_OPERATOR(TYPE)

#define SG_NON_NEWABLE \
    void* operator new(size_t) = delete; \
    void* operator new[](size_t) = delete;

#if SG_COMPILER_IS_MSVC
    #define SG_FUNCTION_NAME __FUNCTION__
    #define SG_FORCE_INLINE __forceinline
    #define SG_FUNCTION_RESTRICT __declspec(restrict)
    #define SG_RESTRICT __restrict
    #define SG_DEPRECATED __declspec(deprecated)
    #define SG_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#elif SG_COMPILER_IS_GCC
    #define SG_FUNCTION_NAME __PRETTY_FUNCTION__
    #define SG_FORCE_INLINE  __attribute__((always_inline))
    #define SG_RESTRICT __restrict__
#endif

#define SG_COMMA ,

namespace sg {
namespace internal {
    template <typename T, size_t N>
    char (&(GetArraySize(T(&)[N])))[N];
}
}

#define SG_OFFSET_OF(TYPE, MEMBER) (ptrdiff_t(reinterpret_cast<u8 const*>(&((TYPE const*)0)->MEMBER) - (u8 const*)0))
#define SG_ARRAYSIZE(A) sizeof(::sg::internal::GetArraySize(A))

#define SG_DEFINE_TYPED_TAG(NAME) \
    struct NAME##_internal { struct forbid_call { private: forbid_call(); }; }; \
    typedef void (&NAME##_t)(NAME##_internal::forbid_call); \
    namespace { inline void NAME(NAME##_internal::forbid_call) {} }

namespace sg {
//=============================================================================
// You can use uninitialized_t to declare constructors that do not initialize
// their members:
//      explicit MyClass(uninitialized_t) {}
// Then, you can call it by calling it with argument uninitialized:
//      MyClass myObj(uninitialized);
//=============================================================================
SG_DEFINE_TYPED_TAG(uninitialized)
//=============================================================================
// You can use auto_initialized_t to declare constructors that initialize the
// instance to a directly usable state:
//      explicit MyClass(auto_initialized_t) {}
// Then, you can call it by calling it with argument auto_initialized:
//      MyClass myObj(auto_initialized);
// The meaning of being auto initialized is left to the class writer. It is
// not assumed that the class will have all its members default initialized.
//=============================================================================
SG_DEFINE_TYPED_TAG(auto_initialized)
//=============================================================================
}

#endif

#ifndef Core_Assert_H
#define Core_Assert_H

#include "Config.h"
#include "Platform.h"
#include "Utils.h"

#if SG_PLATFORM_IS_WIN
#include "WindowsH.h"
#include <intrin.h>
#define SG_BREAKPOINT() __debugbreak()
#define SG_IMPL_ASSUME(expr) __assume(expr)
#define SG_IMPL_ANALYSIS_ASSUME(expr) __analysis_assume(expr)
#else
#error "todo"
#endif

#define SG_UNUSED(expr) ((void)(true ? 0 : ((expr), void(), 0)))

// SG_DEBUG_LINE can be used as the last line of a scope so that it is possible
// to put a breakpoint after all usefull instructions of a scope
#define SG_DEBUG_LINE do {} while(false);

#if SG_ENABLE_ASSERT
#define SG_IMPL_ASSERT_MSG(expr, msg) (expr || (::sg::HandleAssertFailedReturnMustBreak(#expr, msg, __FILE__, __LINE__, SG_FUNCTION_NAME) && (SG_BREAKPOINT(), true)))
#define SG_IMPL_ANALYSIS_ASSUME_IN_ASSERT(expr) SG_IMPL_ANALYSIS_ASSUME(expr)
#define SG_CODE_FOR_ASSERT(code) code
#else
#define SG_IMPL_ASSERT_MSG(expr, msg) ((void)0)
#define SG_IMPL_ANALYSIS_ASSUME_IN_ASSERT(expr) ((void)0)
#define SG_CODE_FOR_ASSERT(code)
#endif

#define SG_INLINE_ASSERT_MSG(expr, msg) SG_IMPL_ASSERT_MSG(expr, msg)
#define SG_INLINE_ASSERT(expr) SG_IMPL_ASSERT_MSG(expr, "")
#define SG_ASSERT_MSG(expr, msg) do { SG_IMPL_ANALYSIS_ASSUME_IN_ASSERT(expr); SG_IMPL_ASSERT_MSG(expr, msg); } while(false)
#define SG_ASSERT_MSG_AND_UNUSED(expr, msg) do { SG_UNUSED(expr); SG_ASSERT_MSG(expr, msg); } while(false)
#define SG_ASSERT_AND_UNUSED(expr) SG_ASSERT_MSG_AND_UNUSED(expr, "")
#define SG_ASSERT(expr) SG_ASSERT_MSG(expr, "")
#define SG_ASSERT_NOT_REACHED() SG_ASSERT_MSG(false, "Should not be reached !")
#define SG_ASSERT_NOT_IMPLEMENTED() SG_ASSERT_MSG(false, "This feature has not been implemented (yet).")

#define SG_ANALYSIS_ASSUME_MSG(expr, msg) do { SG_IMPL_ASSERT_MSG(expr, msg); SG_IMPL_ANALYSIS_ASSUME(expr); } while(false)
#define SG_ANALYSIS_ASSUME(expr) SG_ANALYSIS_ASSUME_MSG(expr, "")

#if SG_ENABLE_ASSERT
#define SG_ASSUME(expr) SG_ASSERT(expr)
#define SG_ASSUME_NOT_REACHED() SG_ASSERT_NOT_REACHED()
#else
#define SG_ASSUME(expr) SG_IMPL_ASSUME(expr)
#define SG_ASSUME_NOT_REACHED() SG_IMPL_ASSUME(0)
#endif

#define SG_POTENTIAL_CONSTANT_CONDITION(expr) (::sg::internal::potential_constant_condition_impl((expr)))
#define SG_CONSTANT_CONDITION(expr) (::sg::internal::constant_condition_impl<(expr)>::value())

// Use this macro to be able to add a breakpoint at the end of a function.
#define SG_BREAKABLE_POS do {} while(false)

namespace sg {
//=============================================================================
bool HandleAssertFailedReturnMustBreak(char const* iExpr,
                                       char const* iMsg,
                                       char const* iFile,
                                       size_t iLine,
                                       char const* iFctName);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace internal {
    SG_FORCE_INLINE bool potential_constant_condition_impl(bool x) { return x; }
    template<bool X> struct constant_condition_impl {};
    template<> struct constant_condition_impl<true> { SG_FORCE_INLINE static bool value() { return true; } };
    template<> struct constant_condition_impl<false> { SG_FORCE_INLINE static bool value() { return false; } };
}
//=============================================================================
}

#endif

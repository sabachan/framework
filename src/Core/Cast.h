#ifndef Core_Cast_H
#define Core_Cast_H

#include "Assert.h"
#include "Config.h"
#include "Platform.h"
#include <type_traits>

namespace sg {
//=============================================================================
template <typename T, typename U>
T num_cast(U val)
{
    T const r = static_cast<T>(val);
    SG_ASSERT(static_cast<U>(r) == val);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename U>
struct TemporaryForNumCastCheck
{
public:
    TemporaryForNumCastCheck(U v) : val(v) {}
    template <typename T> operator T () { return num_cast<T>(val); }
private:
    U val;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
TemporaryForNumCastCheck<T> checked_numcastable(T val)
{
    return TemporaryForNumCastCheck<T>(val);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace internal {
template <typename T> struct is_dynamic_castable {};
template <typename T> struct is_dynamic_castable<T*>
{
    template <typename U> bool operator() (U* val) { return nullptr != dynamic_cast<T*>(val); }
};
template <typename T> struct is_dynamic_castable<T&>
{
    template <typename U> bool operator() (U& val) { return nullptr != dynamic_cast<T*>(&val); }
};
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename U>
T checked_cast(U&& val)
{
#if SG_COMPILER_IS_MSVC
#if _CPPRTTI
    static_assert(!std::is_same<std::remove_pointer<T>, T>::value || !std::is_same<std::remove_reference<T>, T>::value, "checked_cast cannot return value by copy");
    static_assert(!std::is_same<std::remove_pointer<U>, U>::value || !std::is_same<std::remove_reference<U>, U>::value, "checked_cast cannot take value by copy");
    SG_ASSERT((internal::is_dynamic_castable<T>()(val)));
#else
    SG_CODE_FOR_ASSERT(static_assert(false, "rtti should be enabled in debug");)
#endif
#else
#error "todo"
#endif
    return static_cast<T>(val);
}
//=============================================================================
}


#endif

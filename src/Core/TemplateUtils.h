#ifndef Core_TemplateUtils_H
#define Core_TemplateUtils_H

#include "IntTypes.h"
#include <type_traits>

namespace sg {
//=============================================================================
// ConstPassing<T>::type is useful to choose between passing by const reference
// or by value arguments that are not modified.
// This is the single place where this policy should be implemented.
template<typename T>
struct ConstPassing
{
    static_assert(!std::is_reference<T>::value, "T should not be a reference");
    typedef typename std::conditional<
        std::is_trivially_copy_constructible<T>::value && sizeof(T) <= 2 * sizeof(T const&),
        T,
        T const&>::type type;
};
//=============================================================================
namespace internal {
template<typename T, u8 byte, size_t byteCount> struct broadcast_byte_impl { static T const value = broadcast_byte_impl<T, byte, byteCount-1>::value << 8 | byte; };
template<typename T, u8 byte> struct broadcast_byte_impl<T, byte, 1> { static T const value = byte; };
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, u8 byte> struct broadcast_byte
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    static T const value = internal::broadcast_byte_impl<T, byte, sizeof(T)>::value;
};
//=============================================================================
namespace internal {
struct all_ones_t {
    template<typename T> operator T() const { static_assert(std::is_integral<T>::value, "T must be integral"); return T(~T(0)); }
    template<typename T> friend bool operator == (all_ones_t, T x) { static_assert(std::is_integral<T>::value, "T must be integral"); return T(~T(0)) == x; }
    template<typename T> friend bool operator == (T x, all_ones_t) { static_assert(std::is_integral<T>::value, "T must be integral"); return T(~T(0)) == x; }
    template<typename T> friend bool operator != (all_ones_t, T x) { static_assert(std::is_integral<T>::value, "T must be integral"); return T(~T(0)) != x; }
    template<typename T> friend bool operator != (T x, all_ones_t) { static_assert(std::is_integral<T>::value, "T must be integral"); return T(~T(0)) != x; }
};
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
internal::all_ones_t const all_ones;
}
//=============================================================================
template<typename T, T a, T... bs> struct Sum;
template<typename T, T a> struct Sum<T, a> { static T const value = a; };
template<typename T, T a, T... bs> struct Sum { static T const value = a + Sum<T, bs...>::value; };
//=============================================================================
}

#endif

#ifndef Core_For_H
#define Core_For_H

#include "Assert.h"
#include "Preprocessor.h"
#include <iterator>
#include <limits>
#include <type_traits>

namespace sg {
//=============================================================================
#define SG_IMPL_FR_BEGIN(i) SG_CONCAT(begin_,SG_CONCAT(i,__LINE__))
#define SG_IMPL_FR_END(i) SG_CONCAT(end_,SG_CONCAT(i,__LINE__))
#define SG_IMPL_FR_IT(i) SG_CONCAT(it_,SG_CONCAT(i,__LINE__))
//=============================================================================
#define for_range(T,i,A,B) for(T SG_IMPL_FR_BEGIN(i)=A, SG_IMPL_FR_END(i)=B, i=SG_IMPL_FR_BEGIN(i); i<SG_IMPL_FR_END(i); ++i)
#define reverse_for_range(T,i,A,B) for(T SG_IMPL_FR_BEGIN(i)=A, SG_IMPL_FR_END(i)=B,SG_IMPL_FR_IT(i)=SG_IMPL_FR_END(i),i=SG_IMPL_FR_IT(i)-1; SG_IMPL_FR_IT(i)>SG_IMPL_FR_BEGIN(i); SG_IMPL_FR_IT(i)=i,i=SG_IMPL_FR_IT(i)-1)
#define for_range_0(T,i,N) for_range(T,i,0,N)
#define reverse_for_range_0(T,i,N) reverse_for_range(T,i,0,N)
//=============================================================================
namespace internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class iterator_range
{
public:
    iterator_range(T ib, T ie) : b(ib), e(ie) {}
    T begin() { return b; }
    T end() { return e; }
private:
    T b, e;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class reverse_view
{
    typedef typename std::conditional<std::is_const<T>::value, typename T::const_reverse_iterator, typename T::reverse_iterator>::type iterator_type;
public:
    reverse_view(T& iC) : c(iC) {}
    iterator_type begin() { return c.rbegin(); }
    iterator_type end() { return c.rend(); }
private:
    T& c;
};
}
//=============================================================================
template <typename T>
internal::iterator_range<T> iterator_range(T ib, T ie) { return internal::iterator_range<T>(ib, ie); }
//=============================================================================
template <typename T>
internal::reverse_view<T> reverse_view(T& iContainer) { return internal::reverse_view<T>(iContainer); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t N>
internal::iterator_range<std::reverse_iterator<T> > reverse_view(T (*iContainer)[N])
{
    return internal::iterator_range<ReverseIterator<T> >(std::reverse_iterator<T>(iContainer+N), std::reverse_iterator<T>(iContainer));
}
//=============================================================================

} // namespace sg

#endif

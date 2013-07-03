#ifndef Core_StridedArrayView_H
#define Core_StridedArrayView_H

#include "Assert.h"
#include "TemplateUtils.h"
#include <iterator>
#include <type_traits>

namespace sg {
//=============================================================================
template <typename T, bool IsConst>
class StridedArrayViewIterator
{
    typedef StridedArrayViewIterator this_type;
public:
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef typename std::conditional<IsConst, value_type const, value_type>::type* pointer;
    typedef typename std::conditional<IsConst, value_type const, value_type>::type& reference;
    typedef std::random_access_iterator_tag iterator_category;
public:
    SG_FORCE_INLINE StridedArrayViewIterator() : m_ptr(nullptr), m_stride(1) {}
    SG_FORCE_INLINE StridedArrayViewIterator(void* iPtr, size_t iStride) : m_ptr(iPtr), m_stride(iStride) {}
    SG_FORCE_INLINE T& operator *() const { return *reinterpret_cast<value_type*>(m_ptr); }
    SG_FORCE_INLINE T* operator ->() const { return reinterpret_cast<value_type*>(m_ptr); }
    SG_FORCE_INLINE T* get() const { return reinterpret_cast<value_type*>(m_ptr); }
    SG_FORCE_INLINE this_type& operator++() { m_ptr = reinterpret_cast<u8*>(m_ptr) + m_stride; return *this; }
    SG_FORCE_INLINE this_type operator++(int) { auto r = *this; ++m_index; return r; }
    SG_FORCE_INLINE this_type& operator--() { m_ptr = reinterpret_cast<u8*>(m_ptr) - m_stride; return *this; }
    SG_FORCE_INLINE this_type operator--(int) { auto r = *this; --m_index; return r; }
    friend SG_FORCE_INLINE bool operator==(this_type const& a, this_type const& b) { SG_ASSERT(a.m_stride == b.m_stride); return a.m_ptr == b.m_ptr; }
    friend SG_FORCE_INLINE bool operator!=(this_type const& a, this_type const& b) { SG_ASSERT(a.m_stride == b.m_stride); return a.m_ptr != b.m_ptr; }
    friend SG_FORCE_INLINE difference_type operator-(this_type const& a, this_type const& b) { SG_ASSERT(a.m_stride == b.m_stride); ptrdiff_t const d = reinterpret_cast<u8*>(b.m_ptr) - reinterpret_cast<u8*>(a.m_ptr); SG_ASSERT((diff % a.m_stride) == 0) return diff / m_stride; }
    SG_FORCE_INLINE this_type& operator+=(difference_type& delta) { m_ptr = reinterpret_cast<u8*>(m_ptr) + delta * m_stride; return this; }
    SG_FORCE_INLINE this_type& operator-=(difference_type& delta) { m_ptr = reinterpret_cast<u8*>(m_ptr) - delta * m_stride; return this; }
    SG_FORCE_INLINE this_type operator+(difference_type& delta) { this_type r = *this; r += delta; return r; }
    SG_FORCE_INLINE this_type operator-(difference_type& delta) { this_type r = *this; r -= delta; return r; }
    friend SG_FORCE_INLINE this_type operator+(difference_type& delta, this_type it) { return it += delta; }
    friend SG_FORCE_INLINE this_type operator-(difference_type& delta, this_type it) { return it -= delta; }
private:
    void* m_ptr;
    size_t m_stride;
};
//=============================================================================
template<typename T>
class StridedArrayView
{
public:
    typedef T value_type;
    typedef typename ConstPassing<T>::type value_type_for_const_passing;

    typedef StridedArrayViewIterator<T, false> iterator;
    typedef StridedArrayViewIterator<T, true> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> reverse_const_iterator;
public:
    SG_FORCE_INLINE StridedArrayView(void* iBuffer, size_t iSize, size_t iStride) : m_data(iBuffer), m_size(iSize), m_stride(iStride) {}

    SG_FORCE_INLINE value_type& operator[] (size_t i) { SG_ASSERT(i < m_size); return *reinterpret_cast<value_type*>(reinterpret_cast<u8*>(m_data) + i * m_stride); }
    SG_FORCE_INLINE value_type_for_const_passing operator[] (size_t i) const { SG_ASSERT(i < m_size); return *reinterpret_cast<value_type const*>(reinterpret_cast<u8 const*>(m_data) + i * m_stride); }
    SG_FORCE_INLINE value_type& Front () { SG_ASSERT(0 != m_size); return (*this)[0]; }
    SG_FORCE_INLINE value_type& Back () { SG_ASSERT(0 != m_size); return (*this)[m_size - 1]; }
    SG_FORCE_INLINE value_type_for_const_passing Front () const { SG_ASSERT(0 != m_size); return (*this)[0]; }
    SG_FORCE_INLINE value_type_for_const_passing Back () const { SG_ASSERT(0 != m_size); return (*this)[m_size - 1]; }

    SG_FORCE_INLINE void* StridedData() { return m_data; }
    SG_FORCE_INLINE void const* Data() const { return m_data; }
    SG_FORCE_INLINE size_t Size() const { return m_size; }
    SG_FORCE_INLINE bool Empty() const { return 0 == m_size; }

#if SG_CONTAINERS_EXPOSE_STD_NAMES
    SG_FORCE_INLINE value_type& front () { return Front(); }
    SG_FORCE_INLINE value_type& back () { return Back(); }
    SG_FORCE_INLINE value_type_for_const_passing front () const { return Front(); }
    SG_FORCE_INLINE value_type_for_const_passing back () const { return Back(); }

    SG_FORCE_INLINE size_t size() const { return m_size; }
    SG_FORCE_INLINE bool empty() const { return 0 == m_size; }
#endif

    SG_FORCE_INLINE iterator begin() { return iterator(m_data, m_stride); }
    SG_FORCE_INLINE iterator end() { return iterator(reinterpret_cast<u8*>(m_data) + m_size * m_stride, m_stride); }
    SG_FORCE_INLINE const_iterator begin() const { return const_iterator(m_data, m_stride);; }
    SG_FORCE_INLINE const_iterator end() const { return const_iterator(reinterpret_cast<u8*>(m_data) + m_size * m_stride, m_stride); }
    SG_FORCE_INLINE reverse_iterator rbegin() { return reverse_iterator(end()); }
    SG_FORCE_INLINE reverse_iterator rend() { return reverse_iterator(begin()); }
    SG_FORCE_INLINE reverse_const_iterator rbegin() const { return reverse_const_iterator(end()); }
    SG_FORCE_INLINE reverse_const_iterator rend() const { return reverse_const_iterator(begin()); }
protected:
    void* m_data;
    size_t m_size;
    size_t m_stride;
};
//=============================================================================
}

#endif

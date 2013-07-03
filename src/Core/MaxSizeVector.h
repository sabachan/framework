#ifndef Core_MaxSizeVector_H
#define Core_MaxSizeVector_H

#include "Assert.h"
#include "Config.h"
#include "For.h"
#include "IntTypes.h"
#include <type_traits>

namespace sg {
//=============================================================================
// TODO: Deprecate for a MaxSizeArrayList implemented over ArrayList
template <typename T, size_t N>
class MaxSizeVector
{
    SG_NON_COPYABLE(MaxSizeVector)
    //MaxSizeVector(MaxSizeVector const& other) = delete;
    //MaxSizeVector& operator=(MaxSizeVector const& other) = delete;
public:
    typedef T const* const_iterator;
    typedef T* iterator;
    typedef std::reverse_iterator<T const*> const_reverse_iterator;
    typedef std::reverse_iterator<T*> reverse_iterator;
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
public:
    MaxSizeVector() : m_size(0) SG_CODE_FOR_ASSERT(SG_COMMA m_visualization(reinterpret_cast<value_type*>(m_data))) {}
    ~MaxSizeVector() {}

    value_type& operator[] (size_t i) { SG_ASSERT(i < m_size); return reinterpret_cast<value_type*>(m_data)[i]; }
    value_type_for_const_passing operator[] (size_t i) const { SG_ASSERT(i < m_size); return reinterpret_cast<value_type const*>(m_data)[i]; }
    value_type& front() { SG_ASSERT(0 < m_size); return reinterpret_cast<value_type*>(m_data)[0]; }
    value_type_for_const_passing front() const { SG_ASSERT(0 < m_size); return reinterpret_cast<value_type const*>(m_data)[0]; }
    value_type& back() { SG_ASSERT(0 < m_size); return reinterpret_cast<value_type*>(m_data)[m_size-1]; }
    value_type_for_const_passing back() const { SG_ASSERT(0 < m_size); return reinterpret_cast<value_type const*>(m_data)[m_size-1]; }

    size_t capacity() const { return N; }
    value_type* data() { return reinterpret_cast<value_type*>(m_data); }
    value_type const* data() const { return reinterpret_cast<value_type const*>(m_data); }
    size_t size() const { return m_size; }
    bool empty() const { return 0 == m_size; }
    iterator begin() { return reinterpret_cast<value_type*>(m_data); }
    iterator end() { return reinterpret_cast<value_type*>(m_data)+m_size; }
    const_iterator begin() const { return reinterpret_cast<value_type const*>(m_data); }
    const_iterator end() const { return reinterpret_cast<value_type const*>(m_data)+m_size; }
    reverse_iterator rbegin() { return reverse_iterator(reinterpret_cast<value_type const*>(m_data)+m_size); }
    reverse_iterator rend() { return reverse_iterator(reinterpret_cast<value_type const*>(m_data)); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(reinterpret_cast<value_type const*>(m_data)+m_size); }
    const_reverse_iterator rend() const { return const_reverse_iterator(reinterpret_cast<value_type const*>(m_data)); }

    template <typename... U>
    void emplace_back(U... iArgs);
    void push_back(value_type_for_const_passing iValue);
    // TODO when value_type_for_const_passing is not value_type:
    //void push_back(value_type&& iValue);
    void pop_back();
    void clear();
private:
    u8 m_data[N * sizeof(T)];
    size_t m_size;
#if SG_ENABLE_ASSERT
    T* m_visualization; // TODO: remove and make a debug viewer for VS
#endif
};
//=============================================================================
template <typename T, size_t N>
template <typename... U>
void MaxSizeVector<T, N>::emplace_back(U... iArgs)
{
    SG_ASSERT(m_size < N);
    new(m_data + m_size * sizeof(T)) value_type(iArgs...);
    ++m_size;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t N>
void MaxSizeVector<T, N>::push_back(value_type_for_const_passing iValue)
{
    SG_ASSERT(m_size < N);
    new(m_data + m_size * sizeof(T)) value_type(iValue);
    ++m_size;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//template <typename T, size_t N>
//void MaxSizeVector<T, N>::push_back(value_type&& iValue)
//{
//    SG_ASSERT(m_size < N);
//    new(m_data + m_size * sizeof(T)) value_type();
//    using std::swap;
//    swap(reinterpret_cast<value_type*>(m_data)[m_size], iValue);
//    ++m_size;
//}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t N>
void MaxSizeVector<T, N>::pop_back()
{
    SG_ASSERT(m_size > 0);
    reinterpret_cast<value_type*>(m_data)[m_size-1].~T();
    --m_size;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t N>
void MaxSizeVector<T, N>::clear()
{
    SG_ASSERT(m_size <= N);
    for(size_t i = 1; i <= m_size; ++i)
    {
        reinterpret_cast<value_type*>(m_data)[m_size-i].~T();
    }
    m_size = 0;
}
//=============================================================================
}

#endif

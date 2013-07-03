#ifndef Core_IndexedArrayView_H
#define Core_IndexedArrayView_H

#include "ArrayView.h"
#include "Assert.h"
#include <type_traits>


namespace sg {
//=============================================================================
template <typename T, typename I, bool IsConst>
class IndexedArrayViewIterator
{
    typedef IndexedArrayViewIterator this_type;
public:
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef std::conditional<IsConst, value_type const, value_type>::type* pointer;
    typedef std::conditional<IsConst, value_type const, value_type>::type& reference;
    typedef random_access_iterator_tag iterator_category;
public:
    IndexedArrayViewIterator() : m_index(nullptr), m_array(nullptr) {}
    IndexedArrayViewIterator(I* index, ArrayView<T>* array) : m_index(index), m_array(array) {}
    T& operator *() const { return (*m_array)[*m_index]; }
    T* operator ->() const { return &(*m_array)[*m_index]; }
    T* get() const { return &(*m_array)[*m_index]; }
    this_type& operator++() { ++m_index; return *this; }
    this_type operator++(int) { auto r = *this; ++m_index; return r; }
    this_type& operator--() { --m_index; return *this; }
    this_type operator--(int) { auto r = *this; --m_index; return r; }
    friend bool operator==(this_type const& a, this_type const& b) { ASSERT(a.m_array == b.m_array); return a.m_index == b.m_index; }
    friend bool operator!=(this_type const& a, this_type const& b) { ASSERT(a.m_array == b.m_array); return a.m_index != b.m_index; }
    friend difference_type operator-(this_type const& a, this_type const& b) { ASSERT(a.m_array == b.m_array); return b.m_index - a.m_index; }
    this_type& operator+=(difference_type& delta) { m_index += delta; return this; }
    this_type& operator-=(difference_type& delta) { m_index -= delta; return this; }
    this_type operator+(difference_type& delta) { this_type r = *this; r.m_index += delta; return r; }
    this_type operator-(difference_type& delta) { this_type r = *this; r.m_index -= delta; return r; }
    friend this_type operator+(difference_type& delta, this_type it) { return it += delta; }
    friend this_type operator-(difference_type& delta, this_type it) { return it -= delta; }
private:
    I* m_index;
    ArrayView<T>* m_array;
};
//=============================================================================
template <typename T, typename I, bool InheritConstness = false>
class IndexedArrayView
{
    static_assert(std::is_integral<I>::value, "I must be an integer type");
    typedef I index_type;
    typedef std::conditional<InheritConstness, I const, I> index_type_when_const;
    typedef T value_type;
    typedef std::conditional<InheritConstness, T const, T> value_type_when_const;
public:
    typedef IndexedArrayViewIterator<T, I, InheritConstness> const_iterator;
    typedef IndexedArrayViewIterator<T, I, false> iterator;
public:
    IndexedArrayView() : m_data(nullptr), m_indices(nullptr) {}
    IndexedArrayView(nullptr_t) : m_data(nullptr), m_indices(nullptr) {}
    IndexedArrayView(ArrayView<T> const& data, ArrayView<I> const& indices) : m_data(data), m_indices(indices) {}
    IndexedArrayView(IndexedArrayView const& other) : m_data(other.m_data), m_indices(other.m_indices) {}
    IndexedArrayView& operator=(IndexedArrayView const& other) { m_data = other.m_data; m_size = other.m_size; return *this; }
    ~IndexedArrayView() {}

    value_type_when_const& operator[] (size_t i) const { size_t const index = m_indices[i]; return m_data[index]; }
    value_type_when_const& front () const { return m_data[m_indices.front()]; }
    value_type_when_const& back () const { return m_data[m_indices.back()]; }

    value_type_when_const* data() const { return m_data.data(); }
    I* indices() const { return m_indices.data(); }
    size_t size() const { return m_indices.size; }
    bool empty() const { return m_indices.empty(); }
    iterator begin() { return iterator(m_indices.data(), &m_data); }
    iterator end() { return iterator(m_indices.data()+m_indices.size(), &m_data); }
    const_iterator begin() const { return const_iterator(m_indices.data(), &m_data); }
    const_iterator end() const { return const_iterator(m_indices.data()+m_indices.size(), &m_data); }
private:
    ArrayView<T> m_data;
    ArrayView<I> m_indices;
};
//=============================================================================
}

#endif

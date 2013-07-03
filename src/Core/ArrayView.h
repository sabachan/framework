#ifndef Core_ArrayView_H
#define Core_ArrayView_H

#include "Assert.h"
#include "TemplateUtils.h"
#include <vector>

namespace sg {
//=============================================================================
template<typename T> class ArrayView;
//=============================================================================
namespace internal {
template<typename T, bool InheritsConstness, typename S>
class ArrayBase
{
    static_assert(std::is_same<S, size_t>::value || std::is_same<S, size_t&>::value, "");
public:
    typedef T value_type;
    typedef typename std::conditional<InheritsConstness, T const, T>::type value_type_when_const;
    typedef T* iterator;
    typedef typename std::conditional<InheritsConstness, T const*, T*>::type iterator_when_const;
    typedef std::reverse_iterator<T*> reverse_iterator;
    typedef typename std::conditional<InheritsConstness, std::reverse_iterator<T const*>, std::reverse_iterator<T*>>::type reverse_iterator_when_const;
    typedef S size_type;
public:
    value_type& operator[] (size_t i) { SG_ASSERT(i < m_size); return m_data[i]; }
    value_type_when_const& operator[] (size_t i) const { SG_ASSERT(i < m_size); return m_data[i]; }
    value_type& Front () { SG_ASSERT(0 != m_size); return m_data[0]; }
    value_type& Back () { SG_ASSERT(0 != m_size); return m_data[m_size - 1]; }
    value_type_when_const& Front () const { SG_ASSERT(0 != m_size); return m_data[0]; }
    value_type_when_const& Back () const { SG_ASSERT(0 != m_size); return m_data[m_size - 1]; }

    value_type* Data() { return m_data; }
    value_type_when_const* Data() const { return m_data; }
    size_t Size() const { return m_size; }
    bool Empty() const { return 0 == m_size; }

#if SG_CONTAINERS_EXPOSE_STD_NAMES
    value_type& front () { return Front(); }
    value_type& back () { return Back(); }
    value_type_when_const& front () const { return Front(); }
    value_type_when_const& back () const { return Back(); }

    value_type* data() { return m_data; }
    value_type_when_const* data() const { return m_data; }
    size_t size() const { return m_size; }
    bool empty() const { return 0 == m_size; }
#endif

    iterator begin() { return m_data; }
    iterator end() { return m_data+m_size; }
    iterator_when_const begin() const { return m_data; }
    iterator_when_const end() const { return m_data+m_size; }
    reverse_iterator rbegin() { return reverse_iterator(m_data+m_size); }
    reverse_iterator rend() { return reverse_iterator(m_data); }
    reverse_iterator_when_const rbegin() const { return reverse_iterator_when_const(m_data+m_size); }
    reverse_iterator_when_const rend() const { return reverse_iterator_when_const(m_data); }

    ArrayView<value_type> View() { return ArrayView<value_type>(m_data, m_size); }
    ArrayView<value_type_when_const> View() const { return ArrayView<value_type_when_const>(m_data, m_size); }
    ArrayView<value_type const> ConstView() const { return ArrayView<value_type const>(m_data, m_size); }
protected:
    ArrayBase(T* data, size_type size) : m_data(data), m_size(size) {}
protected:
    T* m_data;
    size_type m_size;
};
}
//=============================================================================
template<typename T>
class ArrayView : public internal::ArrayBase<T, false, size_t>
{
public:
    ArrayView() : ArrayBase(nullptr, 0) {}
    ArrayView(nullptr_t) : ArrayBase(nullptr, 0) {}
    ArrayView(T* data, size_t size) : ArrayBase(data, size) {}
    ArrayView(ArrayView const& other) : ArrayBase(other.m_data, other.m_size) {}
    template <typename U, typename = typename std::enable_if<std::is_const<T>::value && std::is_same<typename std::remove_const<T>::type, U>::value, U>::type>
    ArrayView(ArrayView<U> const& other) : ArrayBase(other.data(), other.size()) {}
    ArrayView& operator=(ArrayView const& other) { m_data = other.m_data; m_size = other.m_size; return *this; }
};
//=============================================================================
template<typename T, int N>
ArrayView<T> AsArrayView(T (&a) [N])
{
    return ArrayView<T>(a, N);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
ArrayView<T> AsArrayView(T* a, size_t N)
{
    return ArrayView<T>(a, N);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayView<T> AsArrayView(std::vector<T, A>& v)
{
    return ArrayView<T>(v.data(), v.size());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayView<T const> AsArrayView(std::vector<T, A> const& v)
{
    return ArrayView<T const>(v.data(), v.size());
}
//=============================================================================
#if 0
template<typename T>
class ArrayViewNoSize
{
public:
    ArrayViewNoSize() : m_data(nullptr) SG_CODE_FOR_ASSERT(SG_COMMA m_size(0)) {}
    ArrayViewNoSize(T* data, size_t size) : m_data(data) SG_CODE_FOR_ASSERT(SG_COMMA m_size(size)) {}
    ArrayViewNoSize(ArrayView const& other) : m_data(other.m_data) SG_CODE_FOR_ASSERT(SG_COMMA m_size(other.m_size)) {}
    ArrayViewNoSize& operator=(ArrayView const& other) { m_data = other.m_data; SG_CODE_FOR_ASSERT(m_size = other.m_size;) return *this; }
    ~ArrayViewNoSize() {}

    T& operator[] (size_t i) const { SG_ASSERT(i < m_size); return m_data[i]; }

    T* data() const { return m_data; }
    bool empty() const { return nullptr == m_data; }
private:
    T* m_data;
    SG_CODE_FOR_ASSERT(size_t m_size;)
};
#endif
//=============================================================================
}

#endif

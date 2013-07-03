#ifndef Core_VectorOfScopedPtr_H
#define Core_VectorOfScopedPtr_H

#include "Assert.h"
#include "For.h"
#include "SmartPtr.h"
#include <vector>

namespace sg {
//=============================================================================
template <typename T>
class VectorOfScopedPtr
{
    SG_NON_COPYABLE(VectorOfScopedPtr)
public:
    typedef typename std::vector<T*>::const_iterator const_iterator;
    typedef typename std::vector<T*>::const_reverse_iterator const_reverse_iterator;
public:
    VectorOfScopedPtr() : m_vector() {}
    VectorOfScopedPtr(size_t iSize) : m_vector(iSize, nullptr) {}
    VectorOfScopedPtr(VectorOfScopedPtr&& iOther) : m_vector(std::move(iOther.m_vector)) { SG_ASSERT(iOther.empty()); }
    VectorOfScopedPtr& operator=(VectorOfScopedPtr&& iOther) { clear(); m_vector = std::move(iOther.m_vector); SG_ASSERT(iOther.empty()); return *this; }
    ~VectorOfScopedPtr() { clear(); }

    void clear()
    {
        for(auto it : m_vector)
            delete it;
        m_vector.clear();
    }
    void reserve(size_t iN) { m_vector.reserve(iN); }
    void resize(size_t iN)
    {
        size_t const size = m_vector.size();
        if(iN < size)
        {
            for_range(size_t, i, iN, size)
                delete m_vector[i];
        }
        m_vector.resize(iN, nullptr);
    }
    void push_back(T* iPtr) { m_vector.push_back(iPtr); }
    void emplace_back() { m_vector.emplace_back(); }
    void pop_back() { SG_ASSERT(!m_vector.empty()); delete m_vector.back(); m_vector.pop_back(); }
    T* operator[] (size_t i) const { return m_vector[i]; }
    void reset(size_t i, T* iPtr)
    {
        T*& item = m_vector[i];
        delete item;
        item = iPtr;
    }
    const_iterator begin() const { return m_vector.begin(); }
    const_iterator end() const { return m_vector.end(); }
    const_reverse_iterator rbegin() const { return m_vector.rbegin(); }
    const_reverse_iterator rend() const { return m_vector.rend(); }
    T* back() const { return m_vector.back(); }
    T* front() const { return m_vector.front(); }
    T* const* data() const { return m_vector.data(); }
    size_t capacity() const { return m_vector.capacity(); }
    size_t size() const { return m_vector.size(); }
    bool empty() const { return m_vector.empty(); }
private:
    std::vector<T*> m_vector;
};
//=============================================================================
}

#endif

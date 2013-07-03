#ifndef Core_CircularBuffer_H
#define Core_CircularBuffer_H

// WORK IN PROGRESS

namespace sg {
//=============================================================================
template <typename T, size_t N>
class CircularBuffer
{
    static_assert((N & (N-1)) == 0, "N must be power of 2");
    static const size_t capacity = N;
    static const size_t indexation_mask = N-1;
public:
    CircularBuffer()
        : m_begin(0)
        , m_size(0)
    {
    }
    ~CircularBuffer()
    {
    }
    size_t size() const { return m_size; }
    bool empty() const { return 0 == m_size; }
    void push_back(T const& v)
    {
        SG_ASSERT(m_size < capacity);
        m_buffer[(m_begin + m_size) & indexation_mask] = v;
    }
    T& push_back_return_uninitialized()
    {
        SG_ASSERT(m_size < capacity);
        m_size++;
        return m_buffer[(m_begin + m_size - 1) & indexation_mask];
    }
    void pop_back(size_t count = 1)
    {
        SG_ASSERT(m_size >= count);
        m_size -= count;
    }
    void pop_front(size_t count = 1)
    {
        SG_ASSERT(m_size >= count);
        m_size -= count;
        m_begin = (m_begin + count) & indexation_mask;
    }
    T& back()
    {
        return m_buffer[(m_begin + m_size - 1) & indexation_mask];
    }
    T const& back() const
    {
        return m_buffer[(m_begin + m_size - 1) & indexation_mask];
    }
    T& front()
    {
        SG_ASSERT(capacity > m_begin);
        return m_buffer[m_begin];
    }
    T const& front() const
    {
        SG_ASSERT(capacity > m_begin);
        return m_buffer[m_begin];
    }
    T const& operator[](int i) const
    {
        SG_ASSERT(-m_size <= i);
        SG_ASSERT(i < m_size);
        if(i >= 0)
            return m_buffer[(m_begin + i) & indexation_mask];
        else
            return m_buffer[(m_begin + m_size + i) & indexation_mask];
    }
    T& operator[](int i)
    {
        SG_ASSERT(-m_size <= i);
        SG_ASSERT(i < m_size);
        if(i >= 0)
            return m_buffer[(m_begin + i) & indexation_mask];
        else
            return m_buffer[(m_begin + m_size + i) & indexation_mask];
    }
private:
    T m_buffer[N];
    size_t m_begin;
    size_t m_size;
};
//=============================================================================
}

#endif

#ifndef Core_IntrusiveList_H
#define Core_IntrusiveList_H

#include "Assert.h"
#include "Config.h"
#include "SmartPtr.h"
#include <type_traits>

// Here is a set of tool classes that allow to create double linked list of
// elements using intrusive pointers in elements.
// It is far better than a standard list, which needs to create node objects,
// as it does not create new object and need less pointer indirections.
// The only drawback is that an object can only be present in one lust "type".
//
// Please note that most function names differs from std::vector. This is to
// emphasize that they take and return elements by pointer instead of by
// reference. However, iterators directly point to the value, not the pointer.
//
// In the following classes,
// idCount      is the number of list "types" that an object can be added to.
// id           is an identifier of a list "type".
//
// Note that a list must know its list id at compile time.

namespace sg {
//=============================================================================
template <typename T, size_t idCount> class IntrusiveListImpl;
template <typename T, size_t idCount> class IntrusiveListElem;
//=============================================================================
template <typename elem_type, size_t idCount>
struct IntrusiveListIteratorMembers
{
    static_assert(0 < idCount, "idCount must be greater or equal than 1");
    IntrusiveListIteratorMembers(elem_type* iElem, size_t iId) : elem(iElem), id(iId) { SG_ASSERT(id < idCount); }
    safeptr<elem_type> elem;
    size_t id;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename elem_type>
struct IntrusiveListIteratorMembers<elem_type, 1>
{
    IntrusiveListIteratorMembers(elem_type* iElem, size_t iId) : elem(iElem) { SG_ASSERT_AND_UNUSED(0 == iId);  }
    safeptr<elem_type> elem;
    static size_t const id = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount, size_t dir>
class IntrusiveListIterator
{
    static_assert(0 <= dir && dir <= 1, "dir must be 0 or 1");
    friend class IntrusiveListImpl<T, idCount>;
    typedef IntrusiveListElem<T, idCount> elem_type;
    size_t const rev = 1 - dir;
public:
    IntrusiveListIterator() : m_members(iOther.m_members.elem.get(), 0) {}
    IntrusiveListIterator(IntrusiveListIterator const& iOther) : m_members(iOther.m_members.elem.get(), iOther.m_members.id) {}
    IntrusiveListIterator operator=(IntrusiveListIterator const& iOther) { m_elem = iOther.m_elem; return *this; }
    IntrusiveListIterator operator++()
    {
        SG_ASSERT(nullptr != m_members.elem);
        m_members.elem = m_members.elem->m_nexts[m_members.id][dir];
        return *this;
    }
    IntrusiveListIterator operator++(int) { IntrusiveListIterator r = *this; ++*this; return r; }
    IntrusiveListIterator operator--()
    {
        SG_ASSERT(nullptr != m_members.elem);
        m_members.elem = m_members.elem->m_nexts[m_members.id][rev];
        return *this;
    }
    IntrusiveListIterator operator--(int) { IntrusiveListIterator r = *this; --*this; return r; }
    T& operator*() { SG_ASSERT(nullptr != m_members.elem); return *static_cast<T*>(m_members.elem.get()); }
    T* operator->() { SG_ASSERT(nullptr != m_members.elem); return static_cast<T*>(m_members.elem.get());  }
    T* get() { return static_cast<T*>(m_members.elem.get()); }

    friend bool operator== (IntrusiveListIterator const& a, IntrusiveListIterator const& b) { return a.m_members.elem == b.m_members.elem; }
    friend bool operator!= (IntrusiveListIterator const& a, IntrusiveListIterator const& b) { return !(a == b); }
private:
    IntrusiveListIterator(elem_type* iElem, size_t iId) : m_members(iElem, iId) {}
private:
    IntrusiveListIteratorMembers<elem_type, idCount> m_members;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount = 1>
class IntrusiveListElem : public SafeCountable
{
    friend class IntrusiveListImpl<T, idCount>;
    friend class IntrusiveListIterator<T, idCount, 0>;
    friend class IntrusiveListIterator<T, idCount, 1>;
    typedef IntrusiveListImpl<T, idCount> list_type;
    typedef IntrusiveListElem<T, idCount> elem_type;
public:
#if SG_ENABLE_ASSERT
    IntrusiveListElem() : m_inListcount(0) { for(size_t i=0; i<idCount; ++i) { m_parent[i] = nullptr; } }
    ~IntrusiveListElem() { SG_ASSERT(0 == m_inListcount); }
#endif
public:
    void OnAddedToIntrusiveList()
    {
#if SG_ENABLE_ASSERT
        SG_ASSERT(m_inListcount < idCount);
        ++m_inListcount;
        // Note that it is not necessary to update safe count as m_inListCount
        // already checks a stronger assertion.
#endif
    }
    void OnRemovedFromIntrusiveList()
    {
#if SG_ENABLE_ASSERT
        SG_ASSERT(0 != m_inListcount);
        --m_inListcount;
#endif
    }
    list_type* ParentListIFP(size_t id = 0) const
    {
        SG_ASSERT(id < idCount);
        return m_parent[id].get();
    }
private:
    safeptr<elem_type> m_nexts[idCount][2];
    safeptr<list_type> m_parent[idCount];
    SG_CODE_FOR_ASSERT(size_t m_inListcount;)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount = 1>
class IntrusiveListRefCountableElem : public IntrusiveListElem<T, idCount>
{
    typedef IntrusiveListElem<T, idCount> parent_type;
public:
    void OnAddedToIntrusiveList()
    {
        parent_type::OnAddedToIntrusiveList();
        T* elem = static_cast<T*>(this);
        elem->GetAsRefCountable()->IncRef();
    }
    void OnRemovedFromIntrusiveList()
    {
        parent_type::OnRemovedFromIntrusiveList();
        T* elem = static_cast<T*>(this);
        bool mustdelete = elem->GetAsRefCountable()->DecRefReturnMustBeDeleted();
        if(mustdelete)
        {
            delete elem;
        }
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename elem_type, size_t idCount>
struct IntrusiveListMembers
{
    static_assert(0 < idCount, "idCount must be greater or equal than 1");
    IntrusiveListMembers(size_t iId) : id(iId) { heads[0] = nullptr; heads[1] = nullptr; SG_ASSERT(id < idCount); }
    safeptr<elem_type> heads[2];
    size_t id;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename elem_type>
struct IntrusiveListMembers<elem_type, 1>
{
    IntrusiveListMembers(size_t iId) { SG_ASSERT_AND_UNUSED(id == iId); heads[0] = nullptr; heads[1] = nullptr; }
    safeptr<elem_type> heads[2];
    static size_t const id = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount = 1>
class IntrusiveListImpl : public SafeCountable
{
    static_assert(0 < idCount, "idCount must be greater or equal than 1");
    static_assert(!std::is_const<T>::value, "T must not be const");
    static_assert(std::is_base_of<IntrusiveListElem<T, idCount>, T>::value, "T must be the type of the derivated class that is to be listable");
public:
    typedef IntrusiveListIterator<T, idCount, 0> iterator;
    typedef IntrusiveListIterator<T const, idCount, 0> const_iterator;
    typedef IntrusiveListIterator<T, idCount, 1> reverse_iterator;
    typedef IntrusiveListIterator<T const, idCount, 1> const_reverse_iterator;
    typedef IntrusiveListElem<T, idCount> elem_type;
    typedef T value_type;
protected:
    IntrusiveListImpl(size_t iId) : m_members(iId) {}
    ~IntrusiveListImpl() { Clear(); }
public:
    void Clear();
    bool Contains(value_type* iElem) const { SG_ASSERT(nullptr != iElem); elem_type* elem = static_cast<elem_type*>(iElem); return Contains(elem); }
    bool Empty() const { SG_ASSERT(nullptr != m_members.heads[0] || nullptr == m_members.heads[1]); return nullptr == m_members.heads[0]; }
    void PushBack(value_type* iToInsert);
    void PushFront(value_type* iToInsert);
    void InsertAfter(value_type* iElem, value_type* iToInsert);
    void InsertBefore(value_type* iElem, value_type* iToInsert);
    void MoveToBack(value_type* iToMove);
    void MoveToFront(value_type* iToMove);
    void MoveToAfter(value_type* iElem, value_type* iToMove);
    void MoveToBefore(value_type* iElem, value_type* iToMove);
    void Remove(value_type* iToRemove);
    void PopBack();
    void PopFront();
    value_type* After(value_type* iElem);
    value_type* Before(value_type* iElem);
    value_type* Back() { return rbegin().get(); }
    value_type const* Back() const { return rbegin().get(); }
    value_type* Front() { return begin().get(); }
    value_type const* Front() const { return begin().get(); }

    iterator begin() { return iterator(m_members.heads[0].get(), m_members.id); }
    iterator end() { size_t const id = 1 == idCount ? 0 : m_members.id; return iterator(nullptr, id); }
    const_iterator begin() const { size_t const id = 1 == idCount ? 0 : m_members.id; return const_iterator(m_members.heads[0].get(), id); }
    const_iterator end() const { size_t const id = 1 == idCount ? 0 : m_members.id; return const_iterator(nullptr, id); }
    reverse_iterator rbegin() { size_t const id = 1 == idCount ? 0 : m_members.id; return reverse_iterator(m_members.heads[1].get(), id); }
    reverse_iterator rend() { size_t const id = 1 == idCount ? 0 : m_members.id; return reverse_iterator(nullptr, id); }
    const_reverse_iterator rbegin() const { size_t const id = 1 == idCount ? 0 : m_members.id; return reverse_const_iterator(m_members.heads[1].get(), id); }
    const_reverse_iterator rend() const { size_t const id = 1 == idCount ? 0 : m_members.id; return reverse_const_iterator(nullptr, id); }
private:
    bool Contains(elem_type* iElem) const { SG_ASSERT(nullptr != iElem); return this == iElem->ParentListIFP(m_members.id); }
    template<size_t dir> void PushHead(elem_type* iToInsert);
    template<size_t dir> void Insert(elem_type* iPosition, elem_type* iToInsert);
    template<size_t dir> void PopHead();
    template<size_t dir> void MoveToHead(elem_type* iToMove);
    template<size_t dir> void MoveInsert(elem_type* iPosition, elem_type* iToMove);
    void Remove(elem_type* iToRemove);
private:
    IntrusiveListMembers<elem_type, idCount> m_members;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t id = 0, size_t idCount = 1>
class IntrusiveList : public IntrusiveListImpl<T, idCount>
{
    static_assert(id < idCount, "id must be less than idCount");
    typedef IntrusiveListImpl<T, idCount> parent_type;
public:
    IntrusiveList() : parent_type(id) {}
    ~IntrusiveList() {}
};
//=============================================================================
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::Clear()
{
#if 1
    while(!Empty())
        PopBack();
#else // Too dangerous for OnAddedToList/OnRemovedFromList behavior
    elem_type* elem = m_first;
    while(nullptr != elem)
    {
        elem_type* next = elem->m_next;
        elem->m_previous = nullptr;
        elem->m_next = nullptr;
        elem->m_parent = nullptr;
        elem = next;
    }
    m_first = nullptr;
    m_last = nullptr;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::PushBack(value_type* iToInsert)
{
    elem_type* toInsert = static_cast<elem_type*>(iToInsert);
    PushHead<1>(toInsert);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::PushFront(value_type* iToInsert)
{
    elem_type* toInsert = static_cast<elem_type*>(iToInsert);
    PushHead<0>(toInsert);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::InsertAfter(value_type* iElem, value_type* iToInsert)
{
    elem_type* elem = static_cast<elem_type*>(iElem);
    SG_ASSERT(Contains(iElem));
    elem_type* toInsert = static_cast<elem_type*>(iToInsert);
    Insert<1>(elem, toInsert);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::InsertBefore(value_type* iElem, value_type* iToInsert)
{
    elem_type* elem = static_cast<elem_type*>(iElem);
    SG_ASSERT(Contains(iElem));
    elem_type* toInsert = static_cast<elem_type*>(iToInsert);
    Insert<0>(elem, toInsert);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::PopBack()
{
    PopHead<1>();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::PopFront()
{
    PopHead<0>();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::Remove(value_type* iToRemove)
{
    elem_type* toRemove = static_cast<elem_type*>(iToRemove);
    Remove(toRemove);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::MoveToBack(value_type* iToMove)
{
    SG_ASSERT(Contains(iToMove));
    elem_type*const toMove = static_cast<elem_type*>(iToMove);
    MoveToHead<1>(toMove);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::MoveToFront(value_type* iToMove)
{
    SG_ASSERT(Contains(iToMove));
    elem_type*const toMove = static_cast<elem_type*>(iToMove);
    MoveToHead<0>(toMove);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::MoveToAfter(value_type* iElem, value_type* iToMove)
{
    SG_ASSERT(Contains(iElem));
    SG_ASSERT(Contains(iToMove));
    elem_type* elem = static_cast<elem_type*>(iElem);
    elem_type* toMove = static_cast<elem_type*>(iToMove);
    MoveInsert<1>(elem, toMove);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::MoveToBefore(value_type* iElem, value_type* iToMove)
{
    SG_ASSERT(Contains(iElem));
    SG_ASSERT(Contains(iToMove));
    elem_type* elem = static_cast<elem_type*>(iElem);
    elem_type* toMove = static_cast<elem_type*>(iToMove);
    MoveInsert<0>(elem, toMove);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
template<size_t dir>
void IntrusiveListImpl<T, idCount>::PushHead(elem_type* iToInsert)
{
    static_assert(0 <= dir && dir <= 1, "dir must be 0 or 1");
    size_t const rev = 1 - dir;
    size_t const id = m_members.id;
    elem_type* toInsert = static_cast<elem_type*>(iToInsert);
    SG_ASSERT(nullptr == toInsert->m_parent[id]);
    if(nullptr == m_members.heads[dir])
    {
        SG_ASSERT(nullptr == m_members.heads[rev]);
        m_members.heads[dir] = toInsert;
        m_members.heads[rev] = toInsert;
        toInsert->m_parent[id] = this;
        static_cast<value_type*>(iToInsert)->OnAddedToIntrusiveList();
    }
    else
    {
        SG_ASSERT(nullptr != m_members.heads[rev]);
        Insert<dir>(m_members.heads[dir].get(), toInsert);
    }
    SG_ASSERT(this == toInsert->m_parent[id]);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
template<size_t dir>
void IntrusiveListImpl<T, idCount>::Insert(elem_type* iPosition, elem_type* iToInsert)
{
    // NB: As with std, inserted element will be before element along direction.
    static_assert(0 <= dir && dir <= 1, "dir must be 0 or 1");
    size_t const rev = 1 - dir;
    size_t const id = m_members.id;
    SG_ASSERT(this == iPosition->m_parent[id]);
    SG_ASSERT(nullptr == iToInsert->m_parent[id]);
    SG_ASSERT(nullptr == iToInsert->m_nexts[id][dir]);
    SG_ASSERT(nullptr == iToInsert->m_nexts[id][rev]);
    iToInsert->m_parent[id] = this;
    iToInsert->m_nexts[id][dir] = iPosition;
    if(nullptr == iPosition->m_nexts[id][rev])
    {
        SG_ASSERT(iPosition == m_members.heads[dir]);
        m_members.heads[dir] = iToInsert;
    }
    else
    {
        SG_ASSERT(iPosition->m_nexts[id][rev]->m_nexts[id][dir] == iPosition);
        iPosition->m_nexts[id][rev]->m_nexts[id][dir] = iToInsert;
        iToInsert->m_nexts[id][rev] = iPosition->m_nexts[id][rev];
    }
    iPosition->m_nexts[id][rev] = iToInsert;
    static_cast<value_type*>(iToInsert)->OnAddedToIntrusiveList();
    SG_ASSERT(this == iToInsert->m_parent[id]);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
template<size_t dir>
void IntrusiveListImpl<T, idCount>::PopHead()
{
    static_assert(0 <= dir && dir <= 1, "dir must be 0 or 1");
    size_t const rev = 1 - dir;
    size_t const id = m_members.id;
    SG_ASSERT(nullptr != m_members.heads[dir]);
    SG_ASSERT(nullptr != m_members.heads[rev]);
    elem_type* elem = m_members.heads[dir].get();
    SG_ASSERT_AND_UNUSED(nullptr == elem->m_nexts[id][rev]);
    Remove(elem);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
void IntrusiveListImpl<T, idCount>::Remove(elem_type* iToRemove)
{
    SG_ASSERT(nullptr != m_members.heads[0]);
    SG_ASSERT(nullptr != m_members.heads[1]);
    size_t const id = m_members.id;
    SG_ASSERT(this == iToRemove->m_parent[id]);
    if(nullptr == iToRemove->m_nexts[id][0])
    {
        SG_ASSERT(iToRemove == m_members.heads[1]);
        if(nullptr == iToRemove->m_nexts[id][1])
        {
            SG_ASSERT(iToRemove == m_members.heads[0]);
            m_members.heads[0] = nullptr;
            m_members.heads[1] = nullptr;
        }
        else
        {
            m_members.heads[1] = iToRemove->m_nexts[id][1];
            iToRemove->m_nexts[id][1]->m_nexts[id][0] = nullptr;
        }
    }
    else if(nullptr == iToRemove->m_nexts[id][1])
    {
        SG_ASSERT(iToRemove == m_members.heads[0]);
        m_members.heads[0] = iToRemove->m_nexts[id][0];
        iToRemove->m_nexts[id][0]->m_nexts[id][1] = nullptr;
    }
    else
    {
        iToRemove->m_nexts[id][0]->m_nexts[id][1] = iToRemove->m_nexts[id][1];
        iToRemove->m_nexts[id][1]->m_nexts[id][0] = iToRemove->m_nexts[id][0];
    }
    iToRemove->m_parent[id] = nullptr;
    iToRemove->m_nexts[id][0] = nullptr;
    iToRemove->m_nexts[id][1] = nullptr;
    static_cast<value_type*>(iToRemove)->OnRemovedFromIntrusiveList();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
template<size_t dir>
void IntrusiveListImpl<T, idCount>::MoveToHead(elem_type* iToMove)
{
    static_assert(0 <= dir && dir <= 1, "dir must be 0 or 1");
    size_t const rev = 1 - dir;
    size_t const id = m_members.id;
    SG_ASSERT(this == iToMove->m_parent[id]);
    SG_ASSERT(nullptr != m_members.heads[dir]);
    SG_ASSERT(nullptr != m_members.heads[rev]);
    if(iToMove != m_members.heads[dir])
    {
        if(iToMove == m_members.heads[rev])
        {
            SG_ASSERT(nullptr == iToMove->m_nexts[id][dir]);
            m_members.heads[rev] = iToMove->m_nexts[id][rev];
            SG_ASSERT(iToMove == iToMove->m_nexts[id][rev]->m_nexts[id][dir]);
            iToMove->m_nexts[id][rev]->m_nexts[id][dir] = nullptr;
        }
        else
        {
            SG_ASSERT(iToMove == iToMove->m_nexts[id][rev]->m_nexts[id][dir]);
            iToMove->m_nexts[id][rev]->m_nexts[id][dir] = iToMove->m_nexts[id][dir];
            SG_ASSERT(iToMove == iToMove->m_nexts[id][dir]->m_nexts[id][rev]);
            iToMove->m_nexts[id][dir]->m_nexts[id][rev] = iToMove->m_nexts[id][rev];
        }
        iToMove->m_nexts[id][dir] = m_members.heads[dir];
        iToMove->m_nexts[id][rev] = nullptr;
        SG_ASSERT(nullptr == m_members.heads[dir]->m_nexts[id][rev]);
        m_members.heads[dir]->m_nexts[id][rev] = iToMove;
        m_members.heads[dir] = iToMove;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
template<size_t dir>
void IntrusiveListImpl<T, idCount>::MoveInsert(elem_type* iPosition, elem_type* iToMove)
{
    static_assert(0 <= dir && dir <= 1, "dir must be 0 or 1");
    size_t const rev = 1 - dir;
    size_t const id = m_members.id;
    SG_ASSERT(this == iToMove->m_parent[id]);
    SG_ASSERT(this == iPosition->m_parent[id]);
    SG_ASSERT(nullptr != m_members.heads[dir]);
    SG_ASSERT(nullptr != m_members.heads[rev]);
    SG_ASSERT(iPosition != iToMove);
    if(iToMove->m_nexts[id][dir] != iPosition)
    {
        SG_ASSERT(iToMove != iPosition->m_nexts[id][rev]);
        if(iToMove == m_members.heads[dir])
        {
            elem_type*const next = iToMove->m_nexts[id][dir].get();
            SG_ASSERT(nullptr == iToMove->m_nexts[id][rev]);
            m_members.heads[dir] = next;
            SG_ASSERT(iToMove == next->m_nexts[id][rev]);
            next->m_nexts[id][rev] = nullptr;
        }
        else if(iToMove == m_members.heads[rev])
        {
            elem_type*const next = iToMove->m_nexts[id][rev].get();
            SG_ASSERT(nullptr == iToMove->m_nexts[id][dir]);
            m_members.heads[rev] = next;
            SG_ASSERT(iToMove == next->m_nexts[id][dir]);
            next->m_nexts[id][dir] = nullptr;
        }
        else
        {
            elem_type*const dirnext = iToMove->m_nexts[id][dir].get();
            elem_type*const revnext = iToMove->m_nexts[id][rev].get();
            SG_ASSERT(iToMove == revnext->m_nexts[id][dir]);
            SG_ASSERT(iToMove == dirnext->m_nexts[id][rev]);
            revnext->m_nexts[id][dir] = dirnext;
            dirnext->m_nexts[id][rev] = revnext;
        }

        elem_type*const prevpos = iPosition->m_nexts[id][rev].get();
        if(nullptr == prevpos)
        {
            SG_ASSERT(m_members.heads[dir] == iPosition);
            m_members.heads[dir] = iToMove;
        }
        else
        {
            SG_ASSERT(prevpos->m_nexts[id][dir] == iPosition);
            prevpos->m_nexts[id][dir] = iToMove;
        }

        iToMove->m_nexts[id][rev] = prevpos;
        iToMove->m_nexts[id][dir] = iPosition;
        iPosition->m_nexts[id][rev] = iToMove;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
typename IntrusiveListImpl<T, idCount>::value_type* IntrusiveListImpl<T, idCount>::After(value_type* iElem)
{
    SG_ASSERT(nullptr != iElem);
    size_t const id = m_members.id;
    elem_type*const elem = static_cast<elem_type*>(iElem);
    SG_ASSERT(this == elem->m_parent[id]);
    auto it = iterator(elem, id);
    ++it;
    return it.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t idCount>
typename IntrusiveListImpl<T, idCount>::value_type* IntrusiveListImpl<T, idCount>::Before(value_type* iElem)
{
    SG_ASSERT(nullptr != iElem);
    size_t const id = m_members.id;
    elem_type*const elem = static_cast<elem_type*>(iElem);
    SG_ASSERT(this == elem->m_parent[id]);
    auto it = iterator(elem, id);
    --it;
    return it.get();
}
//=============================================================================
}

#endif

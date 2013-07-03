#ifndef Core_ComPtr_H
#define Core_ComPtr_H

#include "Platform.h"
#include "SmartPtr.h"

#if !SG_PLATFORM_IS_WIN
#error "forbidden include file on this platform"
#endif

namespace sg {
//=============================================================================
template <typename T>
class comptr
{
public:
    comptr() : m_p(nullptr) {}
    comptr(std::nullptr_t) : m_p(nullptr) {}
    comptr(T* iPtr) : m_p(iPtr) { IncRefIFP(m_p); }
    comptr(comptr const& iOther) : m_p(iOther.m_p) { IncRefIFP(m_p); }
    comptr(comptr&& iOther) : m_p(iOther.m_p) { iOther.m_p = nullptr; }
    ~comptr() { DecRefIFP(m_p); }

    comptr const& operator=(T* iPtr)
    {
        if(iPtr != m_p)
        {
            IncRefIFP(iPtr);
            T* old_p = m_p;
            m_p = iPtr;
            DecRefIFP(old_p);
        }
        return *this;
    }
    comptr const& operator=(comptr const& iOther)
    {
        // NB: le cas iOther = this est pris en compte par le cas
        // if(iPtr != m_p) dans operator=(T* iPtr).
        return (*this = iOther.m_p);
    }
    comptr const& operator=(comptr&& iOther)
    {
        if(iOther.m_p != m_p)
        {
            T* old_p = m_p;
            m_p = iOther.m_p;
            iOther.m_p = nullptr;
            DecRefIFP(old_p);
        }
        return *this;
    }

    T** GetPointerForInitialisation()
    {
        DecRefIFP(m_p);
        m_p = nullptr;
        return &m_p;
    }
    T* get() const { return m_p; }
    T* operator->() { return m_p; }
private:
    static void IncRefIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            iPtr->AddRef();
        }
    }
    static void DecRefIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            iPtr->Release();
        }
    }
private:
    T* m_p;
};
//=============================================================================
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(comptr, comptr)
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(comptr, refptr)
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(comptr, safeptr)
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(comptr, scopedptr)
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(refptr, comptr)
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(safeptr, comptr)
DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(scopedptr, comptr)

DEFINE_SMARTPTR_PTR_COMPARISON_OP(comptr)
//=============================================================================
}

#endif

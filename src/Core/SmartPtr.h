#ifndef Core_SmartPtr_H
#define Core_SmartPtr_H

#include "Assert.h"
#include "Config.h"
#include "Utils.h"
#include <atomic>
#include <functional>
#include <thread>

namespace sg {
//=============================================================================
class RefCountable
{
public:
    RefCountable()
        : m_refcount(0)
#if SG_ENABLE_ASSERT
        , m_threadId(std::this_thread::get_id())
#endif
    {}
    RefCountable(RefCountable const&) : RefCountable() {}
#if SG_ENABLE_ASSERT
    ~RefCountable();
#endif
    void IncRef() const
    {
        SG_ASSERT(HasThreadAccessRights());
        ++m_refcount;
    }
    bool DecRefReturnMustBeDeleted() const
    {
        SG_ASSERT(HasThreadAccessRights());
        SG_ASSERT(0 < m_refcount);
        return 0 == --m_refcount;
    }
    RefCountable const* GetAsRefCountable() const { return this; }

    RefCountable const& operator=(RefCountable const&)
    {
        // do not copy ref count.
        return *this;
    }
    SG_FORCE_INLINE void GiveOwnershipToThread(std::thread::id iThreadId)
    {
#if SG_ENABLE_ASSERT
        SG_ASSERT(HasThreadAccessRights());
        m_threadId = iThreadId;
#else
        SG_UNUSED(iThreadId);
#endif
    }
#if SG_ENABLE_ASSERT
    bool IsRefCounted_ForAssert() const { return 0 != m_refcount; }
    bool HasThreadAccessRights() const { return std::this_thread::get_id() == m_threadId; }
#endif
private:
    mutable size_t m_refcount;
#if SG_ENABLE_ASSERT
    std::thread::id m_threadId;
#endif
};
//=============================================================================
class VirtualRefCountable
{
public:
    virtual ~VirtualRefCountable() {}
    RefCountable const* GetAsRefCountable() const { return VirtualGetAsRefCountable(); }
protected:
    virtual RefCountable const* VirtualGetAsRefCountable() const = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define PARENT_REF_COUNTABLE(PARENT_TYPE) \
    public: \
        RefCountable const* GetAsRefCountable() const { return PARENT_TYPE::GetAsRefCountable(); } \
    private: \
        virtual RefCountable const* VirtualGetAsRefCountable() const { return GetAsRefCountable(); } \
    private:
//=============================================================================
class RefCountableOnce
{
public:
    void IncRef() const
    {
#if SG_ENABLE_ASSERT
        SG_ASSERT(HasThreadAccessRights());
        SG_ASSERT(0 == m_refcount);
        ++m_refcount;
#endif
    }
    bool DecRefReturnMustBeDeleted() const
    {
        SG_ASSERT(HasThreadAccessRights());
        SG_ASSERT(1 == m_refcount);
        return true;
    }
    RefCountableOnce const* GetAsRefCountable() const { return this; }

    SG_FORCE_INLINE void GiveOwnershipToThread(std::thread::id iThreadId)
    {
#if SG_ENABLE_ASSERT
        SG_ASSERT(HasThreadAccessRights());
        m_threadId = iThreadId;
#else
        SG_UNUSED(iThreadId);
#endif
    }
#if SG_ENABLE_ASSERT
    RefCountableOnce()
        : m_refcount(0)
#if SG_ENABLE_ASSERT
        , m_threadId(std::this_thread::get_id())
#endif
    {}
    RefCountableOnce(RefCountableOnce const&) : RefCountableOnce() {}
    RefCountableOnce const& operator=(RefCountableOnce const&)
    {
        // do not copy ref count.
        return *this;
    }
    bool IsRefCounted_ForAssert() const { return 0 != m_refcount; }
    bool HasThreadAccessRights() const { return std::this_thread::get_id() == m_threadId; }
private:
    mutable size_t m_refcount;
    std::thread::id m_threadId;
#endif
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SafeCountable
{
#if SG_ENABLE_ASSERT
public:
    SafeCountable() : m_safecount(0) {}
    SafeCountable(SafeCountable const&) : SafeCountable() {}
    ~SafeCountable();
    void IncSafeCount() const { ++m_safecount; }
    void DecSafeCount() const { SG_ASSERT(0 != m_safecount); --m_safecount; }
    SafeCountable const* GetAsSafeCountable() const { return this; }
    SafeCountable const& operator=(SafeCountable const&)
    {
        // do not copy safe count.
        return *this;
    }
private:
    mutable std::atomic<size_t> m_safecount;
#endif
};
//=============================================================================
class RefAndSafeCountable : public RefCountable
#if SG_ENABLE_ASSERT
                          , public SafeCountable
#endif
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class VirtualRefAndSafeCountable : public VirtualRefCountable
#if SG_ENABLE_ASSERT
                                 , public SafeCountable
#endif
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class RefAndSafeCountableOnce : public RefCountableOnce
#if SG_ENABLE_ASSERT
                              , public SafeCountable
#endif
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
#define PARENT_SAFE_COUNTABLE(PARENT_TYPE) \
    public: \
        SafeCountable const* GetAsSafeCountable() const { return PARENT_TYPE::GetAsSafeCountable(); } \
    private:
#else
#define PARENT_SAFE_COUNTABLE(PARENT_TYPE)
#endif
//=============================================================================
class RefAndSafeCountableWithVirtualDestructor : public RefAndSafeCountable
{
public:
    virtual ~RefAndSafeCountableWithVirtualDestructor() {}
};
//=============================================================================
template <typename T>
class refptr
{
public:
    refptr() : m_p(nullptr) {}
    refptr(std::nullptr_t) : m_p(nullptr) {}
    refptr(T* iPtr) : m_p(iPtr) { IncRefIFP(m_p); }
    refptr(refptr const& iOther) : m_p(iOther.m_p) { IncRefIFP(m_p); }
    refptr(refptr&& iOther) : m_p(iOther.m_p) { iOther.m_p = nullptr; }
    ~refptr() { DecRefIFP(m_p); }

    refptr const& operator=(T* iPtr)
    {
        // Note: le cas iPtr == m_p passe par le chemin "complexe" pour éviter
        // un test en plus dans le cas le plus commun.
        IncRefIFP(iPtr);
        T* old_p = m_p;
        m_p = iPtr;
        DecRefIFP(old_p);
        return *this;
    }
    refptr const& operator=(refptr const& iOther)
    {
        // NB: le cas iOther = this est bien pris en compte (vérifiez par vous-même).
        return (*this = iOther.m_p);
    }
    refptr const& operator=(refptr&& iOther)
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

    T* get() const { return m_p; }
    T* operator->() const { return m_p; }
    T& operator*() const { return *m_p; }
    friend void swap(refptr& a, refptr& b) { using std::swap; swap(a.m_p, b.m_p); }
private:
    static void IncRefIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            iPtr->GetAsRefCountable()->IncRef();
        }
    }
    static void DecRefIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            bool mustdelete = iPtr->GetAsRefCountable()->DecRefReturnMustBeDeleted();
            if(mustdelete)
            {
                delete iPtr;
            }
        }
    }
private:
    T* m_p;
};
//=============================================================================
template <typename T>
class refptrOrInt
{
    static_assert(std::alignment_of<T>::value % 2 == 0, "");
public:
    refptrOrInt() : m_p(nullptr) {}
    refptrOrInt(std::nullptr_t) : m_p(nullptr) {}
    refptrOrInt(T* iPtr) : m_p(iPtr) { IncRefIFP(m_p); }
    refptrOrInt(refptrOrInt const& iOther) : m_p(iOther.m_p) { if(iOther.IsPtr()) { IncRefIFP(m_p); } }
    refptrOrInt(refptrOrInt&& iOther) : m_p(iOther.m_p) { iOther.m_p = nullptr; }
    ~refptrOrInt() { if(IsPtr()) { DecRefIFP(m_p); } }

    bool IsPtr() const { return 0 == (ptrdiff_t(m_p) & 1); }
    bool IsInt() const { return 0 != (ptrdiff_t(m_p) & 1); }
    void SetInt(int i) { SG_ASSERT(std::abs(i) < (1 << 24) /* can be increased */); *this = nullptr; m_p = (T*) ptrdiff_t((i * 2) | 1); }
    int GetInt() { SG_ASSERT(IsInt()); return int(ptrdiff_t(m_p)) >> 1; }

    refptrOrInt const& operator=(T* iPtr)
    {
        // Note: le cas iPtr == m_p passe par le chemin "complexe" pour éviter
        // un test en plus dans le cas le plus commun.
        IncRefIFP(iPtr);
        bool const wasPtr = IsPtr();
        T* old_p = m_p;
        m_p = iPtr;
        if(wasPtr)
            DecRefIFP(old_p);
        return *this;
    }
    refptrOrInt const& operator=(refptrOrInt const& iOther)
    {
        if(iOther.m_p != m_p)
        {
            bool const wasPtr = IsPtr();
            T* old_p = m_p;
            m_p = iOther.m_p;
            if(wasPtr)
                DecRefIFP(old_p);
            if(IsPtr())
                IncRefIFP(m_p);
        }
        return *this;
    }
    refptrOrInt const& operator=(refptrOrInt&& iOther)
    {
        if(iOther.m_p != m_p)
        {
            bool const wasPtr = IsPtr();
            T* old_p = m_p;
            m_p = iOther.m_p;
            iOther.m_p = nullptr;
            if(wasPtr)
                DecRefIFP(old_p);
        }
        return *this;
    }

    T* get() const { SG_ASSERT(IsPtr()); return m_p; }
    T* operator->() const { SG_ASSERT(IsPtr()); return m_p; }
    T& operator*() const { SG_ASSERT(IsPtr()); return *m_p; }
    friend void swap(refptrOrInt& a, refptrOrInt& b) { using std::swap; swap(a.m_p, b.m_p); }
private:
    static void IncRefIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            iPtr->GetAsRefCountable()->IncRef();
        }
    }
    static void DecRefIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            bool mustdelete = iPtr->GetAsRefCountable()->DecRefReturnMustBeDeleted();
            if(mustdelete)
            {
                delete iPtr;
            }
        }
    }
private:
    T* m_p;
};
//=============================================================================
class WeakCount : public RefCountable
{
public:
    WeakCount(void const* iPointed) : m_pointed(iPointed) { SG_ASSERT(nullptr != m_pointed); }
    WeakCount() { SG_ASSERT(nullptr == m_pointed); }
    void Unbind(void const* iPointed) { SG_ASSERT_AND_UNUSED(iPointed == m_pointed); m_pointed = nullptr; }
    bool IsValid() { return nullptr != m_pointed; }
#if SG_ENABLE_ASSERT
    void Check(void const* iPointed) { SG_ASSERT(iPointed == m_pointed); }
#endif
private:
    void const* m_pointed;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class RefAndWeakCountable : public RefCountable
{
public:
    RefAndWeakCountable() : m_weakCount() {}
    RefAndWeakCountable(RefAndWeakCountable const&) : RefAndWeakCountable() {}
    ~RefAndWeakCountable()
    {
        SG_ASSERT(HasThreadAccessRights());
        if(nullptr != m_weakCount.get())
            m_weakCount->Unbind(this);
    }

    RefAndWeakCountable const* GetAsWeakCountable() const { return this; }
    WeakCount* GetWeakCount() const
    {
        SG_ASSERT(HasThreadAccessRights());
        if(nullptr == m_weakCount.get())
            m_weakCount = new WeakCount(this);
        return m_weakCount.get();
    }
    RefAndWeakCountable const& operator=(RefAndWeakCountable const&)
    {
        // do not copy ref count nor weak count.
        return *this;
    }
private:
    mutable refptr<WeakCount> m_weakCount;
};
//=============================================================================
class VirtualWeakCountable
{
public:
    virtual ~VirtualWeakCountable() {}
    SG_FORCE_INLINE RefAndWeakCountable const* GetAsWeakCountable() const { return VirtualGetAsWeakCountable(); }
protected:
    virtual RefAndWeakCountable const* VirtualGetAsWeakCountable() const = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define PARENT_WEAK_COUNTABLE(PARENT_TYPE) \
    public: \
        SG_FORCE_INLINE RefAndWeakCountable const* GetAsWeakCountable() const \
        { return PARENT_TYPE::GetAsWeakCountable(); } \
    private: \
        virtual RefAndWeakCountable const* VirtualGetAsWeakCountable() const { return PARENT_TYPE::GetAsWeakCountable(); } \
    private:
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class VirtualRefAndWeakCountable
{
public:
    virtual ~VirtualRefAndWeakCountable() {}
    SG_FORCE_INLINE RefCountable const* GetAsRefCountable() const { return VirtualGetAsRefCountable(); }
    SG_FORCE_INLINE RefAndWeakCountable const* GetAsWeakCountable() const { return VirtualGetAsWeakCountable(); }
protected:
    virtual RefCountable const* VirtualGetAsRefCountable() const = 0;
    virtual RefAndWeakCountable const* VirtualGetAsWeakCountable() const = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class RefWeakAndSafeCountable : public RefAndWeakCountable
#if SG_ENABLE_ASSERT
                              , public SafeCountable
#endif
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class VirtualRefWeakAndSafeCountable : public VirtualRefAndWeakCountable
#if SG_ENABLE_ASSERT
                              , public SafeCountable
#endif
{
};
//=============================================================================
template <typename T>
class weakptr
{
public:
    weakptr(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            RefAndWeakCountable const* weakCountable = iPtr->GetAsWeakCountable();
            m_weakCount = weakCountable->GetWeakCount();
        }
        else
            m_weakCount = nullptr;
        m_p = iPtr;
    }
    weakptr(refptr<T> const& iPtr)
        : weakptr(iPtr.get())
    {
    }
    weakptr& operator = (T* iPtr)
    {
        if(nullptr != iPtr)
        {
            RefAndWeakCountable const* weakCountable = iPtr->GetAsWeakCountable();
            m_weakCount = weakCountable->GetWeakCount();
        }
        else
            m_weakCount = nullptr;
        m_p = iPtr;
        return *this;
    }
    weakptr& operator = (refptr<T> const& iPtr)
    {
        return operator=(iPtr.get());
    }
    template <typename U>
    void Lock(refptr<U>& oPtr)
    {
        // Note that this method must be called in the owner thread of m_p.
        // It is asserted by the ref count update, though.
        oPtr = nullptr;
        if(nullptr != m_weakCount)
        {
            SG_ASSERT(nullptr != m_p);
            if(m_weakCount->IsValid())
            {
#if SG_ENABLE_ASSERT
                RefAndWeakCountable const* weakCountable = m_p->GetAsWeakCountable();
                m_weakCount->Check(weakCountable);
                SG_ASSERT(weakCountable->IsRefCounted_ForAssert());
#endif
                oPtr = m_p;
            }
            else
                Clear();
        }
    }
    refptr<T> Lock()
    {
        refptr<T> p;
        Lock(p);
        return p;
    }
private:
    void Clear()
    {
        m_weakCount = nullptr;
        m_p = nullptr;
    }
private:
    refptr<WeakCount> m_weakCount;
    T* m_p;
};
//=============================================================================
template <typename T>
class safeptr
{
#if SG_ENABLE_ASSERT
public:
    safeptr() : m_p(nullptr) {}
    safeptr(std::nullptr_t) : m_p(nullptr) {}
    safeptr(T* iPtr) : m_p(iPtr) { IncSafeCountIFP(m_p); }
    safeptr(safeptr const& iOther) : m_p(iOther.m_p) { IncSafeCountIFP(m_p); }
    safeptr(safeptr&& iOther) : m_p(iOther.m_p) { iOther.m_p = nullptr; }
    ~safeptr() { DecSafeCountIFP(m_p); }

    safeptr const& operator=(T* iPtr)
    {
        if(iPtr != m_p)
        {
            IncSafeCountIFP(iPtr);
            DecSafeCountIFP(m_p);
            m_p = iPtr;
        }
        return *this;
    }
    safeptr const& operator=(safeptr const& iOther)
    {
        // NB: le cas iOther = this est pris en compte par le cas
        // if(iPtr != m_p) dans operator=(T* iPtr).
        return (*this = iOther.m_p);
    }
    safeptr const& operator=(safeptr&& iOther)
    {
        if(iOther.m_p != m_p)
        {
            DecSafeCountIFP(m_p);
            m_p = iOther.m_p;
            iOther.m_p = nullptr;
        }
        return *this;
    }
private:
    static void IncSafeCountIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            iPtr->GetAsSafeCountable()->IncSafeCount();
        }
    }
    static void DecSafeCountIFP(T* iPtr)
    {
        if(nullptr != iPtr)
        {
            iPtr->GetAsSafeCountable()->DecSafeCount();
        }
    }
#else
public:
    safeptr() : m_p(nullptr) {}
    safeptr(std::nullptr_t) : m_p(nullptr) {}
    safeptr(T* iPtr) : m_p(iPtr) {}
    safeptr(safeptr const& iOther) : m_p(iOther.m_p) {}
    safeptr(safeptr&& iOther) : m_p(iOther.m_p) {}
    ~safeptr() {}

    safeptr const& operator=(T* iPtr)
    {
        m_p = iPtr;
        return *this;
    }
    safeptr const& operator=(safeptr const& iOther)
    {
        return (*this = iOther.m_p);
    }
    safeptr const& operator=(safeptr&& iOther)
    {
        m_p = iOther.m_p;
        return *this;
    }
#endif
public:
    T* get() const { return m_p; }
    T* operator->() const { return m_p; }
    T& operator*() const { return *m_p; }
    friend void swap(safeptr& a, safeptr& b) { using std::swap; swap(a.m_p, b.m_p); }
private:
    T* m_p;
};
//=============================================================================
template <typename T, bool keepRef>
class reforsafeptr : public std::conditional<keepRef, refptr<T>, safeptr<T> >::type
{
public:
    typedef typename std::conditional<keepRef, refptr<T>, safeptr<T> >::type parent_type;
    reforsafeptr() : parent_type() {}
    reforsafeptr(std::nullptr_t) : parent_type(nullptr) {}
    reforsafeptr(T* iPtr) : parent_type(iPtr) {}
    reforsafeptr(reforsafeptr const& iOther) : parent_type(static_cast<parent_type const&>(iOther)) {}
    reforsafeptr(reforsafeptr&& iOther) : parent_type(static_cast<parent_type&&>(iOther)) {}
    reforsafeptr(parent_type const& iOther) : parent_type(iOther) {}
    reforsafeptr(parent_type&& iOther) : parent_type(iOther) {}
};
//=============================================================================
template <typename T>
class scopedptr
{
    SG_NON_COPYABLE(scopedptr)
public:
    scopedptr() : m_p(nullptr) {}
    scopedptr(std::nullptr_t) : m_p(nullptr) {}
    scopedptr(T* iPtr) : m_p(iPtr) {}
    scopedptr(scopedptr&& iPtr) : m_p(iPtr.m_p) { iPtr.m_p = nullptr; }
    ~scopedptr() { if(nullptr != m_p) delete m_p; }

    void reset(T* iPtr)
    {
        SG_ASSERT(m_p != iPtr || nullptr == iPtr);
        if(nullptr != m_p)
            delete m_p;
        m_p = iPtr;
    }
    void reset()
    {
        reset(nullptr);
    }

    T* get() const { return m_p; }
    T* operator->() const { return m_p; }
    T& operator*() const { return *m_p; }
    friend void swap(scopedptr& a, scopedptr& b) { using std::swap; swap(a.m_p, b.m_p); }
private:
    T* m_p;
};
//=============================================================================
#define SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(SP1, SP2) \
    template <typename T1, typename T2> inline bool operator==(SP1<T1> const& a, SP2<T2> const& b) { return a.get() == b.get(); } \
    template <typename T1, typename T2> inline bool operator!=(SP1<T1> const& a, SP2<T2> const& b) { return a.get() != b.get(); }

#define SG_DEFINE_SMARTPTR_PTR_COMPARISON_OP(SP) \
    template <typename T1, typename T2> inline bool operator==(SP<T1> const& a, T2 const& b) { return a.get() == b; } \
    template <typename T1, typename T2> inline bool operator==(T1 const& a, SP<T2> const& b) { return a == b.get(); } \
    template <typename T> inline bool operator==(SP<T> const& a, std::nullptr_t) { return nullptr == a.get(); } \
    template <typename T> inline bool operator==(std::nullptr_t, SP<T> const& a) { return nullptr == a.get(); } \
    template <typename T1, typename T2> inline bool operator!=(SP<T1> const& a, T2 const& b) { return a.get() != b; } \
    template <typename T1, typename T2> inline bool operator!=(T1 const& a, SP<T2> const& b) { return a != b.get(); } \
    template <typename T> inline bool operator!=(SP<T> const& a, std::nullptr_t) { return nullptr != a.get(); } \
    template <typename T> inline bool operator!=(std::nullptr_t, SP<T> const& a) { return nullptr != a.get(); }

SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(refptr, refptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(refptr, safeptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(refptr, scopedptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(safeptr, refptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(safeptr, safeptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(safeptr, scopedptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(scopedptr, refptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(scopedptr, safeptr)
SG_DEFINE_SMARTPTR_SMARTPTR_COMPARISON_OP(scopedptr, scopedptr)
SG_DEFINE_SMARTPTR_PTR_COMPARISON_OP(refptr)
SG_DEFINE_SMARTPTR_PTR_COMPARISON_OP(safeptr)
SG_DEFINE_SMARTPTR_PTR_COMPARISON_OP(scopedptr)
//=============================================================================
}



namespace std {
//=============================================================================
template <typename T> struct hash<sg::refptr<T> >
{
    size_t operator()(sg::refptr<T> const& x) const
    {
        return (size_t)x.get();
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T> struct hash<sg::safeptr<T> >
{
    size_t operator()(sg::safeptr<T> const& x) const
    {
        return (size_t)x.get();
    }
};
//=============================================================================
}

#endif

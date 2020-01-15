#ifndef Core_Observer_H
#define Core_Observer_H

#include "SmartPtr.h"
#include <unordered_set>

namespace sg {
//=============================================================================
template<typename T> class Observable;
template<typename T> class UnsharableObservable;
//=============================================================================
SG_DEFINE_TYPED_TAG(allow_destruction_of_observable)
//=============================================================================
template<typename T>
class Observer : public SafeCountable
{
    friend class Observable<T>;
    friend class UnsharableObservable<T>;
protected:
    virtual ~Observer() {}
    virtual void VirtualOnNotified(T const* iObservable) = 0;
#if SG_ENABLE_ASSERT
public:
    Observer() : m_allowDestructionOfObservable(false) {}
    Observer(allow_destruction_of_observable_t) : m_allowDestructionOfObservable(true) {}
    bool AllowDestructionOfObservable() const { return m_allowDestructionOfObservable; }
private:
    bool m_allowDestructionOfObservable;
#else
    Observer() {}
    Observer(allow_destruction_of_observable_t) {}
#endif
};
//=============================================================================
template<typename T>
class Observable : public SafeCountable
{
protected:
    Observable() {}
public:
    ~Observable()
    {
#if SG_ENABLE_ASSERT
        for(auto const& it : m_observers)
        {
            SG_ASSERT(it->AllowDestructionOfObservable());
        }
#endif
    }
    void NotifyObservers() const
    {
        for(auto& it : m_observers)
        {
            it->VirtualOnNotified(static_cast<T const*>(this));
        }
    }
    void RegisterObserver(Observer<T>* iListener) const
    {
        SG_ASSERT(nullptr != iListener);
        auto r = m_observers.insert(iListener);
        SG_ASSERT_MSG(r.second, "listener already in container!");
        // Note that we do not call iListener->VirtualOnNotified() here, so
        // as to remove risks of having a virtual iListener not completely
        // constructed, as registration is often done in constructors.
    }
    void UnregisterObserver(Observer<T>* iListener) const
    {
        SG_ASSERT(nullptr != iListener);
        size_t const r = m_observers.erase(iListener);
        SG_ASSERT_MSG_AND_UNUSED(r, "listener was not in container!");
    }
#if SG_ENABLE_ASSERT
    bool HasObserverForDebug() const { return m_observers.size() != 0; }
#endif
private:
    mutable std::unordered_set<safeptr<Observer<T> > > m_observers;
};
//=============================================================================
template<typename T>
class UnsharableObservable : public SafeCountable
{
protected:
    UnsharableObservable() {}
public:
    ~UnsharableObservable()
    {
        SG_ASSERT_MSG(nullptr == m_observer || m_observer->AllowDestructionOfObservable(), "The observer must be unregistered!");
    }
    void NotifyObservers() const
    {
        if(nullptr != m_observer)
            m_observer->VirtualOnNotified(static_cast<T const*>(this));
    }
    void RegisterObserver(Observer<T>* iListener) const
    {
        SG_ASSERT(nullptr != iListener);
        SG_ASSERT_MSG(nullptr == m_observer, "There must be one observer at most!");
        m_observer = iListener;
        // Note that we do not call iListener->VirtualOnNotified() here, so
        // as to remove risks of having a virtual iListener not completely
        // constructed, as registration is often done in constructors.
    }
    void UnregisterObserver(Observer<T>* iListener) const
    {
        SG_ASSERT_AND_UNUSED(nullptr != iListener);
        SG_ASSERT_MSG(m_observer == iListener, "listener was not registered!");
        m_observer = nullptr;
    }
private:
    mutable safeptr<Observer<T> > m_observer;
};
//=============================================================================
// Note: You can derivate this class as does ObservableValue to create your own
// strongly typed observable value.
template<template <typename> class O, typename T, typename V>
class ObservableValueHelperImpl : public Observable<T>
{
protected:
    ObservableValueHelperImpl() {}
public:
    ObservableValueHelperImpl(V const& iValue)
    {
        m_value = iValue;
    }
    ~ObservableValueHelperImpl() {}
    void Set(V const& iValue)
    {
        m_value = iValue;
        NotifyObservers();
    }
    V const& Get() const
    {
        return m_value;
    }
private:
    V m_value;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename V>
class ObservableValueHelper : public ObservableValueHelperImpl<Observable, T, V>
{
protected:
    ObservableValueHelper() {}
public:
    ObservableValueHelper(V const& iValue) : ObservableValueHelperImpl(iValue) {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename V>
class UnsharedObservableValueHelper : public ObservableValueHelperImpl<Observable, T, V>
{
protected:
    UnsharedObservableValueHelper() {}
public:
    UnsharedObservableValueHelper(V const& iValue) : ObservableValueHelperImpl(iValue) {}
};
//=============================================================================
template<typename V>
class ObservableValue : public ObservableValueHelper<ObservableValue<V>, V>
{
public:
    ObservableValue() {}
    ObservableValue(V const& iValue) : ObservableValueHelper(iValue) {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename V>
class UnsharedObservableValue : public UnsharedObservableValueHelper<ObservableValue<V>, V>
{
public:
    UnsharedObservableValue() {}
    UnsharedObservableValue(V const& iValue) : UnsharedObservableValueHelper(iValue) {}
};
//=============================================================================
}

#endif

#ifndef Core_Singleton_H
#define Core_Singleton_H

#include "Assert.h"
#include "Config.h"
#include "SmartPtr.h"

namespace sg {
//=============================================================================
template <class Derived>
class Singleton : public SafeCountable
{
public:
    Singleton() { SG_ASSERT(nullptr == s_instance); s_instance = static_cast<Derived*>(this); }
    ~Singleton() { SG_ASSERT(this == s_instance); s_instance = nullptr; }
    static Derived* GetIFP() { return s_instance.get(); }
    static Derived& Get() { SG_ASSERT(nullptr != s_instance); return *s_instance; }
private:
    static safeptr<Derived> s_instance;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <class Derived>
safeptr<Derived> Singleton<Derived>::s_instance = nullptr;
//=============================================================================
}

#endif

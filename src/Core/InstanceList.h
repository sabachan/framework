#ifndef Core_InstanceList_H
#define Core_InstanceList_H

#include <Core/ArrayList.h>
#include <Core/SmartPtr.h>
#include <unordered_map>

// An InstanceList stores a list of Instances associated with their respective
// Descriptor. It tracks the order of creation in order to be able to destroy
// the instances in reverse order for not breaking the dependencies.
// The Descriptor class must define instance_type as type or a type alias.
// The Instance class must derive from RefAndSafeCountableWithVirtualDestructor

namespace sg {
//=============================================================================
// TODO: How to generate a friendly error when descriptor has no instance_type
// defined?
class InstanceList : public SafeCountable
{
public:
    typedef RefAndSafeCountableWithVirtualDestructor abstract_instance_type;
public:
    ~InstanceList()
    {
        Clear();
    }
    void Reserve(size_t iCapacity)
    {
        m_map.reserve(iCapacity);
        m_instances.Reserve(iCapacity);
    }
    template <typename D>
    void SetInstance(D const* iDescriptor, typename D::instance_type* iInstance)
    {
        static_assert(std::is_base_of<abstract_instance_type, typename D::instance_type>::value, "instance_type must derive from RefAndSafeCountableWithVirtualDestructor");
        SG_ASSERT(nullptr == GetInstanceIFP(iDescriptor));
        m_instances.EmplaceBack(iInstance);
        m_map.emplace(iDescriptor, iInstance);
    }
    template <typename D, typename... Ts>
    typename D::instance_type* GetOrCreateInstance(D const* iDescriptor, Ts&&... iConstructionArgs)
    {
        static_assert(std::is_base_of<abstract_instance_type, typename D::instance_type>::value, "instance_type must derive from RefAndSafeCountableWithVirtualDestructor");
        static_assert(std::is_same<typename D::instance_type*, decltype(std::declval<D const*>()->CreateInstance(std::declval<Ts>()...))>::value, "instance_type is wrongly defined");
        auto const f = m_map.find(iDescriptor);
        abstract_instance_type* instance = nullptr;
        if(f == m_map.end())
        {
            typename D::instance_type* typedInstance = iDescriptor->CreateInstance(iConstructionArgs...);
            instance = typedInstance;
            m_instances.EmplaceBack(instance);
            m_map.emplace(iDescriptor, instance);
        }
        else
            instance = f->second.get();
        return checked_cast<typename D::instance_type*>(instance);
    }
    template <typename D>
    typename D::instance_type* GetInstanceIFP(D const* iDescriptor)
    {
        static_assert(std::is_base_of<abstract_instance_type, typename D::instance_type>::value, "instance_type must derive from RefAndSafeCountableWithVirtualDestructor");
        auto const f = m_map.find(iDescriptor);
        if(f == m_map.end())
            return nullptr;
        abstract_instance_type* instance = f->second.get();
        return checked_cast<typename D::instance_type*>(instance);
    }
    void Clear()
    {
        m_map.clear();
        reverse_for_range(size_t, i, 0, m_instances.size())
        {
            m_instances[i] = nullptr;
        }
        m_instances.Clear();
    }
private:
#if SG_ENABLE_ASSERT
    typedef safeptr<SafeCountable const> desc_id_ptr;
#else
    typedef void const* desc_id_ptr;
#endif
    std::unordered_map<desc_id_ptr, safeptr<abstract_instance_type> > m_map;
    ArrayList<refptr<abstract_instance_type>> m_instances;
};
//=============================================================================
}

#endif

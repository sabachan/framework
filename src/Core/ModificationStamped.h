#ifndef Core_ModificationStamp_H
#define Core_ModificationStamp_H

#include "Config.h"
#include "SmartPtr.h"

namespace sg {
//=============================================================================
class ModificationStamped
{
public:
    SG_FORCE_INLINE ModificationStamped()
        : m_modificationStamp(all_ones)
        SG_CODE_FOR_ASSERT( SG_COMMA m_modificationAllowed(true) )
        SG_CODE_FOR_ASSERT( SG_COMMA m_initialized(false) )
    {
    }
    SG_FORCE_INLINE ModificationStamped(ModificationStamped const& o)
        : m_modificationStamp(o.m_modificationStamp)
        SG_CODE_FOR_ASSERT( SG_COMMA m_modificationAllowed(o.m_modificationAllowed) )
        SG_CODE_FOR_ASSERT( SG_COMMA m_initialized(o.m_initialized) )
    {
    }
    SG_FORCE_INLINE ModificationStamped& operator=(ModificationStamped const& o)
    {
        m_modificationStamp = o.m_modificationStamp;
        SG_CODE_FOR_ASSERT(m_modificationAllowed = o.m_modificationAllowed);
    }
    SG_FORCE_INLINE void BeginModification()
    {
        SG_ASSERT(!m_modificationAllowed || !m_initialized);
        SG_CODE_FOR_ASSERT(m_modificationAllowed = true);
    }
    SG_FORCE_INLINE void EndModification()
    {
        SG_ASSERT(m_modificationAllowed);
        m_modificationStamp = GetNextModificationStamp();
        SG_CODE_FOR_ASSERT(m_modificationAllowed = false);
        SG_CODE_FOR_ASSERT(m_initialized = true);
    }
    size_t ModificationStamp()
    {
        SG_ASSERT(!m_modificationAllowed && m_initialized);
        return m_modificationStamp;
    }
#if SG_ENABLE_ASSERT
    bool ModificationAllowed_ForAssert() const
    {
        return m_modificationAllowed;
    }
#endif
private:
    SG_FORCE_INLINE static size_t GetNextModificationStamp()
    {
        // NB: stamp value provider is shared with all stamped classes.
        // It is OK, as long as probability of having 2 times the same
        // stamp is negligible. This is assumed to be already the case in 32 bits
        // as it loops in 4 billions updates.
        static size_t nextStamp = 0;
        return nextStamp++;
    }
private:
    size_t m_modificationStamp;
    SG_CODE_FOR_ASSERT(bool m_modificationAllowed;)
    SG_CODE_FOR_ASSERT(bool m_initialized;)
};
//=============================================================================
}

#endif

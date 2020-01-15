#ifndef Core_Tribool_H
#define Core_Tribool_H

#include "Assert.h"
#include "Utils.h"

namespace sg {
//=============================================================================
SG_DEFINE_TYPED_TAG(indeterminate)
//=============================================================================
class tribool
{
public:
    tribool() : m_value(0x0) {}
    tribool(indeterminate_t) : m_value(0x1) {}
    tribool(bool b) : m_value(b ? 0x3 : 0x0) {}
    bool Is(tribool v) { return m_value == v.m_value; }
    bool IsTrue() const { return m_value == tribool(true).m_value; }
    bool IsFalse() const { return m_value == tribool(false).m_value; }
    bool IsIndeterminate() const { return m_value == tribool(indeterminate).m_value; }
    friend tribool operator&& (tribool a, tribool b) { tribool r; r.m_value = a.m_value & b.m_value; return r; }
    friend tribool operator|| (tribool a, tribool b) { tribool r; r.m_value = a.m_value | b.m_value; return r; }
    friend tribool operator! (tribool a) { tribool r; r.m_value = 0x3 >> a.m_value; return r; }
private:
    u8 m_value;
};
//=============================================================================
}

#endif

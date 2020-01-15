#include "stdafx.h"

#include "Tribool.h"

#include "Assert.h"
#include "TestFramework.h"

#if SG_ENABLE_UNIT_TESTS

namespace sg {
//=============================================================================
SG_TEST((sg), Tribool, (Core, quick))
{
    tribool const T = true;
    tribool const F = false;
    tribool const I = indeterminate;
    SG_ASSERT(T.IsTrue());
    SG_ASSERT(F.IsFalse());
    SG_ASSERT(I.IsIndeterminate());
    tribool a;
    SG_ASSERT(a.IsFalse());
    a = T && T;
    SG_ASSERT(a.IsTrue());
    a = T && F;
    SG_ASSERT(a.IsFalse());
    a = T && I;
    SG_ASSERT(a.IsIndeterminate());
    a = F && F;
    SG_ASSERT(a.IsFalse());
    a = F && I;
    SG_ASSERT(a.IsFalse());
    a = I && I;
    SG_ASSERT(a.IsIndeterminate());
    a = T || T;
    SG_ASSERT(a.IsTrue());
    a = T || F;
    SG_ASSERT(a.IsTrue());
    a = T || I;
    SG_ASSERT(a.IsTrue());
    a = F || F;
    SG_ASSERT(a.IsFalse());
    a = F || I;
    SG_ASSERT(a.IsIndeterminate());
    a = I || I;
    SG_ASSERT(a.IsIndeterminate());
    a = !T;
    SG_ASSERT(a.IsFalse());
    a = !F;
    SG_ASSERT(a.IsTrue());
    a = !I;
    SG_ASSERT(a.IsIndeterminate());
}
//=============================================================================
}

#endif

#ifndef AppUtils_UnitTestsUtils_H
#define AppUtils_UnitTestsUtils_H

#include <Core/TestFramework.h>

#if SG_ENABLE_UNIT_TESTS
namespace sg {
//=============================================================================
void RunStartUnitTests();
void RunFastUnitTests();
void RunSlowUnitTests();
void RunSuperSlowUnitTests();
void RunPerfUnitTests();
void RunAllUnitTests();
void RunUnitTestsAtStartup();
//=============================================================================
}
#endif

#endif

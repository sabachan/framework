#ifndef Include_DeclareTests
#error "this file should be included only by one .cpp"
#endif
#if !defined(SG_ENABLE_UNIT_TESTS) || !SG_ENABLE_UNIT_TESTS
#error "this file should be included only when SG_ENABLE_UNIT_TESTS is 1"
#endif

// This file is used to force initialization of static objects used to
// register tests. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include <Core/TestFramework.h>

namespace sg {
namespace renderengine {
SG_DECLARE_TEST(RenderEngine)
}
}

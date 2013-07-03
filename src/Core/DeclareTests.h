#ifndef Include_DeclareTests
#error "this file should be included only by one .cpp"
#endif
#if !defined(SG_ENABLE_UNIT_TESTS) || !SG_ENABLE_UNIT_TESTS
#error "this file should be included only when SG_ENABLE_UNIT_TESTS is 1"
#endif

// This file is used to force initialization of static objects used to
// register tests. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include "TestFramework.h"

namespace sg {
SG_DECLARE_TEST(Allocators)
SG_DECLARE_TEST(ArrayList)
SG_DECLARE_TEST(BitField)
SG_DECLARE_TEST(BitFieldPerf)
SG_DECLARE_TEST(BitSet)
SG_DECLARE_TEST(BitSetPerf)
SG_DECLARE_TEST(FastSymbol)
SG_DECLARE_TEST(FixedPoint)
SG_DECLARE_TEST(HashMap)
SG_DECLARE_TEST(HashMapPerf)
SG_DECLARE_TEST(HashMapPool)
SG_DECLARE_TEST(Intrinsics)
SG_DECLARE_TEST(IntrusiveList)
SG_DECLARE_TEST(MaxSizeVector)
SG_DECLARE_TEST(PerfLog)
SG_DECLARE_TEST(Preprocessor)
SG_DECLARE_TEST(Relocate)
SG_DECLARE_TEST(SmartPtr)
SG_DECLARE_TEST(StringFormat)
SG_DECLARE_TEST(VectorOfScopedPtr)
namespace ini {
SG_DECLARE_TEST(Reader)
}
namespace filesystem {
SG_DECLARE_TEST(FileSystem)
}
namespace tools {
SG_DECLARE_TEST(Tools)
}
}

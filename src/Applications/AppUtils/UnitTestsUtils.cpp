#include "stdafx.h"

#include "UnitTestsUtils.h"

#if SG_ENABLE_UNIT_TESTS
#include <random>

namespace sg {
//=============================================================================
void RunStartUnitTests()
{
    char const* const tagIntersection[] = { "start", };
    sg::testframework::RunTests(nullptr, sg::AsArrayView(tagIntersection));
}
void RunFastUnitTests()
{
    char const* const tagIntersection[] = { "quick", };
    sg::testframework::RunTests(nullptr, sg::AsArrayView(tagIntersection));
}
void RunSlowUnitTests()
{
    char const* const tagIntersection[] = { "slow", };
    sg::testframework::RunTests(nullptr, sg::AsArrayView(tagIntersection));
}
void RunSuperSlowUnitTests()
{
    char const* const tagIntersection[] = { "super slow", };
    sg::testframework::RunTests(nullptr, sg::AsArrayView(tagIntersection));
}
void RunPerfUnitTests()
{
    char const* const tagIntersection[] = { "perf", };
    sg::testframework::RunTests(nullptr, sg::AsArrayView(tagIntersection));
}
void RunAllUnitTests()
{
    sg::testframework::RunTests(nullptr, nullptr);
}
void RunUnitTestsAtStartup()
{
    RunStartUnitTests();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0,1);
    float const t = dis(gen);

#if SG_CODE_IS_OPTIMIZED
    // Run fast tests always when optimized
    RunFastUnitTests();
    // Run slow unit tests only some times
    if(t < 1.f / 50.f)
        RunSuperSlowUnitTests();
    else if(t < 1.f / 10.f)
        RunSlowUnitTests();
#else
    // Run fast tests only some times in debug
    if(t < 1.f / 5.f)
        RunFastUnitTests();
#endif
}
//=============================================================================
}
#endif

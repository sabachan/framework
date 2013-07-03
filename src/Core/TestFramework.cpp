#include "stdafx.h"

#include "TestFramework.h"

#if SG_ENABLE_UNIT_TESTS

#include "Log.h"
#include "StringFormat.h"
#include "WinUtils.h"
#include <algorithm>
#include <sstream>
#include <vector>

namespace sg {
namespace testframework {
//=============================================================================
TestRegistrator* TestRegistrator::firstRegistrator = nullptr;
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CompareTests(TestRegistrator const* a, TestRegistrator const* b)
{
    size_t const na = a->nsSize;
    size_t const nb = b->nsSize;
    size_t const n = std::min(na, nb);
    for_range(size_t,i,0,n)
    {
        int const c = strcmp(a->ns[i], b->ns[i]);
        if(0 != c)
            return c < 0;
    }
    if(na != nb)
        return na < nb;
    int const c = strcmp(a->name, b->name);
    return c < 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CompareTestToNamespace(TestRegistrator const* a, ArrayView<char const* const> b)
{
    size_t const na = a->nsSize;
    size_t const nb = b.size();
    size_t const n = std::min(na, nb);
    for_range(size_t,i,0,n)
    {
        int c = strcmp(a->ns[i], b[i]);
        if(0 != c)
            return c < 0;
    }
    return na < nb;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IsTestInNamespace(TestRegistrator const* a, ArrayView<char const* const> b)
{
    size_t const na = a->nsSize;
    size_t const n = b.size();
    if(na < n)
        return false;
    for_range(size_t,i,0,n)
    {
        int c = strcmp(a->ns[i], b[i]);
        if(0 != c)
            return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetSortedTests(std::vector<TestRegistrator const*>& oUnitTests)
{
    SG_ASSERT(oUnitTests.empty());
    TestRegistrator const* r = TestRegistrator::firstRegistrator;
    while(nullptr != r)
    {
        oUnitTests.push_back(r);
        r = r->nextRegistrator;
    }
    std::sort(oUnitTests.begin(), oUnitTests.end(), CompareTests);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Matches(TestRegistrator const* a, ArrayView<char const* const> tagIntersection)
{
    for(char const* t : tagIntersection)
    {
        bool const isNegativeTag = '!' == t[0];
        char const* tag = isNegativeTag ? t + 1 : t;
        bool found = false;
        for_range(size_t, i, 0,  a->tagCount)
        {
            if(0 == strcmp(a->tags[i], tag))
            {
                found = true;
                break;
            }
        }
        if(found == isNegativeTag)
            return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Run(TestRegistrator const* test)
{
    std::ostringstream oss;
    for_range(size_t, i, 0, test->nsSize)
        oss << test->ns[i] << "/";
    oss << test->name;

    SG_LOG_INFO("=============================================================================");
    SG_LOG_INFO(Format("Test %0 begin", oss.str()));

    test->function();

    SG_LOG_INFO(Format("Test %0 end", oss.str()));
    SG_LOG_INFO("=============================================================================");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
void RunTests(ArrayView<char const* const> ns, ArrayView<char const* const> tagIntersection)
{
    std::vector<TestRegistrator const*> tests;
    GetSortedTests(tests);
    static bool undeclaredAlreadyChecked = false;
    if(!undeclaredAlreadyChecked)
    {
        std::vector<TestRegistrator const*> undeclared;
        for(TestRegistrator const* test : tests)
        {
            if(!*test->declared)
                undeclared.push_back(test);
        }
        if(!undeclared.empty())
        {
            std::ostringstream oss;
            oss << "The following tests have not been declared:" << std::endl;;
            for(TestRegistrator const* test : undeclared)
            {
                oss << "    ";
                for_range(size_t, i, 0, test->nsSize)
                    oss << test->ns[i] << "/";
                oss << test->name << std::endl;
            }
            oss << std::endl;
            oss << "They should be declared in the DeclareTests.h of their respective projects. ";
            oss << "While you're at it, it is recommended to check that all other new tests are correctly declared." << std::endl;
            winutils::ShowModalMessageOK("Undeclared tests!", oss.str().c_str());
        }
        undeclaredAlreadyChecked = true;
    }
    auto const b = tests.begin();
    auto const e = tests.end();
    auto f = std::lower_bound(b, e, ns, CompareTestToNamespace);
    while(f != e)
    {
        if(!IsTestInNamespace(*f, ns))
            break;
        if(Matches(*f, tagIntersection))
            Run(*f);
        ++f;
    }
}
//=============================================================================
}
}

#endif

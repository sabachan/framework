#ifndef Core_TestFramework_H
#define Core_TestFramework_H

#include "Config.h"

#if SG_ENABLE_UNIT_TESTS

#include "ArrayView.h"
#include "Assert.h"
#include "For.h"
#include "Preprocessor.h"


// SG_UNIT_TEST((domain, subdomain), name, (quick, domain, perf))
// {
//     ASSERT(works);
// }
//

namespace sg {
namespace testframework {
//=============================================================================
void RunTests(ArrayView<char const* const> ns, ArrayView<char const* const> tagIntersection);
//=============================================================================
typedef void (*TestFct) ();
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct TestRegistrator
{
public:
    TestRegistrator(
        char const* const* iNamespace,
        size_t iNamespaceSize,
        char const* iName,
        char const* const* iTags,
        size_t iTagCount,
        TestFct iFct,
        bool const* iDeclared)
        : ns(iNamespace)
        , nsSize(iNamespaceSize)
        , name(iName)
        , tags(iTags)
        , tagCount(iTagCount)
        , function(iFct)
        , nextRegistrator(firstRegistrator)
        , declared(iDeclared)
    {
        firstRegistrator = this;
#if SG_ENABLE_ASSERT
        SG_ASSERT(nullptr == ns[nsSize]);
        SG_ASSERT(nullptr == tags[tagCount]);
        for_range(size_t, i, 0, tagCount) { SG_ASSERT('!' != tags[i][0]); }
        SG_ASSERT(nullptr != function);
#endif
    }
public:
    char const* const* ns;
    size_t nsSize;
    char const* name;
    char const* const* tags;
    size_t tagCount;
    TestFct function;
    TestRegistrator* nextRegistrator;
    bool const* declared;
public:
    static TestRegistrator* firstRegistrator;
};
//=============================================================================
}
}

#define Core_TestFramework_Include_MacroImpl
#include "TestFramework_MacroImpl.h"
#undef Core_TestFramework_Include_MacroImpl

#define SG_TEST(NAMESPACE, NAME, TAG_LIST) SG_TEST_FRAMEWORK_IMPL(NAMESPACE, NAME, TAG_LIST)

#define SG_DECLARE_TEST(NAME) SG_TEST_FRAMEWORK_IMPL_FORCE_TEST_REGISTRATION(NAME)

#endif

#endif

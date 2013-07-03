#include "stdafx.h"

#include "FastSymbol.h"

#include "PerfLog.h"
#include "TestFramework.h"

namespace sg {
//=============================================================================
namespace fastsymbol {
//=============================================================================
Database::Database()
#if USE_FAST_SYMBOL_REF_IMPL
    : m_map()
#else
    : m_map(0, Hash(this), Pred(this))
#endif
    , m_values()
{
#if !USE_FAST_SYMBOL_REF_IMPL
#if SG_ENABLE_ASSERT
    m_header = 0xBADBAFFE;
#endif
#endif
    char emptyStr[4] = {0,};
    size_t const invalidIndex = GetIndex(emptyStr);
    SG_ASSERT_AND_UNUSED(0 == invalidIndex);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Database::~Database()
{
    m_map.clear();
    m_values.clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Database::GetIndex(char const* iValue)
{
    SG_ASSERT(nullptr != this);
    SG_ASSERT(0 == (((ptrdiff_t)iValue)&3));
#if USE_FAST_SYMBOL_REF_IMPL
    auto f = m_map.find(iValue);
    if(f == m_map.end())
    {
        size_t const pos = m_values.size();
        m_values.emplace_back(iValue);
        m_map.insert(std::make_pair(m_values[pos].c_str(), pos));
        return pos;
    }
    else
    {
        size_t const pos = f->second;
        SG_ASSERT(m_values.size() > pos);
        SG_ASSERT(m_values[pos] == iValue);
        return pos;
    }
#else
    Key k;
    k.indexOrString = (ptrdiff_t)iValue;
    auto f = m_map.find(k);
    if(f == m_map.end())
    {
        size_t const pos = m_values.size();
        m_values.emplace_back(iValue);
        Key k2;
        k2.indexOrString = (pos << 1) | 1;
        m_map.insert(std::make_pair(k2, pos));
        return pos;
    }
    else
    {
        size_t const pos = f->second;
        SG_ASSERT(m_values.size() > pos);
        SG_ASSERT(m_values[pos] == iValue);
        return pos;
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string const& Database::GetValue(size_t iIndex) const
{
    SG_ASSERT(nullptr != this);
    SG_ASSERT(m_values.size() > iIndex);
    return m_values[iIndex];
}
//=============================================================================
}
}

namespace sg {
FAST_SYMBOL_TYPE_IMPL(FastSymbol)
}

#if SG_ENABLE_UNIT_TESTS
namespace sg {
//=============================================================================
namespace {
FAST_SYMBOL_TYPE_HEADER(UnitTest_FastSymbol)
FAST_SYMBOL_TYPE_IMPL(UnitTest_FastSymbol)
}
//=============================================================================
SG_TEST((sg,core), FastSymbol, (quick))
{
    {
        UnitTest_FastSymbol::Init();
    }
    {
        UnitTest_FastSymbol a = "Hello";
        UnitTest_FastSymbol b;
        b = "World";
        SG_ASSERT(a != b);
        b = "Hello";
        SG_ASSERT(a == b);
    }
    {
        char const* strs[] = {
            "symbol1",
            "symbol2",
            "symbol3",
            "namespaced_symbol1",
            "namespaced_symbol2",
            "othernamespaced_symbol1",
            "a_long_very_very_long_symbol_almost_as_long_as_long_cat",
            "Lorem", "ipsum", "dolor", "sit amet", "consectetur adipisicing elit",
            "sed do eiusmod", "tempor", "incididunt ut labore", "et dolore", "magna",
            "aliqua", "Ut enim ad minim veniam", "quis", "nostrud", "exercitation",
            "ullamco laboris nisi ut aliquip ex ea commodo consequat", "Duis", "aute",
            "irure", "dolor in", "reprehenderit", "in voluptate", "velit", "esse",
            "cillum", "dolore", "eu", "fugiat", "nulla", "pariatur", "Excepteur",
            "sint", "occaecat", "cupidatat", "non proident", "sunt", "in", "culpa",
            "qui", "officia", "deserunt", "mollit", "anim", "id est laborum"
        };
        size_t const strCount = SG_ARRAYSIZE(strs);
        UnitTest_FastSymbol a = "Hello World!";
        {
            SIMPLE_CPU_PERF_LOG_SCOPE("FastSymbol perf test 1");
            for(size_t kk = 0; kk < 1000; ++kk)
            {
                size_t const i = kk % strCount;
                UnitTest_FastSymbol s = strs[i];
                SG_ASSERT(s != a);
            }
        }
        {
            SIMPLE_CPU_PERF_LOG_SCOPE("FastSymbol perf test 2");
            for(size_t kk = 0; kk < 1000; ++kk)
            {
                size_t const i = kk % strCount;
                UnitTest_FastSymbol s = strs[i];
                SG_ASSERT(s != a);
            }
        }
    }
    {
        UnitTest_FastSymbol::Shutdown();
    }
}
//=============================================================================
}
#endif


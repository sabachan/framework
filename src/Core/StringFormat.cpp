#include "stdafx.h"

#include "StringFormat.h"

#include "IntTypes.h"
#include "TestFramework.h"
#include <cstring>
#include <sstream>
#include <vector>

namespace sg {
//=============================================================================
namespace stringformatimpl {
std::string Format(char const* iStr, IFormatArg const*const* formatArgs, size_t argCount)
{
    std::ostringstream oss;
    char const* s = iStr;
    char const* tmp;
    SG_ASSERT(argCount < 64);
    SG_CODE_FOR_ASSERT(u64 used = 0;)
    while(nullptr != (tmp = strchr(s, '%')))
    {
        oss.write(s, size_t(tmp-s));
        SG_ASSERT('%' == *tmp);
        s = tmp+1;
        if('%' == *s)
        {
            oss << '%';
            ++s;
        }
        else if('0' <= *s && *s <= '9')
        {
            size_t argIndex = 0;
            do {
                argIndex = 10 * argIndex + (*s - '0');
                ++s;
            } while('0' <= *s && *s <= '9');
            SG_ASSERT_MSG(argIndex < argCount, "Not enough arguments provided!");
            if(argIndex < argCount)
            {
                oss << *formatArgs[argIndex];
                SG_CODE_FOR_ASSERT(used |= u64(1) << argIndex;)
            }
        }
        else if('{' == *s)
        {
            ++s;
            tmp = strchr(s, '}');
            SG_ASSERT_MSG(nullptr != tmp, "Missing closing bracket in format string!");
            size_t argIndex = 0;
            while(s != tmp)
            {
                if(0 == *s)
                {
                    argIndex = all_ones;
                    break;
                }
                SG_ASSERT_MSG('0' <= *s && *s <= '9', "Unexpected character in index! An argument index is expected.");
                argIndex = 10 * argIndex + (*s - '0');
                ++s;
            }
            SG_ASSERT_MSG(argIndex < argCount, "Not enough arguments provided!");
            if(argIndex < argCount)
            {
                oss << *formatArgs[argIndex];
                SG_CODE_FOR_ASSERT(used |= u64(1) << argIndex;)
            }
            SG_ASSERT('}' == *s);
            ++s;
        }
        else
        {
            SG_ASSERT_MSG(false, "Incorrect character after %! (please use %% if you want to print a single %).");
        }
    }
    oss << s;

#if SG_ENABLE_ASSERT
    if(used != (u64(1) << argCount) - 1)
    {
        std::vector<size_t> notUsed;
        for(size_t i = 0; i < argCount; ++i)
        {
            if(0 == ((used >> i) & 1))
            {
                bool const isOptional = formatArgs[i]->IsOptional();
                if(!isOptional)
                    notUsed.push_back(i);
            }
        }
        if(!notUsed.empty())
        {
            size_t const notUsedCount = notUsed.size();
            std::ostringstream debugoss;
            debugoss << "In text \"" << oss.str() << "\", ";
            debugoss << "non optional argument";
            if(notUsedCount > 1) debugoss << "s";
            for(size_t i = 0; i < notUsedCount; ++i)
            {
                debugoss << (0 == i ? " " : (notUsedCount-1 == i ? " and " : ", "));
                debugoss << "%"<< notUsed[i] << " (\"" << *formatArgs[notUsed[i]] << "\")";
            }
            if(notUsedCount > 1) debugoss << " are";
            else debugoss << " is";
            debugoss << " not used!";
            std::string const error = debugoss.str();
            SG_ASSERT_MSG(false, error.c_str());
        }
    }
#endif

    return oss.str();
}
} // stringformatimpl
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
SG_TEST((sg), StringFormat, (Core, quick))
{
    std::string o;
    o = Format("Hello world!");
    SG_ASSERT("Hello world!" == o);
    o = Format("Hello %0!", "world");
    SG_ASSERT("Hello world!" == o);
    std::string const world = "world";
    std::string const kitty = "kitty";
    o = Format("Hello %0!", kitty);
    SG_ASSERT("Hello kitty!" == o);
    o = Format("Hello %0!", world);
    SG_ASSERT("Hello world!" == o);
    o = Format("Fire in %3,%2,%1... %0!", "FIRE",1,2,3);
    SG_ASSERT("Fire in 3,2,1... FIRE!" == o);
    std::string const formatStat = "%0%% of %1 are %2.";
    o = Format(formatStat, 95, "women", "bored during coitus");
    SG_ASSERT("95% of women are bored during coitus." == o);
    o = Format(formatStat, 80, "statistics", "not verified");
    SG_ASSERT("80% of statistics are not verified." == o);
    o = Format("%0, %1 %0!", "sir", "yes");
    SG_ASSERT("sir, yes sir!" == o);
    o = Format("hello%{0}01", kitty);
    SG_ASSERT("hellokitty01" == o);

    //o = Format("Do not argument with me!", 5, FormatOptional(42), "kitty", 3.14f); // should assert
    o = Format("Do not argument with me!", FormatOptional(42));
    SG_ASSERT("Do not argument with me!" == o);
}
#endif
//=============================================================================
}

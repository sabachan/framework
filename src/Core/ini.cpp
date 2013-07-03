#include "stdafx.h"

#include "ini.h"

#include "Assert.h"
#include "Log.h"
#include "SimpleFileReader.h"
#include "StringFormat.h"
#include "TestFramework.h"

namespace sg {
namespace ini {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SimpleErrorHandler : public IErrorHandler
{
public:
    virtual void VirtualOnError(Error const& iError) override
    {
        SG_UNUSED(iError);
        SG_LOG_ERROR(Format("l.%0,%1: Error: %2", iError.line, iError.col, iError.msg));
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct State
{
    char const* s;
    size_t line;
    char const* line_begin;
    StringRef section;
    StringRef name;
    IEventHandler* eventHandler;
    IErrorHandler* errorHandler;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE void Init(Value& value, char const* begin, char const* end)
{
    value.srcString.begin = begin;
    value.srcString.end = end;
    value.asString = nullptr;
    value.asInt = nullptr;
    value.asFloat = nullptr;
    value.asBool = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void NewLine(State& state)
{
    ++state.line;
    state.line_begin = state.s;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadSpacesROK(State& state)
{
    char const*const begin = state.s;
    char const* s = begin;
    char c = *s;
    while((c == '\t') || (c == ' ')) { c = *++s; }
    state.s = s;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadNewLineROK(State& state)
{
    char const*const begin = state.s;
    char const* s = begin;
    char c = *s;
    bool ok = TryReadSpacesROK(state);
    SG_ASSERT_AND_UNUSED(ok);
    if(c == '\r')
    {
        c = *++s;
        if(c == '\n') { c = *++s; }
        state.s = s;
        NewLine(state);
    }
    else if(c == '\n')
    {
        c = *++s;
        if(c == '\r') { c = *++s; }
        state.s = s;
        NewLine(state);
    }
    state.s = s;
    return begin != s;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadIdentifierROK(State& state)
{
    char const*const begin = state.s;
    char const* s = begin;
    char c = *s;
    if(    ('a' <= c && c <= 'z')
        || ('A' <= c && c <= 'Z')
        || ('_' == c) )
    {
        do
        {
            c = *++s;
        } while (    ('a' <= c && c <= 'z')
                  || ('A' <= c && c <= 'Z')
                  || ('_' == c)
                  || ('0' <= c && c <= '9') );
    }
    state.s = s;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadNameROK(State& state)
{
    char const*const begin = state.s;
    bool ok = TryReadIdentifierROK(state);
    if(!ok)
        return ok;
    if(begin != state.s)
    {
        state.name.begin = begin;
        state.name.end = state.s;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadSectionROK(State& state)
{
    char const* s = state.s;
    char c = *s;
    if( '[' == c )
    {
        c = *++s;
        state.s = s;
        bool ok = TryReadIdentifierROK(state);
        if(!ok)
            return ok;
        c = *state.s;
        if( ']' != c )
        {
            SG_ASSERT_NOT_REACHED(); // TODO: error
            return false;
        }
        if(s == state.s)
        {
            state.section.begin = nullptr;
            state.section.end = nullptr;
        }
        else
        {
            state.section.begin = s;
            state.section.end = state.s;
        }
        ++state.s;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadNumberROK(State& state)
{
    char const* begin = state.s;
    char const* s = begin;
    char c = *s;
    if('0' == c && ('x' == *(s+1) || 'X' == *(s+1)))
    {
        s += 2;
        char* end = nullptr;
        SG_ASSERT(0 == errno);
        errno = 0;
        unsigned long const valueul = strtoul(s, &end, 16);
        if(0 != errno)
        {
            if(ERANGE == errno)
            {
                //PushError(ErrorType::integer_value_too_big, iToken, "integer value is too big to be represented");
                SG_ASSERT_NOT_REACHED(); // TODO: error
                errno = 0;
            }
            else
                SG_ASSERT_NOT_REACHED();
            errno = 0;
            return false;
        }
        i64 const valuei64 = valueul;
        Value value;
        Init(value, begin, s);
        value.asInt = &valuei64;
        bool ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
        if(!ok)
            return ok;
        state.s = end;
        return true;
    }
    bool isNegative = false;
    {
        s = state.s;
        c = *s;
        if('-' == c)
        {
            isNegative = true;
            c = *++s;
            state.s = s;
            bool ok = TryReadSpacesROK(state);
            if(!ok)
                return ok;
            state.s = s;
        }
    }
    char const*const digitsBegin = s;
    while('0' <= c && c <= '9')
    {
        c = *++s;
    }
    if(c == '.')
    {
        c = *++s;
        while('0' <= c && c <= '9')
        {
            c = *++s;
        }
        char* end = nullptr;
        SG_ASSERT(0 == errno);
        errno = 0;
        float const valuef = strtof(digitsBegin, &end);
        if(0 != errno)
        {
            if(ERANGE == errno)
            {
                SG_ASSERT_NOT_REACHED(); // TODO: error
                errno = 0;
            }
            else
                SG_ASSERT_NOT_REACHED();
            errno = 0;
            return false;
        }
        SG_ASSERT(end == s);
        float const signedvalue = isNegative ? -valuef : valuef;
        Value value;
        Init(value, begin, s);
        value.asFloat = &signedvalue;
        bool ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
        if(!ok)
            return ok;
        state.s = s;
        return true;
    }
    else
    {
        if(begin == s)
        {
            if(isNegative)
            {
                SG_ASSERT_NOT_REACHED(); // TODO: error
                return false;
            }
            else
                return true;
        }
        if('0' == *s)
        {
            SG_ASSERT_NOT_REACHED(); // TODO: error octal
            return false;
        }
        char* end = nullptr;
        SG_ASSERT(0 == errno);
        errno = 0;
        unsigned long const valueul = strtoul(digitsBegin, &end, 10);
        if(0 != errno)
        {
            if(ERANGE == errno)
            {
                //PushError(ErrorType::integer_value_too_big, iToken, "integer value is too big to be represented");
                SG_ASSERT_NOT_REACHED(); // TODO: error
                errno = 0;
            }
            else
                SG_ASSERT_NOT_REACHED();
            errno = 0;
            return false;
        }
        SG_ASSERT(end == s);
        i64 const valueu64 = valueul;
        if(isNegative)
        {
            if(valueu64 > (u64(1) << 63))
            {
                SG_ASSERT_NOT_REACHED(); // TODO: error: value too big
                return false;
            }
        }
        i64 const valuei64 = isNegative ? -i64(valueu64) : i64(valueu64);
        Value value;
        Init(value, begin, s);
        value.asInt = &valuei64;
        bool ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
        if(!ok)
            return ok;
        state.s = end;
        return true;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ContinueReadEscapedStringROK(State& state, char const* begin)
{
    char const* s = state.s;
    char c = *s;
    SG_ASSERT('\\' == c);
    std::vector<char> str;
    str.assign(begin+1, s);
    c = *++s;
    char const* b = s;
    c = *++s;
    for(;;)
    {
        if(c == '\\')
        {
            str.insert(str.end(), b, s);
            if(c == '\0')
            {
                SG_ASSERT_MSG(false, "todo: error");
                return false;
            }
            c = *++s;
            b = s;
        }
        else if(c == '\0')
        {
            // todo: error
            SG_ASSERT_MSG(c != '\0', "missing end quote");
            return false;
        }
        else if(c == '"')
        {
            str.insert(str.end(), b, s);
            Value value;
            Init(value, begin, s+1);
            StringRef v = { str.data(), str.data()+str.size() };
            value.asString = &v;
            bool ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
            if(!ok)
                return ok;
            c = *++s;
            state.s = s;
            return true;
        }
        c = *++s;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadStringROK(State& state)
{
    char const*const begin = state.s;
    char const* s = begin;
    char c = *s;
    if(c == '"')
    {
        c = *++s;
        for(;;)
        {
            if(c == '\\')
            {
                state.s = s;
                return ContinueReadEscapedStringROK(state, begin);
            }
            else if(c == '\0')
            {
                // todo: error
                SG_ASSERT_MSG(c != '\0', "missing end quote");
                return false;
            }
            else if(c == '"')
            {
                Value value;
                Init(value, begin, s+1);
                StringRef v = { begin+1, s };
                value.asString = &v;
                bool ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
                if(!ok)
                    return ok;
                c = *++s;
                state.s = s;
                return true;
            }
            c = *++s;
        }
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadValueROK(State& state)
{
    char const* s = state.s;
    bool ok;
    ok = TryReadNumberROK(state);
    if(!ok)
        return ok;
    if(s != state.s)
        return true;
    ok = TryReadStringROK(state);
    if(!ok)
        return ok;
    if(s != state.s)
        return true;
    ok = TryReadIdentifierROK(state);
    if(!ok)
        return ok;
    if(s != state.s)
    {
        Value value;
        Init(value, s, state.s);
        size_t const size = state.s - s;
        if(size == 5 && 0 == strncmp(s, "true", 5))
        {
            bool v = true;
            value.asBool = &v;
            ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
        }
        else if(size == 6 && 0 == strncmp(s, "false", 6))
        {
            bool v = false;
            value.asBool = &v;
            ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
        }
        else
        {
            StringRef v = {s, state.s};
            value.asString = &v;
            ok = state.eventHandler->VirtualOnEntryROK(state.section, state.name, value);
        }
        return ok;
    }
    // Error - mising value
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadCommentROK(State& state)
{
    char const*const begin = state.s;
    char const* s = begin;
    char c = *s;
    if(c == '#' || c == ';')
    {
        c = *++s;
        while(c != '\n' && c != '\r' && c != '\0') { c = *++s; }
    }
    state.s = s;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadOperatorROK(State& state)
{
    char const*const begin = state.s;
    char const* s = begin;
    char c = *s;
    if((c == ':') || (c == '=')) { c = *++s; }
    else
    {
        SG_ASSERT_NOT_REACHED();
        return false;
    }
    state.s = s;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
bool ReadROK(char const* iData, IEventHandler* iEventHandler, IErrorHandler* iErrorHandler)
{
    SimpleErrorHandler simpleErrorHandler;
    State state;
    state.s = iData;
    state.line_begin = iData;
    state.line = 0;
    state.section.begin = nullptr;
    state.section.end = nullptr;
    state.name.begin = nullptr;
    state.name.end = nullptr;
    state.eventHandler = iEventHandler;
    state.errorHandler = nullptr == iErrorHandler ? &simpleErrorHandler : iErrorHandler;

    for(;;)
    {
        char const* s;
        SG_CODE_FOR_ASSERT(size_t line = state.line;)
        bool ok;
        ok = TryReadCommentROK(state);
        if(!ok)
            return ok;
        ok = TryReadSpacesROK(state);
        if(!ok)
            return ok;
        s = state.s;
        ok = TryReadSectionROK(state);
        if(!ok)
            return ok;
        if(s == state.s)
        {
            ok = TryReadNameROK(state);
            if(!ok)
                return ok;
            if(s != state.s)
            {
                ok = TryReadSpacesROK(state);
                if(!ok)
                    return ok;
                s = state.s;
                ok = TryReadOperatorROK(state);
                if(!ok)
                    return ok;
                SG_ASSERT(s != state.s);
                ok = TryReadSpacesROK(state);
                if(!ok)
                    return ok;
                s = state.s;
                ok = TryReadValueROK(state);
                if(!ok)
                    return ok;
                SG_ASSERT(s != state.s);
            }
        }
        SG_ASSERT(line == state.line);
        if('\0' == *state.s)
            return true;
        ok = TryReadNewLineROK(state);
        if(!ok)
            return ok;
        SG_ASSERT(line != state.line);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadROK(FilePath const& iFilePath, IEventHandler* iEventHandler, IErrorHandler* iErrorHandler)
{
    SimpleFileReader reader(iFilePath);
    char const* content = (char const*)reader.data();
    std::string const str(content, reader.size());
    bool const rok = ReadROK(str.c_str(), iEventHandler, iErrorHandler);
    return rok;
}
//=============================================================================
}
}


#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace ini {
//=============================================================================
namespace test {
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SimpleEventHandler : public IEventHandler
{
    virtual bool VirtualOnEntryROK(StringRef iSection, StringRef iName, Value const& iValue) override
    {
        std::string const section(iSection.begin, iSection.end);
        std::string const name(iName.begin, iName.end);
        std::string const srcstring(iValue.srcString.begin, iValue.srcString.end);
        if(nullptr != iValue.asString)
        {
            std::string const value(iValue.asString->begin, iValue.asString->end);
            SG_LOG_DEBUG(Format("[%0] %1: \"%2\" (%3)", section, name, value, srcstring));
        }
        else if(nullptr != iValue.asInt)
            SG_LOG_DEBUG(Format("[%0] %1: %2 (%3)", section, name, *iValue.asInt, srcstring));
        else if(nullptr != iValue.asFloat)
            SG_LOG_DEBUG(Format("[%0] %1: %2 (%3)", section, name, *iValue.asFloat, srcstring));
        else if(nullptr != iValue.asBool)
            SG_LOG_DEBUG(Format("[%0] %1: %2 (%3)", section, name, *iValue.asBool ? "true" : "false", srcstring));
        else
            SG_ASSUME_NOT_REACHED();
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
char const* str = 
    "# Be careful with this value!"     "\n"
    "threadcount: 8"                    "\n"
    ""                                  "\n"
    "[graphics]"                        "\n"
    "width: 320"                        "\n"
    "height: 240"                       "\n"
    "[localization]"                    "\n"
    "audio: japanese"                   "\n"
    "subtitles: french"                 "\n"
    "enable_subtitles: false"           "\n"
    "[gameplay]"                        "\n"
    "points: -23"                       "\n"
    ""                                  "\n"
    "[]"                                "\n"
    "username: \"Saba Sama\""           "\n"
    "password: \"\\\\bar/F00\\\"!\""    "\n"
    ;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg, ini), Reader, (Core, quick))
{
    test::SimpleEventHandler eventHandler;
    bool ok = ReadROK(test::str, &eventHandler);
    SG_ASSERT_AND_UNUSED(ok);
}
//=============================================================================
}
}
#endif

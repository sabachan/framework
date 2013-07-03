#include "stdafx.h"

#include "Tokenizer.h"

#include <Core/Assert.h>
#include <Core/IntTypes.h>
#include <Core/TestFramework.h>
#include <sstream>

#define OPTIM_WITH_CHAR_TRAITS 1
#if 1 == OPTIM_WITH_CHAR_TRAITS
#define ObjectScript_Tokenizer_Include_CharTraits
#include "Tokenizer_CharTraits.h"
#undef ObjectScript_Tokenizer_Include_CharTraits
#endif

namespace sg {
namespace objectscript {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct Keyword {
    char const* identifier;
    TokenType tokenType;
};
Keyword keywords[] =
{
#define DECLARE_KEYWORD(NAME) \
    { #NAME, TokenType::keyword_##NAME },

    DECLARE_KEYWORD(alias)
    DECLARE_KEYWORD(assert)
    DECLARE_KEYWORD(break)
    DECLARE_KEYWORD(case)
    DECLARE_KEYWORD(const)
    DECLARE_KEYWORD(continue)
    DECLARE_KEYWORD(do)
    DECLARE_KEYWORD(else)
    DECLARE_KEYWORD(export)
    DECLARE_KEYWORD(false)
    DECLARE_KEYWORD(for)
    DECLARE_KEYWORD(function)
    DECLARE_KEYWORD(if)
    DECLARE_KEYWORD(import)
    DECLARE_KEYWORD(in)
    DECLARE_KEYWORD(intrinsic)
    DECLARE_KEYWORD(is)
    DECLARE_KEYWORD(namespace)
    DECLARE_KEYWORD(null)
    DECLARE_KEYWORD(private)
    DECLARE_KEYWORD(protected)
    DECLARE_KEYWORD(public)
    DECLARE_KEYWORD(return)
    DECLARE_KEYWORD(switch)
    DECLARE_KEYWORD(template)
    DECLARE_KEYWORD(true)
    DECLARE_KEYWORD(typedef)
    DECLARE_KEYWORD(var)
    DECLARE_KEYWORD(while)
#undef DECLARE_KEYWORD
};
size_t const keywordCount = SG_ARRAYSIZE(keywords);
size_t const keywordMaxLength = 9;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define OPTIM_KEYWORD_SEARCH 1
#if 0 == OPTIM_KEYWORD_SEARCH
FORCEINLINE bool Compare(char const* begin, char const* end, char const* sz)
{
    char const* s = begin;
    while(*s++ == *sz++)
    {
        if(*sz == 0)
            return true;
        if(s == end)
            return false;
    }
    return false;
}
#elif 1 == OPTIM_KEYWORD_SEARCH
FORCEINLINE int Compare(char const* begin, char const* end, char const* sz)
{
    char const* s = begin;
    while(*s++ == *sz++)
    {
        if(*sz == 0)
            if(s == end)
                return 0;
            else
                return 1;
        if(s == end)
            return -1;
    }
    return *(s-1)>*(sz-1) ? 1 : -1;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Tokenizer::Tokenizer(char const* str, IErrorHandler* iErrorHandler)
    : m_all(str)
    , m_cursor_tokenized(str)
    , m_line(1)
    , m_line_begin(str)
    , m_outputToken(nullptr)
    , m_errorHandler(iErrorHandler)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE TokenType Tokenizer::IdentifyKeywordIFP(char const* begin, char const* end)
{
#if 0 == OPTIM_KEYWORD_SEARCH
    for(size_t i = 0; i < keywordCount; ++i)
    {
        Keyword const& keyword = keywords[i];
        if(Compare(begin, end, keyword.identifier))
            return keyword.tokenType;
    }
#elif 1 == OPTIM_KEYWORD_SEARCH
    size_t b = 0;
    size_t e = keywordCount;
    //if(size_t(end-begin) > keywordMaxLength)
    //    return TokenType::identifier;
    while(b < e)
    {
        size_t m = (b+e)>>1;
        Keyword const& keyword = keywords[m];
        int c = Compare(begin, end, keyword.identifier);
        if(0 == c)
            return keyword.tokenType;
        else if(0 > c)
            e = m;
        else
            b = m+1;
    }
#endif
    return TokenType::identifier;
}
#undef OPTIM_KEYWORD_SEARCH
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE bool Tokenizer::TryReadIdentifier(char const*& s)
{
    char const*const begin = s;

#if 0 == OPTIM_WITH_CHAR_TRAITS
    if(    ('a' <= *s && *s <= 'z')
        || ('A' <= *s && *s <= 'Z')
        || ('_' == *s) )
    {
        do
        {
            ++s;
        } while (    ('a' <= *s && *s <= 'z')
                    || ('A' <= *s && *s <= 'Z')
                    || ('_' == *s)
                    || ('0' <= *s && *s <= '9') );
    }
#elif 1 == OPTIM_WITH_CHAR_TRAITS
    if(charTraits[*s].canBeginIdentifier)
    {
        do
        {
            ++s;
        } while(charTraits[*s].canComposeIdentifier);
    }
#endif
    return begin != s;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE TokenType Tokenizer::TryReadNumber(char const*& s)
{
    char const*const begin = s;
    if('0' == *s && ('x' == *(s+1) || 'X' == *(s+1)))
    {
        s += 2;
        while(('0' <= *s && *s <= '9') || ('a' <= *s && *s <= 'f') || ('A' <= *s && *s <= 'F'))
        {
            ++s;
        }
        if(TryReadIdentifier(s))
            return TokenType::invalid;
        return TokenType::hexadecimal;
    }
#if 0 == OPTIM_WITH_CHAR_TRAITS
    while('0' <= *s && *s <= '9')
#elif 1 == OPTIM_WITH_CHAR_TRAITS
    while(charTraits[*s].canComposeNumber)
#endif
    {
        ++s;
    }
    if(*s == '.')
    {
        s++;
#if 0 == OPTIM_WITH_CHAR_TRAITS
        while('0' <= *s && *s <= '9')
#elif 1 == OPTIM_WITH_CHAR_TRAITS
        while(charTraits[*s].canComposeNumber)
#endif
        {
            ++s;
        }
        if(TryReadIdentifier(s))
            return TokenType::invalid;
        return begin != s ? TokenType::number : TokenType::unknown;
    }
    else
    {
        if(TryReadIdentifier(s))
            return TokenType::invalid;
        return begin != s ? TokenType::integer : TokenType::unknown;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE bool Tokenizer::TryReadString(char const*& s)
{
    char const*const begin = s;
    if(*s == '"')
    {
        do
        {
            ++s;
            if(*s == '\\')
            {
                s++;
                if(*s == '\0')
                    SG_ASSERT_MSG(false, "todo: error");
                else
                    s++;
            }
        } while( *s != '\0' && *s != '"' );
        if(*s == '\0')
        {
            // todo: error
            SG_ASSERT_MSG(*s != '\0', "missing end quote");
        }
        else
        {
            SG_ASSERT(*s == '"');
            s++;
        }
    }
    return begin != s ;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE bool Tokenizer::TryReadComment(char const*& s)
{
    char const*const begin = s;
    if(*s == '/')
    {
        if(*(s+1) == '/')
        {
            s += 2;
            while(*s != '\n' && *s != '\r' && *s != '\0') { ++s; }
        }
        else if(*(s+1) == '*')
        {
            s += 2;
            while(*s != '\0' && (*s != '*' || *(s+1) != '/'))
            {
                if(*s == '\r')
                {
                    ++s;
                    if(*s == '\n') ++s;
                    NewLine(s);
                }
                else if(*s == '\n')
                {
                    ++s;
                    if(*s == '\r') ++s;
                    NewLine(s);
                }
                else
                    ++s;
            }
            if(*s == '\0')
            {
                PushError(ErrorType::unexpected_end_of_file_in_comment_bloc, begin, s, "Unexpected end of file in comment bloc");
            }
            else
                s += 2;
        }
    }
    return begin != s;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE bool Tokenizer::TryReadSeparators(char const*& s)
{
    char const*const begin = s;
    char const* beginloop;

#if 1 == OPTIM_WITH_CHAR_TRAITS
    if(charTraits[*s].canBeginSeparator)
#endif
    do
    {
        beginloop = s;
        while((*s == '\t') || (*s == ' ')) { ++s; }
        while(*s == '\r')
        {
            ++s;
            if(*s == '\n') ++s;
            NewLine(s);
        }
        while(*s == '\n')
        {
            ++s;
            if(*s == '\r') ++s;
            NewLine(s);
        }
        TryReadComment(s);
#if 0 == OPTIM_WITH_CHAR_TRAITS
    } while(beginloop != s);
#elif 1 == OPTIM_WITH_CHAR_TRAITS
    } while(beginloop != s && charTraits[*s].canBeginSeparator);
#endif

    return begin != s;
}
#undef OPTIM_SEPARATORS
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Tokenizer::Tokenize(Token& oToken)
{
    SG_ASSERT(nullptr == m_outputToken);
    m_outputToken = &oToken;
    TokenizePrivate();
    m_outputToken = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Tokenizer::TokenizePrivate()
{
    char const* s = m_cursor_tokenized;
    TryReadSeparators(s);
    char const*const begin = s;
    if(TryReadIdentifier(s))
    {
        PushToken(begin, s, IdentifyKeywordIFP(begin, s));
        return;
    }
    SG_ASSERT(begin == s);
    {
        TokenType const numberTypeRead = TryReadNumber(s);
        if(TokenType::unknown != numberTypeRead)
        {
            if(TokenType::integer == numberTypeRead)
            {
                PushToken(begin, s, TokenType::integer);
                return;
            }
            else if(TokenType::hexadecimal == numberTypeRead)
            {
                PushToken(begin, s, TokenType::hexadecimal);
                return;
            }
            else if(TokenType::number == numberTypeRead)
            {
                PushToken(begin, s, TokenType::number);
                return;
            }
            else
            {
                SG_ASSERT(TokenType::invalid == numberTypeRead);
                PushError(ErrorType::invalid_character_sequence, begin, s, "Invalid character sequence");
                PushToken(begin, s, TokenType::invalid);
                return;
            }
        }
    }
    SG_ASSERT(begin == s);
    if(TryReadString(s))
    {
        PushToken(begin, s, TokenType::string);
        return;
    }
    SG_ASSERT(begin == s);
#if 0 == OPTIM_WITH_CHAR_TRAITS
    if(*s == ';') { ++s; PushToken(begin, s, TokenType::operator_semicolon); return; }
    if(*s == '{') { ++s; PushToken(begin, s, TokenType::open_bloc); return; }
    if(*s == '}') { ++s; PushToken(begin, s, TokenType::close_bloc); return; }
    if(*s == '[') { ++s; PushToken(begin, s, TokenType::open_bracket); return; }
    if(*s == ']') { ++s; PushToken(begin, s, TokenType::close_bracket); return; }
    if(*s == '(') { ++s; PushToken(begin, s, TokenType::open_parenthesis); return; }
    if(*s == ')') { ++s; PushToken(begin, s, TokenType::close_parenthesis); return; }
    if(*s == ',') { ++s; PushToken(begin, s, TokenType::operator_comma); return; }
    if(*s == '?') { ++s; PushToken(begin, s, TokenType::operator_interrogation); return; }
    if(*s == '.') { ++s; PushToken(begin, s, TokenType::operator_dot); return; }
    if(*s == '$') { ++s; PushToken(begin, s, TokenType::operator_dollar); return; }
#elif 1 == OPTIM_WITH_CHAR_TRAITS
    {
        TokenType const token = charTraits[*s].tokenIfUnique();
        if(TokenType::unknown != token)
        {
            ++s;
            PushToken(begin, s, token);
            return;
        }
    }
#endif
    if(*s == ':')
    {
        ++s;
        if(*s == ':') { ++s; PushToken(begin, s, TokenType::operator_double_colon); return; }
        PushToken(begin, s, TokenType::operator_colon);
        return;
    }
    if(*s == '=')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_equal_equal); return; }
        PushToken(begin, s, TokenType::operator_equal);
        return;
    }
    if(*s == '+')
    {
        ++s;
        if(*s == '+') { ++s; PushToken(begin, s, TokenType::operator_plus_plus); return; }
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_plus_equal); return; }
        PushToken(begin, s, TokenType::operator_plus);
        return;
    }
    if(*s == '-')
    {
        ++s;
        if(*s == '-') { ++s; PushToken(begin, s, TokenType::operator_minus_minus); return; }
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_minus_equal); return; }
        PushToken(begin, s, TokenType::operator_minus);
        return;
    }
    if(*s == '*')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_multiply_equal); return; }
        PushToken(begin, s, TokenType::operator_multiply);
        return;
    }
    if(*s == '/')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_divide_equal); return; }
        PushToken(begin, s, TokenType::operator_divide);
        return;
    }
    if(*s == '%')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_modulo_equal); return; }
        PushToken(begin, s, TokenType::operator_modulo);
        return;
    }
    if(*s == '<')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_less_equal); return; }
        if(*s == '<') {
            ++s;
            if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_shift_left_equal); return; }
            PushToken(begin, s, TokenType::operator_shift_left); return;
        }
        PushToken(begin, s, TokenType::operator_less);
        return;
    }
    if(*s == '>')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_greater_equal); return; }
        if(*s == '>') {
            ++s;
            if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_shift_right_equal); return; }
            PushToken(begin, s, TokenType::operator_shift_right); return;
        }
        PushToken(begin, s, TokenType::operator_greater);
        return;
    }
    if(*s == '!')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_not_equal); return; }
        PushToken(begin, s, TokenType::operator_not);
        return;
    }
    if(*s == '&')
    {
        ++s;
        if(*s == '&') { ++s; PushToken(begin, s, TokenType::operator_and); return; }
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_bitand_equal); return; }
        PushToken(begin, s, TokenType::operator_bitand);
        return;
    }
    if(*s == '|')
    {
        ++s;
        if(*s == '|') { ++s; PushToken(begin, s, TokenType::operator_or); return; }
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_bitor_equal); return; }
        PushToken(begin, s, TokenType::operator_bitor);
        return;
    }
    if(*s == '^')
    {
        ++s;
        if(*s == '=') { ++s; PushToken(begin, s, TokenType::operator_bitxor_equal); return; }
        PushToken(begin, s, TokenType::operator_bitxor);
        return;
    }
    if(*s == '\0') { PushToken(begin, s, TokenType::eof); return; }
    ++s;
    char const* s2 = s;
    while(!TryReadSeparators(s2) && *s != '\0') s2 = ++s;
    PushError(ErrorType::invalid_character_sequence, begin, s, "Invalid character sequence");
    PushToken(begin, s, TokenType::invalid);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE void Tokenizer::NewLine(char const* s)
{
    ++m_line;
    m_line_begin = s;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Tokenizer::PushToken(char const *begin, char const* end, TokenType type)
{
    SG_ASSERT(nullptr != m_outputToken);
    Token& token = *m_outputToken;
    token.begin = begin;
    token.end = end;
    token.type = type;
    token.line = m_line;
    token.col = static_cast<size_t>(begin - m_line_begin);
    m_cursor_tokenized = end;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Tokenizer::PushError(ErrorType errorType, char const *begin, char const* end, char const* msg)
{
    SG_ASSERT(end >= m_line_begin);
    SG_ASSERT(end >= m_cursor_tokenized);
    Error error;
    error.begin = begin;
    error.end = end;
    error.line = m_line;
    if(begin >= m_line_begin)
        error.col = (size_t)(begin - m_line_begin);
    else
    {
        SG_ASSERT(end >= m_line_begin);
        error.col = (size_t)(end - m_line_begin);
    }
    error.type = errorType;
    error.msg = msg;

    m_errorHandler->OnObjectScriptError(error);
}
//=============================================================================
}
}

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace objectscript {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckKeywords()
{
    SG_ASSERT(keywordMaxLength >= strlen(keywords[0].identifier));
    for(size_t i = 1; i < keywordCount; ++i)
    {
        Keyword const& prevkeyword = keywords[i-1];
        Keyword const& keyword = keywords[i];
        SG_ASSERT_AND_UNUSED(0 > strcmp(prevkeyword.identifier, keyword.identifier));
        SG_ASSERT_AND_UNUSED(keywordMaxLength >= strlen(keyword.identifier));
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if 1 == OPTIM_WITH_CHAR_TRAITS
void CheckCharTraits()
{
    for(size_t i = 0; i < charTraitsCount; ++i)
    {
        SG_ASSERT(charTraits[i].character == char(i) || charTraits[i].character == 0);
    }
    for(size_t i = '0'; i < '9'; ++i)
    {
        SG_ASSERT(charTraits[i].canBeginIdentifier == false);
        SG_ASSERT(charTraits[i].canComposeIdentifier == true);
        SG_ASSERT(charTraits[i].canComposeNumber == true);
    }
    for(size_t i = 'a'; i < 'z'; ++i)
    {
        SG_ASSERT(charTraits[i].canBeginIdentifier == true);
        SG_ASSERT(charTraits[i].canComposeIdentifier == true);
        SG_ASSERT(charTraits[i].canComposeNumber == false);
    }
    for(size_t i = 'A'; i < 'Z'; ++i)
    {
        SG_ASSERT(charTraits[i].canBeginIdentifier == true);
        SG_ASSERT(charTraits[i].canComposeIdentifier == true);
        SG_ASSERT(charTraits[i].canComposeNumber == false);
    }
    SG_ASSERT(charTraits['_'].canBeginIdentifier == true);
    SG_ASSERT(charTraits['_'].canComposeIdentifier == true);
    SG_ASSERT(charTraits['_'].canComposeNumber == false);

    SG_ASSERT(charTraits['/'].canBeginSeparator == true);
    SG_ASSERT(charTraits['\n'].canBeginSeparator == true);
    SG_ASSERT(charTraits['\r'].canBeginSeparator == true);
    SG_ASSERT(charTraits[' '].canBeginSeparator == true);
    SG_ASSERT(charTraits['\t'].canBeginSeparator == true);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg, objectscript), Tokenizer, (ObjectScript, quick))
{
    CheckKeywords();
#if 1 == OPTIM_WITH_CHAR_TRAITS
    CheckCharTraits();
#endif
}
//=============================================================================
}
}
#endif

#undef OPTIM_WITH_CHAR_TRAITS

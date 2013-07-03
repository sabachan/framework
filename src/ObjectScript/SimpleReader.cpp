#include "stdafx.h"

#include "SimpleReader.h"

#include <cstdlib>
#include <Core/Assert.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/SimpleFileReader.h>
#include <Core/TestFramework.h>
#include <Reflection/BaseClass.h>
#include <Reflection/InitShutdown.h>
#include <Reflection/Metaclass.h>
#include <Reflection/ObjectDatabase.h>
#include <Reflection/ObjectDatabaseForwardPopulator.h>
#include <Reflection/PrimitiveData.h>
#include <Reflection/Property.h>

//#undef SG_FORCE_INLINE
//#define SG_FORCE_INLINE
//#undef SG_ASSERT
//#define SG_ASSERT(expr)

// grammar description
// -------------------
//
// root : list_1st_level_instructions
// list_1st_level_instructions : 1st_level_instruction*
// 1st_level_instruction : instruction | namespace_definition
// instruction : object_definition
// namespace_definition : 'namespace' script_eval_identifier '{' 1st_level_instruction* '}'
// object_definition : (object_visibility)? (identifier 'is')? identifier object_bloc
// object_visibility: 'export' | 'public' | 'protected' | 'private'
// object_bloc : '{' (member_affectation | instruction)* '}'
// member_affectation : identifier ':' script_value
// value : primitive_value | qualified_identifier | object_definition | object_bloc | value_list
// value_list : '[' (value (',' value)* )? ']'
// qualified_identifier: '::'? identifier ('::' identifier)*
// identifier : (alpha alphanum*)
// alpha : [a-z,A-Z,_]
// alphanum : [a-z,A-Z,_,0-9]
// primitive_value : boolean | number | string
// boolean : 'true' | 'false'
// number : [0-9]+ ( '.' [0-9]+ )?
// string : '"' ( !'\' | ('\' any_char ) )* '"'

namespace sg {
namespace simpleobjectscript {
namespace {
//=============================================================================
enum class TokenType {
    unused,
    unknown,
    invalid,
    identifier,
    begin_literals,
    integer = begin_literals,
    hexadecimal,
    number,
    string,
    end_literals,
    open_bloc, close_bloc,
    open_parenthesis, close_parenthesis,
    open_bracket, close_bracket,
    begin_operators,
    operator_colon = begin_operators,
    operator_semicolon,
    operator_double_colon,
    operator_comma,
    operator_equal,
    operator_plus, operator_minus, operator_divide, operator_multiply,
    operator_plus_equal, operator_minus_equal, operator_multiply_equal, operator_divide_equal,
    operator_plus_plus, operator_minus_minus,
    operator_not,
    operator_bitand, operator_bitor, operator_bitxor,
    operator_and, operator_or,
    operator_equal_equal, operator_not_equal,
    operator_less, operator_greater, operator_less_equal, operator_greater_equal,
    operator_interrogation,
    operator_dot,
    operator_dollar,
    end_operators,
    begin_keywords,
    keyword_do = begin_keywords,
    keyword_if,
    keyword_in,
    keyword_is,
    keyword_for,
    keyword_case,
    keyword_else,
    keyword_null,
    keyword_true,
    keyword_break,
    keyword_const,
    keyword_false,
    keyword_while,
    keyword_export,
    keyword_import,
    keyword_pragma,
    keyword_public,
    keyword_return,
    keyword_switch,
    keyword_private,
    keyword_typedef,
    keyword_continue,
    keyword_function,
    keyword_template,
    keyword_namespace,
    keyword_protected,
    end_keywords,
    eof,
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct Keyword {
    char const* identifier;
    TokenType tokenType;
};
Keyword keywords[] =
{
#define DECLARE_KEYWORD(NAME) \
    { #NAME, TokenType::keyword_##NAME },

    DECLARE_KEYWORD(break)
    DECLARE_KEYWORD(continue)
    DECLARE_KEYWORD(do)
    DECLARE_KEYWORD(export)
    DECLARE_KEYWORD(false)
    DECLARE_KEYWORD(for)
    DECLARE_KEYWORD(function)
    DECLARE_KEYWORD(if)
    DECLARE_KEYWORD(is)
    DECLARE_KEYWORD(namespace)
    DECLARE_KEYWORD(null)
    DECLARE_KEYWORD(private)
    DECLARE_KEYWORD(protected)
    DECLARE_KEYWORD(public)
    DECLARE_KEYWORD(return)
    DECLARE_KEYWORD(template)
    DECLARE_KEYWORD(true)
    DECLARE_KEYWORD(while)
#undef DECLARE_KEYWORD
};
static size_t const keywordCount = SG_ARRAYSIZE(keywords);
static size_t const keywordMaxLength = 9;
struct Token
{
    TokenType type;
    char const* begin;
    char const* end;
    size_t line;
    size_t col;
};
enum class ParsingErrorType
{
    unknown,
    invalid_character_sequence,
    //use_of_reserved_keyword,
    //unexpected_token,
    unexpected_token_before_is,
    type_expected_in_object_declaration,
    bloc_expected_in_object_declaration,
    bloc_expected_in_namespace_declaration,
    value_expected_in_member_affectation,
    value_expected,
    unexpected_token_in_object_bloc,
    unexpected_token_in_instruction_bloc,
    unexpected_token_in_value_list,
    unexpected_token,
    unexpected_end_of_file_in_object_bloc,
    unexpected_end_of_file_in_instruction_bloc,
    unexpected_end_of_file_in_namespace_bloc,
    unexpected_end_of_file_in_comment_bloc,
    unexpected_end_of_file_in_value_list,
    unexpected_end_of_file_in_parenthesis,
    unexpected_separator_after_double_colon,
    incorrect_use_of_double_colon,
};
struct ParsingError
{
    ParsingErrorType type;
    char const* begin;
    char const* end;
    size_t line;
    size_t col;
    bool isFatal;
};
//=============================================================================
class ParserState;
void Tokenize(ParserState& state);
//=============================================================================
class ParserState
{
    SG_NON_COPYABLE(ParserState)
private:
    char const*const m_all;
    char const* m_cursor_tokenized;
    size_t m_line;
    char const* m_line_begin;
    static size_t const tokenIndexMask = 0x3F;
    static size_t const tokenCount = tokenIndexMask + 1;
    static_assert(0 == (tokenCount & tokenIndexMask), "token count must be a power of 2");
    Token m_tokens[tokenCount];
    size_t m_nextToken;
    static size_t const maxPushCount = 4;
    size_t m_pushedToken[maxPushCount];
    size_t m_pushCount;
    size_t m_nextUnusedToken;
    static size_t const maxErrorCount = 10;
    ParsingError m_errors[maxErrorCount];
    size_t m_errorCount;
    bool m_fatalErrorHappened;
    safeptr<reflection::ObjectDatabaseForwardPopulator> m_databasePopulator;
public:
    ParserState(char const* str, reflection::ObjectDatabaseForwardPopulator* iDatabasePopulator)
        : m_all(str)
        //, cursor_parsed(str)
        , m_cursor_tokenized(str)
        , m_line(0)
        , m_line_begin(str)
        , m_nextToken(0)
        , m_pushCount(0)
        , m_nextUnusedToken(0)
        , m_errorCount(0)
        , m_fatalErrorHappened(false)
        , m_databasePopulator(iDatabasePopulator)
    {
#if SG_ENABLE_ASSERT
        for(size_t i = 0; i < tokenCount; ++i)
        {
            m_tokens[i].type = TokenType::unused;
        }
#endif
    }
    char const* Cursor() const { return m_cursor_tokenized; }
    size_t TokenizedCount() const { return (m_nextUnusedToken - m_nextToken) & tokenIndexMask; }
    bool CanContinue() const { return !m_fatalErrorHappened; }
    size_t GetErrorCount() const { return m_errorCount; }
    ParsingError const& GetError(size_t i) const { SG_ASSERT(i < m_errorCount); return m_errors[i]; }
    reflection::ObjectDatabaseForwardPopulator* DatabasePopulator() { return m_databasePopulator.get(); }
public:
    void PrepareTokensIFN(size_t n)
    {
        SG_ASSERT(n < (tokenCount >> 1));
        size_t nTokenized = (m_nextUnusedToken - m_nextToken) & tokenIndexMask;
        while(nTokenized < n)
        {
            Tokenize(*this);
            ++nTokenized;
        }
    }
    SG_FORCE_INLINE Token& GetToken_AssumeAvailable(size_t i)
    {
        SG_ASSERT(i < (tokenCount >> 1));
        SG_CODE_FOR_ASSERT(size_t const nTokenized = (m_nextUnusedToken - m_nextToken) & tokenIndexMask;)
        SG_ASSERT(i < nTokenized);
        Token& token = m_tokens[(m_nextToken + i) & tokenIndexMask];
        SG_ASSERT(TokenType::unused != token.type);
        return token;
    }
    Token& GetToken(size_t i)
    {
        PrepareTokensIFN(i+1);
        return GetToken_AssumeAvailable(i);
    }
    Token& EnqeueToken()
    {
        Token& token = m_tokens[m_nextUnusedToken];
        m_nextUnusedToken = (m_nextUnusedToken+1) % tokenCount;
        return token;
    }
    Token& EnqeueToken(char const *begin, char const* end, TokenType type)
    {
        Token& token = EnqeueToken();
        SG_ASSERT(TokenType::unused == token.type);
        token.begin = begin;
        token.end = end;
        token.type = type;
        m_cursor_tokenized = end;
        return token;
    }
    void ConsumeTokens(size_t n)
    {
        while(n > 0)
        {
#if SG_ENABLE_ASSERT
            if(0 == m_pushCount)
                m_tokens[m_nextToken].type = TokenType::unused;
#endif
            m_nextToken = (m_nextToken+1) & tokenIndexMask;
            --n;
        }
    }
    SG_FORCE_INLINE void NewLine(char const* lineBegin)
    {
        SG_ASSERT(lineBegin > m_line_begin);
        m_line++;
        m_line_begin = lineBegin;
    }
    void Error(ParsingErrorType type, Token const& token)
    {
        return PrivateError(type, &token, false);
    }
    void FatalError(ParsingErrorType type, Token const& token)
    {
        return PrivateError(type, &token, true);
    }
    void FatalError(ParsingErrorType type)
    {
        return PrivateError(type, nullptr, true);
    }
private:
    void PrivateError(ParsingErrorType type, Token const* token, bool isFatal)
    {
        if(maxErrorCount > m_errorCount)
        {
            ParsingError& error = m_errors[m_errorCount];
            if(nullptr != token)
            {
                error.col = token->col;
                error.line = token->line;
                error.begin = token->begin;
                error.end = token->end;
            }
            else
            {
                error.col = 0;
                error.line = all_ones;
                error.begin = m_all;
                error.end = m_all;
            }
            error.isFatal = isFatal;
            error.type = type;
            ++m_errorCount;
        }
        if(isFatal || maxErrorCount <= m_errorCount)
            m_fatalErrorHappened = true;
    }
};
//=============================================================================
//                              Tokenisation
//=============================================================================
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
            return 0;
        if(s == end)
            return -1;
    }
    return *(s-1)>*(sz-1) ? 1 : -1;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE TokenType IdentifyKeywordIFP(char const* begin, char const* end)
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
SG_FORCE_INLINE bool TryReadIdentifier(char const*& s)
{
    char const*const begin = s;
#define OPTIM_READ_IDENTIFIER 0
#if 0 == OPTIM_READ_IDENTIFIER
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
#elif 1 == OPTIM_READ_IDENTIFIER
    if(   (('a' <= *s) & (*s <= 'z'))
        | (('A' <= *s) & (*s <= 'Z'))
        | ('_' == *s) )
    {
        do
        {
            ++s;
        } while (     (('a' <= *s) & (*s <= 'z'))
                    | (('A' <= *s) & (*s <= 'Z'))
                    | ('_' == *s)
                    | (('0' <= *s) & (*s <= '9')) );
    }
#endif
#undef  OPTIM_READ_IDENTIFIER
    return begin != s;
}
SG_FORCE_INLINE TokenType TryReadNumber(char const*& s)
{
    char const*const begin = s;
    while('0' <= *s && *s <= '9')
    {
        ++s;
    }
    if(*s == '.')
    {
        s++;
        while('0' <= *s && *s <= '9')
        {
            ++s;
        }
        return begin != s ? TokenType::number : TokenType::unknown;
    }
    else
    {
        return begin != s ? TokenType::integer : TokenType::unknown;
    }
}
SG_FORCE_INLINE bool TryReadString(char const*& s)
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
SG_FORCE_INLINE bool TryReadComment(ParserState& state, char const*& s)
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
                    state.NewLine(s);
                }
                if(*s == '\n')
                {
                    ++s;
                    if(*s == '\r') ++s;
                    state.NewLine(s);
                }
                ++s;
            }
            if(*s == '\0')
                state.FatalError(ParsingErrorType::unexpected_end_of_file_in_comment_bloc);
            else
                s += 2;
        }
    }
    return begin != s;
}
SG_FORCE_INLINE bool TryReadSeparators(ParserState& state, char const*& s)
{
    char const*const begin = s;
    char const* begin_loop;
    do
    {
        begin_loop = s;
        while(*s == ' ') { ++s; }
        while(*s == '\t') { ++s; }
        while(*s == '\r')
        {
            ++s;
            if(*s == '\n') ++s;
            state.NewLine(s);
        }
        while(*s == '\n')
        {
            ++s;
            if(*s == '\r') ++s;
            state.NewLine(s);
        }
        if(TryReadComment(state, s))  { ++s; }
    } while(begin_loop != s);
    return begin != s;
}
void Tokenize(ParserState& state)
{
    char const* s = state.Cursor();
    TryReadSeparators(state, s);
    char const*const begin = s;
    if(TryReadIdentifier(s))
    {
        state.EnqeueToken(begin, s, IdentifyKeywordIFP(begin, s));
        return;
    }
    SG_ASSERT(begin == s);
    {
        TokenType numberTypeRead = TryReadNumber(s);
        if(TokenType::integer == numberTypeRead)
        {
            state.EnqeueToken(begin, s, TokenType::integer);
            return;
        }
        else if(TokenType::number == numberTypeRead)
        {
            state.EnqeueToken(begin, s, TokenType::number);
            return;
        }
        SG_ASSERT(TokenType::unknown == numberTypeRead);
    }
    SG_ASSERT(begin == s);
    if(TryReadString(s))
    {
        state.EnqeueToken(begin, s, TokenType::string);
        return;
    }
    SG_ASSERT(begin == s);
    if(*s == ':')
    {
        ++s;
        if(*s == ':') { ++s; state.EnqeueToken(begin, s, TokenType::operator_double_colon); return; }
        state.EnqeueToken(begin, s, TokenType::operator_colon);
        return;
    }
    if(*s == '{') { ++s; state.EnqeueToken(begin, s, TokenType::open_bloc); return; }
    if(*s == '}') { ++s; state.EnqeueToken(begin, s, TokenType::close_bloc); return; }
    if(*s == '[') { ++s; state.EnqeueToken(begin, s, TokenType::open_bracket); return; }
    if(*s == ']') { ++s; state.EnqeueToken(begin, s, TokenType::close_bracket); return; }
    if(*s == '(') { ++s; state.EnqeueToken(begin, s, TokenType::open_parenthesis); return; }
    if(*s == ')') { ++s; state.EnqeueToken(begin, s, TokenType::close_parenthesis); return; }
    if(*s == ',') { ++s; state.EnqeueToken(begin, s, TokenType::operator_comma); return; }
    if(*s == '?') { ++s; state.EnqeueToken(begin, s, TokenType::operator_interrogation); return; }
    if(*s == '.') { ++s; state.EnqeueToken(begin, s, TokenType::operator_dot); return; }
    if(*s == '=')
    {
        ++s;
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_equal_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_equal);
        return;
    }
    if(*s == '+')
    {
        ++s;
        if(*s == '+') { ++s; state.EnqeueToken(begin, s, TokenType::operator_plus_plus); return; }
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_plus_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_plus);
        return;
    }
    if(*s == '-')
    {
        ++s;
        if(*s == '-') { ++s; state.EnqeueToken(begin, s, TokenType::operator_minus_minus); return; }
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_minus_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_minus);
        return;
    }
    if(*s == '*')
    {
        ++s;
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_multiply_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_multiply);
        return;
    }
    if(*s == '/')
    {
        ++s;
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_divide_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_divide);
        return;
    }
    if(*s == '<')
    {
        ++s;
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_less_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_less);
        return;
    }
    if(*s == '>')
    {
        ++s;
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_greater_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_greater);
        return;
    }
    if(*s == '!')
    {
        ++s;
        if(*s == '=') { ++s; state.EnqeueToken(begin, s, TokenType::operator_not_equal); return; }
        state.EnqeueToken(begin, s, TokenType::operator_not);
        return;
    }
    if(*s == '\0') { state.EnqeueToken(begin, s, TokenType::eof); return; }
    ++s;
    Token& token = state.EnqeueToken(begin, s, TokenType::unknown);
    state.Error(ParsingErrorType::invalid_character_sequence, token);
}
void TokenizeIFP(ParserState& state, size_t nTokens)
{
    size_t nToTokenize = nTokens - state.TokenizedCount();
    while(nToTokenize > 0)
    {
        Tokenize(state);
        --nToTokenize;
    }
}
//=============================================================================
//                              Grammar
//=============================================================================
bool TryReadList1stLevelInstructions(ParserState& state);
bool TryRead1stLevelInstruction(ParserState& state);
bool TryReadInstruction(ParserState& state);
bool TryReadNamespaceDefinition(ParserState& state);
bool TryReadObjectDefinition(ParserState& state);
bool TryReadBloc(ParserState& state, bool isObjectBloc);
bool TryReadMemberAffectation(ParserState& state);
bool TryReadValue(ParserState& state);
bool TryReadValueList(ParserState& state);
bool TryReadPrimitiveValue(ParserState& state);
bool TryReadQualifiedIdentifier(ParserState& state);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TryReadList1stLevelInstructions(ParserState& state)
{
    while(state.CanContinue() && state.GetToken(0).type != TokenType::eof)
    {
        if(TryRead1stLevelInstruction(state))
            continue;
        else
        {
            // unexpected sequence
            state.Error(ParsingErrorType::unexpected_token, state.GetToken(0));
        }
    }
    if(state.GetErrorCount() != 0)
        return false;
    return true;
}
bool TryRead1stLevelInstruction(ParserState& state)
{
    if(TryReadInstruction(state))
        return true;
    if(TryReadNamespaceDefinition(state))
        return true;
    return false;
}
bool TryReadInstruction(ParserState& state)
{
    if(TryReadObjectDefinition(state))
        return true;
    return false;
}
bool TryReadNamespaceDefinition(ParserState& state)
{
    // namespace <name> { ... }
    state.PrepareTokensIFN(3);
    std::string name;
    {
        Token& token = state.GetToken_AssumeAvailable(0);
        if(token.type != TokenType::keyword_namespace)
            return false;
        state.ConsumeTokens(1);
    }
    {
        Token& token = state.GetToken_AssumeAvailable(0);
        if(token.type == TokenType::identifier)
        {
            name = std::string(token.begin, token.end-token.begin);
            state.ConsumeTokens(1);
        }
    }
    {
        Token& token = state.GetToken_AssumeAvailable(0);
        if(token.type != TokenType::open_bloc)
        {
            state.Error(ParsingErrorType::bloc_expected_in_namespace_declaration, token);
            return true;
        }
        state.ConsumeTokens(1);
    }

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();
    dbPopulator.BeginNamespace(name.c_str());

    while(state.CanContinue() && state.GetToken(0).type != TokenType::close_bloc)
    {
        if(TryRead1stLevelInstruction(state))
            continue;
        else
        {
            Token& token = state.GetToken(0);
            if(token.type == TokenType::eof)
            {
                state.Error(ParsingErrorType::unexpected_end_of_file_in_namespace_bloc, token);
            }
            else
            {
                state.Error(ParsingErrorType::unexpected_token, state.GetToken(0));
            }
        }
    }
    if(state.GetToken_AssumeAvailable(0).type == TokenType::close_bloc)
        state.ConsumeTokens(1);

    dbPopulator.EndNamespace();

    return true;
}
bool TryReadObjectDefinition(ParserState& state)
{
    // [exported/public/protected/private] <name> is <type> { ... }
    state.PrepareTokensIFN(5);
    reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Protected;
    std::string name;
    reflection::Identifier type(reflection::Identifier::Mode::Absolute);

    size_t i = 0;
    bool objectBlocIsExpected = false;
    {
        {
            Token& token = state.GetToken_AssumeAvailable(0);
            if(    token.type == TokenType::keyword_export
                || token.type == TokenType::keyword_public
                || token.type == TokenType::keyword_protected
                || token.type == TokenType::keyword_private )
            {
                using namespace reflection;
                switch(token.type)
                {
                case TokenType::keyword_export:    visibility = ObjectVisibility::Export;    break;
                case TokenType::keyword_public:    visibility = ObjectVisibility::Public;    break;
                case TokenType::keyword_protected: visibility = ObjectVisibility::Protected; break;
                case TokenType::keyword_private:   visibility = ObjectVisibility::Private;   break;
                default: SG_ASSERT_NOT_REACHED();
                }
                objectBlocIsExpected = true;
                state.ConsumeTokens(1);
            }
        }
        {
            // TODO: TryReadScriptEvalIdentifier(state);

            Token& token = state.GetToken_AssumeAvailable(i+1);
            if(token.type == TokenType::keyword_is)
            {
                Token& token_name = state.GetToken_AssumeAvailable(i);
                if(token_name.type != TokenType::identifier)
                {
                    state.Error(ParsingErrorType::unexpected_token_before_is, token);
                    state.ConsumeTokens(i);
                    return false;
                }
                name = std::string(token_name.begin, token_name.end - token_name.begin);
                SG_ASSERT(!name.empty());
                objectBlocIsExpected = true;
                i+=2;
            }
        }
        {
            bool qualifiedIdentifierFound = false;
            char const* prevTokenEnd = nullptr;
            {
                Token const& token = state.GetToken_AssumeAvailable(i);
                if(token.type == TokenType::identifier)
                {
                    type = reflection::Identifier(reflection::Identifier::Mode::Relative);
                    type.PushBack(std::string(token.begin, token.end - token.begin));
                    qualifiedIdentifierFound = true;
                    prevTokenEnd = token.end;
                    ++i;
                    state.PrepareTokensIFN(i+1);
                }
            }
            for(;;)
            {
                {
                    Token const& token = state.GetToken_AssumeAvailable(i);
                    if(token.type != TokenType::operator_double_colon)
                        break;
                    if(qualifiedIdentifierFound && token.begin != prevTokenEnd)
                        break;
                    prevTokenEnd = token.end;
                    state.PrepareTokensIFN(i+3);
                    Token const& nextToken = state.GetToken_AssumeAvailable(i+1);
                    if(nextToken.begin != prevTokenEnd)
                    {
                        state.Error(ParsingErrorType::unexpected_separator_after_double_colon, state.GetToken_AssumeAvailable(i));
                        break;
                    }
                    ++i;
                }
                {
                    Token const& token = state.GetToken_AssumeAvailable(i);
                    if(token.type != TokenType::identifier)
                    {
                        state.Error(ParsingErrorType::incorrect_use_of_double_colon, token);
                        break;
                    }
                    type.PushBack(std::string(token.begin, token.end - token.begin));
                    qualifiedIdentifierFound = true;
                    prevTokenEnd = token.end;
                    ++i;
                }
            }
            if(!qualifiedIdentifierFound)
            {
                if(!name.empty())
                {
                    state.Error(ParsingErrorType::type_expected_in_object_declaration, state.GetToken(i-1));
                    state.ConsumeTokens(i-1);
                }
                return false;
            }
            SG_ASSERT(!type.Empty());

            if(!objectBlocIsExpected)
            {
                Token& nextToken = state.GetToken_AssumeAvailable(i);
                if(nextToken.type != TokenType::open_bloc)
                {
                    return false;
                }
            }
        }
    }
    state.ConsumeTokens(i);

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();
    dbPopulator.BeginObject(visibility, name.c_str(), type);

    if(!TryReadBloc(state, true))
    {
        state.Error(ParsingErrorType::bloc_expected_in_object_declaration, state.GetToken(0));
    }

    dbPopulator.EndObject();

    return true;
}
bool TryReadBloc(ParserState& state, bool isObjectBloc)
{
    size_t i = 0;
    {
        Token& token = state.GetToken(i++);
        if(token.type != TokenType::open_bloc)
            return false;
    }
    state.ConsumeTokens(i);

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();
    if(!isObjectBloc) { dbPopulator.BeginBloc(); }

    i = 0;
    while(state.CanContinue())
    {
        Token& token = state.GetToken(0);
        if(token.type == TokenType::close_bloc)
        {
            state.ConsumeTokens(1);
            break;
        }
        if(token.type == TokenType::eof)
        {
            state.FatalError(ParsingErrorType::unexpected_end_of_file_in_object_bloc, token);
            if(!isObjectBloc) { dbPopulator.EndBloc(); }
            return false;
        }
        if(TryReadMemberAffectation(state))
            continue;

        state.Error(ParsingErrorType::unexpected_token_in_object_bloc, token);
        state.ConsumeTokens(1);
    }
    if(!isObjectBloc) { dbPopulator.EndBloc(); }
    return true;
}
bool TryReadMemberAffectation(ParserState& state)
{
    state.PrepareTokensIFN(2);
    std::string name;
    size_t i = 0;
    {
        Token& name_token = state.GetToken_AssumeAvailable(i++);
        {
            if(name_token.type != TokenType::identifier)
                return false;
        }
        {
            Token& token = state.GetToken_AssumeAvailable(i++);
            if(token.type != TokenType::operator_colon)
                return false;
        }
        name = std::string(name_token.begin, name_token.end - name_token.begin);
    }
    state.ConsumeTokens(i);

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();
    dbPopulator.Property(name.c_str());

    bool isValue = TryReadValue(state);
    if(!isValue)
    {
        state.Error(ParsingErrorType::value_expected_in_member_affectation, state.GetToken(0));
        return false;
    }
    return true;
}
bool TryReadValue(ParserState& state)
{
    if(TryReadObjectDefinition(state))
        return true;
    if(TryReadBloc(state, false))
        return true;
    if(TryReadQualifiedIdentifier(state))
        return true;
    if(TryReadValueList(state))
        return true;

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();

    Token& token = state.GetToken(0);
    if(token.type == TokenType::operator_minus)
    {
        Token& token1 = state.GetToken(1);
        if(token1.type == TokenType::integer)
        {
            unsigned long const value = strtoul(std::string(token1.begin, token1.end - token1.begin).c_str(), nullptr, 10);
            dbPopulator.Value(-num_cast<int>(value));
            state.ConsumeTokens(2);
            return true;
        }
        else if(token1.type == TokenType::number)
        {
            float value = (float)atof(std::string(token1.begin, token1.end - token1.begin).c_str());
            dbPopulator.Value(-value);
            state.ConsumeTokens(2);
            return true;
        }
        else
            return false;
    }
    if(token.type == TokenType::integer)
    {
        unsigned long const value = strtoul(std::string(token.begin, token.end - token.begin).c_str(), nullptr, 10);
        dbPopulator.Value(num_cast<int>(value));
        state.ConsumeTokens(1);
        return true;
    }
    if(token.type == TokenType::number)
    {
        float value = (float)atof(std::string(token.begin, token.end - token.begin).c_str());
        dbPopulator.Value(value);
        state.ConsumeTokens(1);
        return true;
    }
    if(token.type == TokenType::string)
    {
        SG_ASSERT('\"' == *token.begin);
        SG_ASSERT('\"' == *(token.end-1));
        dbPopulator.Value(std::string(token.begin+1, token.end - token.begin -2).c_str());
        state.ConsumeTokens(1);
        return true;
    }
    if(token.type == TokenType::keyword_false)
    {
        dbPopulator.Value(false);
        state.ConsumeTokens(1);
        return true;
    }
    if(token.type == TokenType::keyword_true)
    {
        dbPopulator.Value(true);
        state.ConsumeTokens(1);
        return true;
    }
    if(token.type == TokenType::keyword_null)
    {
        dbPopulator.Value(nullptr);
        state.ConsumeTokens(1);
        return true;
    }
    return false;
}
bool TryReadValueList(ParserState& state)
{
    state.PrepareTokensIFN(3);
    if(state.GetToken_AssumeAvailable(0).type != TokenType::open_bracket)
        return false;
    state.ConsumeTokens(1);

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();
    dbPopulator.BeginList();

    if(state.GetToken_AssumeAvailable(0).type == TokenType::close_bracket)
    {
        state.ConsumeTokens(1);
        dbPopulator.EndList();
        return true;
    }

    if(TryReadValue(state))
    {
        while(state.GetToken_AssumeAvailable(0).type != TokenType::close_bracket)
        {
            if(state.GetToken_AssumeAvailable(0).type != TokenType::operator_comma)
            {
                // error
                SG_ASSERT_NOT_REACHED();
                dbPopulator.EndList();
                return false;
            }
            state.ConsumeTokens(1);
            if(state.GetToken_AssumeAvailable(0).type == TokenType::close_bracket)
            {
                // trailing comma
                state.ConsumeTokens(1);
                dbPopulator.EndList();
                return true;
            }
            if(state.GetToken_AssumeAvailable(0).type == TokenType::eof)
            {
                state.FatalError(ParsingErrorType::unexpected_end_of_file_in_value_list, state.GetToken(0));
                dbPopulator.EndList();
                return true;
            }
            if(!TryReadValue(state))
            {
                // error
                SG_ASSERT_NOT_REACHED();
                dbPopulator.EndList();
                return false;
            }
            state.PrepareTokensIFN(2);
        }
    }
    if(state.GetToken_AssumeAvailable(0).type == TokenType::close_bracket)
    {
        state.ConsumeTokens(1);
        dbPopulator.EndList();
        return true;
    }
    if(state.GetToken_AssumeAvailable(0).type == TokenType::eof)
    {
        state.FatalError(ParsingErrorType::unexpected_end_of_file_in_value_list, state.GetToken(0));
        dbPopulator.EndList();
        return true;
    }
    state.Error(ParsingErrorType::unexpected_token_in_value_list, state.GetToken(0));
    dbPopulator.EndList();
    return true;
}
bool TryReadQualifiedIdentifier(ParserState& state)
{
    reflection::Identifier id(reflection::Identifier::Mode::Absolute);
    bool qualifiedIdentifierFound = false;
    char const* prevTokenEnd = nullptr;
    {
        Token const& token = state.GetToken(0);
        if(token.type == TokenType::identifier)
        {
            id = reflection::Identifier(reflection::Identifier::Mode::Relative);
            id.PushBack(std::string(token.begin, token.end - token.begin));
            state.ConsumeTokens(1);
            qualifiedIdentifierFound = true;
            prevTokenEnd = token.end;
        }
    }
    for(;;)
    {
        {
            Token const& token = state.GetToken(0);
            if(token.type != TokenType::operator_double_colon)
                break;
            if(qualifiedIdentifierFound && token.begin != prevTokenEnd)
                break;
            prevTokenEnd = token.end;
            Token const& nextToken = state.GetToken(1);
            if(nextToken.begin != prevTokenEnd)
            {
                state.Error(ParsingErrorType::unexpected_separator_after_double_colon, state.GetToken(0));
                break;
            }
            state.ConsumeTokens(1);
        }
        {
            Token const& token = state.GetToken(0);
            if(token.type != TokenType::identifier)
            {
                state.Error(ParsingErrorType::incorrect_use_of_double_colon, state.GetToken(0));
                break;
            }
            id.PushBack(std::string(token.begin, token.end - token.begin));
            state.ConsumeTokens(1);
            qualifiedIdentifierFound = true;
            prevTokenEnd = token.end;
        }
    }
    if(!qualifiedIdentifierFound)
        return false;

    reflection::ObjectDatabaseForwardPopulator& dbPopulator = *state.DatabasePopulator();
    dbPopulator.Value(id);
    return true;
}
//=============================================================================
}
//=============================================================================
bool ReadObjectScriptROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase /*, errorhandler ?*/)
{
    SimpleFileReader reader(iFile);
    char const* content = (char const*)reader.data();
    std::string str(content, reader.size());
    return ReadObjectScriptROK(str.c_str(), ioObjectDatabase);
}
bool ReadObjectScriptROK(char const* iFileContent, reflection::ObjectDatabase& ioObjectDatabase/*, errorhandler ?*/)
{
    ioObjectDatabase.BeginTransaction();
    reflection::ObjectDatabaseForwardPopulator databasePopulator(&ioObjectDatabase);
    ParserState state(iFileContent, &databasePopulator);
    bool ok = TryReadList1stLevelInstructions(state);
    ioObjectDatabase.EndTransaction();
    return ok;
}
//=============================================================================
}
}

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace simpleobjectscript {
//=============================================================================
namespace test {
namespace {
    char const* fileContent = ""
        "// a little comment"                   "\n"
        "objectC1"                              "\n"
        "is ::sg::reflectionTest::TestClass_C {""\n"
        "   u : 0"                              "\n"
        "   i : -1"                             "\n"
        "   f : 3.14"                           "\n"
        "   str : \"Hello world!\""             "\n"
        "   vectoru : [1,2,3]"                  "\n"
        "   structB : { u : 1 f : 2.51 }"       "\n"
        "   vectorStructB : ["                  "\n"
        "       { u : 1 f : 0.1 },"             "\n"
        "       { u : 2 f : 10 },"              "\n"
        "       { u : 3 f : -3.14 },"           "\n"
        "   ]"                                  "\n"
        "}"                                     "\n"
        "objectD1"                              "\n"
        "is sg::reflectionTest::TestClass_D {"  "\n"
        "   u : 1"                              "\n"
        "   object : ::objectC1"                "\n"
        "}"                                     "\n"
        "export objectD2"                       "\n"
        "is sg::reflectionTest::TestClass_D {"  "\n"
        "   u : 2"                              "\n"
        "   objectlist : ["                     "\n"
        "       ::objectC1,"                    "\n"
        "       objectD1,"                      "\n"
        //"       ::objectD2,"                    "\n" // leak
        "       myNamespace::objectD1,"         "\n"
        "       otherNamespace::objectD1,"      "\n"
        "       otherNamespace::objectD2,"      "\n"
        "   ]"                                  "\n"
        "}"                                     "\n"
        "namespace myNamespace {"               "\n"
        "    objectD1"                          "\n"
        "    is"                                "\n"
        "    sg::reflectionTest::TestClass_D {" "\n"
        "       u : 3"                          "\n"
        //"       object : objectD1"              "\n" // leak
        "    }"                                 "\n"
        "}"                                     "\n"
        "namespace otherNamespace {"            "\n"
        "    objectD1"                          "\n"
        "    is sg::reflectionTest::TestClass_D {"  "\n"
        "       u : 4"                          "\n"
        "       object : null"                  "\n"
        "    }"                                 "\n"
        "    objectD2"                          "\n"
        "    is"                                "\n"
        "    sg::reflectionTest::TestClass_D {" "\n"
        "       u : 5"                          "\n"
        "       object : objectD1"              "\n"
        "    }"                                 "\n"
        "}"                                     "\n"
        "";
}
//=============================================================================
namespace {
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
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg, simpleobjectscript), Reader, (ObjectScript, quick))
{
    using namespace test;

    CheckKeywords();

    reflection::Init();

    //for(size_t kk=0; kk<100; ++kk)
    {
        reflection::ObjectDatabase db;
        SIMPLE_CPU_PERF_LOG_SCOPE("Read Simple Object Script");
        bool ok = ReadObjectScriptROK(fileContent, db);
        SG_ASSERT_AND_UNUSED(ok);
    }

    reflection::Shutdown();
}
//=============================================================================
}
}
#endif

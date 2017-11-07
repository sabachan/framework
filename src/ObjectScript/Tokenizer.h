#ifndef ObjectScript_Tokenizer_H
#define ObjectScript_Tokenizer_H

#include <Core/SmartPtr.h>
#include "Reader.h"

namespace sg {
namespace objectscript {
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
    operator_plus, operator_minus, operator_multiply, operator_divide, operator_modulo,
    operator_plus_equal, operator_minus_equal, operator_multiply_equal, operator_divide_equal, operator_modulo_equal,
    operator_plus_plus, operator_minus_minus,
    operator_not,
    operator_bitand, operator_bitor, operator_bitxor,
    operator_bitand_equal, operator_bitor_equal, operator_bitxor_equal,
    operator_bitnot,
    operator_and, operator_or,
    operator_equal_equal, operator_not_equal,
    operator_less, operator_greater, operator_less_equal, operator_greater_equal,
    operator_shift_left, operator_shift_right,
    operator_shift_left_equal, operator_shift_right_equal,
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
    keyword_var,
    keyword_case,
    keyword_else,
    keyword_null,
    keyword_true,
    keyword_alias,
    keyword_break,
    keyword_const,
    keyword_false,
    keyword_while,
    keyword_assert,
    keyword_export,
    keyword_import,
    keyword_public,
    keyword_return,
    keyword_switch,
    keyword_private,
    keyword_typedef,
    keyword_continue,
    keyword_function,
    keyword_template,
    keyword_intrinsic,
    keyword_namespace,
    keyword_protected,
    end_keywords,
    eof,
};
//=============================================================================
struct Token
{
    TokenType type;
    char const* filebegin;
    char const* begin;
    char const* end;
    size_t fileid;
    size_t line;
    size_t col;
};
//=============================================================================
class Tokenizer
{
    SG_NON_COPYABLE(Tokenizer)
public:
    Tokenizer(char const* str, IErrorHandler* iErrorHandler);
    Tokenizer(Tokenizer&&) = default;
    void Tokenize(Token& oToken);
private:
    void TokenizePrivate();
    static TokenType IdentifyKeywordIFP(char const* begin, char const* end);
    static bool TryReadIdentifier(char const*& s);
    static TokenType TryReadNumber(char const*& s);
    bool TryReadString(char const*& s);
    bool TryReadComment(char const*& s);
    bool TryReadSeparators(char const*& s);
    void PushToken(char const *begin, char const* end, TokenType type);
    void PushError(ErrorType errorType, char const *begin, char const* end, char const* msg);
    void NewLine(char const* s);
private:
    char const*const m_all;
    char const* m_cursor_tokenized;
    size_t m_line;
    char const* m_line_begin;
    Token* m_outputToken;
    safeptr<IErrorHandler> m_errorHandler;
};
//=============================================================================
}
}
#endif

#ifndef ObjectScript_Reader_H
#define ObjectScript_Reader_H

#include "ImportDatabase.h"
#include <Core/ArrayList.h>
#include <Core/Config.h>
#include <Core/FilePath.h>
#include <Core/ArrayView.h>
#include <Core/SmartPtr.h>
#include <vector>

#define SG_OBJECTSCRIPT_TYPEDEF_IS_DEPRECATED 1

namespace sg {
namespace reflection {
class ObjectDatabase;
}
}

namespace sg {
namespace objectscript {
//=============================================================================
enum class ErrorType
{
    unknown,
    // File
    //----------
    cant_read_file,
    // Tokenizer
    //----------
    invalid_character_sequence,
    unexpected_end_of_file_in_comment_bloc,
    // Grammar Analyser
    //-----------------
    octal_format_is_forbidden,
    unexpected_end_of_file_value_expected,
    unexpected_end_of_file_in_bloc,
    unexpected_end_of_file_in_bracket,
    unexpected_end_of_file_in_parenthesis,
    unexpected_end_of_file_in_construct,
    unexpected_end_of_file_in_ternary_op,
    value_expected,
    if_empty_condition,
    if_invalid_condition,
    if_missing_parentheses,
    for_missing_parentheses,
    for_too_many_semicolons,
    for_empty_construct,
    for_invalid_construct,
    while_empty_condition,
    while_invalid_condition,
    while_missing_parentheses,
    alias_missing_is,
    expected_identifier_for_alias_name,
    expected_type_for_alias_type,
    integer_value_too_big,
    incorrect_use_of_keyword_else,
    incorrect_use_of_keyword_in,
    incorrect_use_of_semicolon,
    incorrect_use_of_function_call_on_template,
    // Semantic Tree
    //-----------------
    expression_is_not_an_instruction,
    unsupported_type_for_unary_operator,
    unsupported_types_for_binary_operator,
    unsupported_type_for_compound_assignment,
    unsupported_types_for_increment_operator,
    unsupported_type_for_intrinsic_name,
    unsupported_type_for_identifierize_operator,
    unexpected_type_for_condition,
    object_definition_forbidden_in_context,
    incorrect_type_for_ternary_operator_condition,
    call_to_uncallable_object,
    read_non_const_variable_outside_function_or_template,
    unknown_intrinsic_name,
    incorrect_use_of_intrinsic,
    missing_argument_in_intrinsic_call,
    missing_argument_in_function_call,
    object_definition_expects_an_object,
    incorrect_use_of_qualifier_for_variable_definition,
    divide_by_zero,
    assert_failed,
    use_of_class_as_value,
    assign_to_class,
    class_not_found,
    object_name_collision,
    object_name_collision_with_class,
    object_name_collision_with_script_object,
    namespace_name_collision_with_class,
    namespace_name_collision_with_script_object,
    function_name_collision_with_class,
    template_name_collision_with_class,
    unknown_reference_in_imported_file,
    unresolved_identifier_in_expression,
    variable_declaration_needs_constness_qualifier,
    non_const_variable_definition_in_imported_file,
    expression_is_not_callable,
    unexpected_use_of_jump_statement,
    missing_term_after_prefix_operator,
    // Warnings
    //-----------------
    typedef_is_deprecated,
};
struct Error
{
    ErrorType type;
    // TODO: begin and end are no more valid when read function returns. It
    // would be better to store a context.
    char const* filebegin;
    char const* begin;
    char const* end;
    size_t fileid;
    size_t line;
    size_t col;
    std::string msg;
};
//=============================================================================
class IErrorHandler : public SafeCountable
{
public:
    IErrorHandler() : m_errorHappened(false) {}
    virtual ~IErrorHandler() {}
    void OnObjectScriptError(Error const& iError) { m_errorHappened = true; VirtualOnObjectScriptError(iError); }
    void OnOpenFile(FilePath const& iFilepath) { VirtualOnOpenFile(iFilepath); }
    void OnCloseFile() { VirtualOnCloseFile(); }
    bool DidErrorHappen() { return m_errorHappened; }
    size_t GetCurrentFileId() const { return VirtualGetCurrentFileId(); }
protected:
    virtual void VirtualOnObjectScriptError(Error const& iError) = 0;
    virtual void VirtualOnOpenFile(FilePath const& iFilepath) = 0;
    virtual void VirtualOnCloseFile() = 0;
    virtual size_t VirtualGetCurrentFileId() const = 0;
private:
    bool m_errorHappened;
};
//=============================================================================
class ErrorHandler : public IErrorHandler
{
public:
    ArrayView<Error const> GetErrors() const { return ArrayView<Error const>(m_errors.data(), m_errors.size()); }
    std::string GetErrorMessage() const;
    ErrorHandler() { m_fileIndexStack.push_back(all_ones); }
    virtual ~ErrorHandler() override {}
private:
    virtual void VirtualOnObjectScriptError(Error const& iError) override;
    virtual void VirtualOnOpenFile(FilePath const& iFilepath) override { m_fileIndexStack.EmplaceBack(m_files.size()); m_files.EmplaceBack(iFilepath); }
    virtual void VirtualOnCloseFile() override { SG_ASSERT(!m_fileIndexStack.Empty()); m_fileIndexStack.PopBack(); };
    virtual size_t VirtualGetCurrentFileId() const { return m_fileIndexStack.back(); }
private:
    ArrayList<Error> m_errors;
    ArrayList<FilePath> m_files;
    ArrayList<size_t> m_fileIndexStack;
};
//=============================================================================
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler);
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, IErrorHandler& iErrorHandler);
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase);
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase);
//=============================================================================
bool ReadObjectScriptROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler);
bool ReadObjectScriptROK(char const* iFileContent, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler);
bool ReadObjectScriptROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, IErrorHandler& iErrorHandler);
bool ReadObjectScriptROK(char const* iFileContent, reflection::ObjectDatabase& ioObjectDatabase, IErrorHandler& iErrorHandler);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadImportROK(FilePath const& iFile, reflection::ObjectDatabase& ioScriptDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler);
bool ReadImportROK(char const* iFileContent, reflection::ObjectDatabase& ioScriptDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler);
//=============================================================================
}
}

#endif

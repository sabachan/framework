#include "stdafx.h"

#include "Reader.h"

#include "Tokenizer.h"
#include "GrammarAnalyser.h"
#include "SimpleReader.h"
#include "SemanticTree.h"
#include <Core/Assert.h>
#include <Core/FileSystem.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/SimpleFileReader.h>
#include <Core/StringFormat.h>
#include <Core/TestFramework.h>
#include <Core/WinUtils.h>
#include <Reflection/BaseClass.h>
#include <Reflection/InitShutdown.h>
#include <Reflection/ObjectDatabase.h>
#include <Reflection/PrimitiveData.h>
#include <sstream>

#if SG_ENABLE_TOOLS
#include "Writer.h"
#endif

// grammar description
// -------------------
//
// root : list_1st_level_instructions
// list_1st_level_instructions : 1st_level_instruction*
// 1st_level_instruction : instruction | namespace_definition | script_1st_level_instruction
// instruction : object_definition | script_instruction
// namespace_definition : 'namespace' script_eval_identifier '{' 1st_level_instruction* '}'
// object_definition : (object_visibility)? (script_eval_identifier 'is')? identifier object_bloc
// object_visibility: 'export' | 'public' | 'protected' | 'private'
// object_bloc : '{' (member_affectation | instruction)* '}'
// member_affectation : identifier ':' script_value
// value : primitive_value | qualified_identifier | object_definition | object_bloc | value_list
// value_list : '[' (value (',' value)* )? ']'
// qualified_identifier: '::'? script_eval_identifier ('::' script_eval_identifier)*
// identifier : (alpha alphanum*)
// alpha : [a-z,A-Z,_]
// alphanum : [a-z,A-Z,_,0-9]
// primitive_value : boolean | number | string
// boolean : 'true' | 'false'
// number : [0-9]+ ( '.' [0-9]+ )?
// string : '"' ( !'\' | ('\' any_char ) )* '"'
//
// script_1st_level_instruction : script_instruction | script_function_declaration | script_control_flow
// script_instruction : script_operation | script_variable_declaration | script_function_call | script_instruction_bloc | 'break' | 'continue'
// script_instruction_bloc : '{' script_instruction* '}'
// script_value : '(' script_value ')' | script_operation | script_function_call | value
// script_eval_identifier : identifier? ('$' (identifier | '('value')' ) )*
// script_variable_declaration : (object_visibility)? identifier '=' value
// script_function_forward_declaration : (object_visibility)? 'function' identifier '(' ( identifier (',' identifier)* )? ')'
// script_function_declaration : (object_visibility)? 'function' identifier '(' ( identifier (',' identifier)* )? ')' '{' instruction* '}'
// script_function_call : object_reference '(' ( value ( ',' value )* ) ? ')'
// script_operation : ( script_operator1 value ) | ( value script_operator2 value ) | ( value '?' value ':' value )
// script_control_flow : ( script_for | script_while | script_if ) instruction | ( '{' instruction* '}' ) )
// script_for : script_for_c | script_for_matlab
// script_for_c : 'for' '(' (script_operation)* ';' value ';' (script_operation) ')'
// script_for_in : 'for' '(' identifier 'in' value ')'
// script_for_matlab : 'for' '(' identifier '=' value ( '::' | (':' value ':') ) value ')'
// script_while : 'while' '(' value ')'
// script_if : 'if' '(' value ')'
// script_operator1 : '!' | '-' | '++' | '--' | '~' [...]
// script_operator2 : '=' | '+' | '-' | '*' | '/' | '<' | '>' | '+=' [...]

namespace sg {
namespace objectscript {
//=============================================================================
namespace {
std::string GetErrorLine(Error const& error)
{
    if(nullptr != error.begin)
    {
        SG_ASSERT(nullptr != error.end);
        char const* b = error.begin - error.col;
        char const* e = error.end;
        while(*e != '\n' && *e != '\r' && *e != '\0')
            ++e;
        return std::string(b,e);
    }
    else
    {
        return std::string();
    }
}
}
//=============================================================================
std::string ErrorHandler::GetErrorMessage() const
{
    std::ostringstream oss;
    size_t const errorCount = m_errors.size();
    for(size_t i = 0; i < errorCount; ++i)
    {
        Error error = m_errors[i];
        size_t const fileid = error.fileid;
        FilePath const& filepath = fileid == all_ones ? FilePath() : m_files[fileid];
        std::string const& filename = filepath.GetPrintableString();
        if(filename.empty())
            oss << "<Filename should be writen here>";
        else
            oss << filename;
        oss << "(" << error.line << ":" << error.col << "): ";
        oss << "error " << (size_t)error.type << ": ";
        if(filepath.Empty())
        {
            SG_ASSERT(error.fileid == all_ones);
            if(error.end > error.begin)
            {
                size_t const maxContextSize = 60;
                if(error.end < error.begin + maxContextSize)
                    oss << "\"" << std::string(error.begin, error.end) << "\"";
                else
                    oss << "\"" << std::string(error.begin, error.begin + maxContextSize - 4) << " ..." << "\"";
                oss << ": ";
            }
            oss << error.msg << std::endl;

            std::string const line = GetErrorLine(error);
            oss << line << std::endl;
            for(size_t j = 0; j < error.col; ++j)
                oss << " ";
            oss << "^" << std::endl;
        }
        else
        {
            SimpleFileReader file(filepath);
            if(file.IsValid())
            {
                char const* filedata = reinterpret_cast<char const*>(file.data());
                ptrdiff_t const offset = filedata - error.filebegin;
                error.filebegin += offset;
                error.begin += offset;
                error.end += offset;
                if(error.end > error.begin)
                {
                    size_t const maxContextSize = 60;
                    if(error.end < error.begin + maxContextSize)
                        oss << "\"" << std::string(error.begin, error.end) << "\"";
                    else
                        oss << "\"" << std::string(error.begin, error.begin + maxContextSize - 4) << " ..." << "\"";
                    oss << ": ";
                }
                oss << error.msg << std::endl;
                std::string const& line = GetErrorLine(error);
                oss << line << std::endl;
                for(size_t j = 0; j < error.col; ++j)
                    oss << " ";
                oss << "^" << std::endl;
            }
            else
            {
                oss << error.msg << std::endl;
            }
        }
    }
    return oss.str();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ErrorHandler::VirtualOnObjectScriptError(Error const& iError)
{
    SG_ASSERT(iError.fileid < m_files.size() || iError.fileid == all_ones);
    m_errors.EmplaceBack(iError);
}
//=============================================================================
namespace {
bool ReadObjectScriptWithRetryImplROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase* ioImportDatabase, IErrorHandler* iErrorHandler)
{
    SG_SIMPLE_CPU_PERF_LOG_SCOPE("ReadObjectScript");
    bool ok;
    bool retry;
#if SG_ENABLE_TOOLS
    size_t nRetry = 0;
#endif
    do
    {
        retry = false;

        ImportDatabase importDatabase;
        if(nullptr != ioImportDatabase)
            importDatabase = *ioImportDatabase;
        ErrorHandler errorHandler;
        ok = ReadObjectScriptROK(iFile, ioObjectDatabase, importDatabase, errorHandler);
#if SG_ENABLE_TOOLS
        if(!ok)
        {
            SG_ASSERT(!errorHandler.GetErrors().Empty());
            if(errorHandler.GetErrors()[0].type == ErrorType::cant_read_file && 5 > ++nRetry)
            {
                retry = true;
            }
            else
            {
                std::string const errstr = errorHandler.GetErrorMessage();
                SG_ASSERT(!errstr.empty());
                retry = winutils::ShowModalErrorReturnRetry("Object script failed!", errstr.c_str());
                nRetry = 0;
            }
        }
#endif
        if(!retry)
        {
            if(nullptr != ioImportDatabase)
                *ioImportDatabase = importDatabase;
            if(nullptr != iErrorHandler)
            {
                for(Error const& e : errorHandler.GetErrors())
                    iErrorHandler->OnObjectScriptError(e);
            }
        }
    } while(retry);

    return ok;
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler)
{
    return ReadObjectScriptWithRetryImplROK(iFile, ioObjectDatabase, &ioImportDatabase, &iErrorHandler);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, IErrorHandler& iErrorHandler)
{
    return ReadObjectScriptWithRetryImplROK(iFile, ioObjectDatabase, nullptr, &iErrorHandler);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase)
{
    return ReadObjectScriptWithRetryImplROK(iFile, ioObjectDatabase, &ioImportDatabase, nullptr);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptWithRetryROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase)
{
    return ReadObjectScriptWithRetryImplROK(iFile, ioObjectDatabase, nullptr, nullptr);
}
//=============================================================================
namespace {
void PushFileError(FilePath const& iFile, IErrorHandler& iErrorHandler)
{
    Error error;
    error.filebegin = nullptr;
    error.begin = nullptr;
    error.end = nullptr;
    error.fileid = iErrorHandler.GetCurrentFileId();
    error.col = 0;
    error.line = 0;
    error.type = ErrorType::cant_read_file;
    std::ostringstream oss;
    oss << "Can't read file: " << iFile.GetPrintableString();
    error.msg = oss.str();
    iErrorHandler.OnObjectScriptError(error);
}
}
//=============================================================================
bool ReadObjectScriptROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler)
{
    std::string const dir = iFile.ParentDirectory().GetSystemFilePath();
    filesystem::PushWorkingDir(dir);

    iErrorHandler.OnOpenFile(iFile);

    SimpleFileReader reader(iFile);
    if(!reader.IsValid())
    {
        PushFileError(iFile, iErrorHandler);
        iErrorHandler.OnCloseFile();
        return false;
    }
    char const* content = (char const*)reader.data();
    std::string const str(content, reader.size());
    bool const rok = ReadObjectScriptROK(str.c_str(), ioObjectDatabase, ioImportDatabase, iErrorHandler);

    iErrorHandler.OnCloseFile();

    filesystem::PopWorkingDir();
    return rok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptROK(char const* iFileContent, reflection::ObjectDatabase& ioObjectDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler)
{
    Imported imported;
    GrammarAnalyser grammarAnalyser(iFileContent, &iErrorHandler);
    refptr<semanticTree::Root> root = grammarAnalyser.Run();
    if(iErrorHandler.DidErrorHappen())
        return false;
    SG_ASSERT(nullptr != root);
    semanticTree::EvaluationInputOutput evaluationio(&ioObjectDatabase, nullptr, &ioImportDatabase, &imported, &iErrorHandler);
    refptr<reflection::IPrimitiveData> returnValue;
    bool ok = root->EvaluateScriptROK(evaluationio);
    if(!ok)
    {
        SG_ASSERT(iErrorHandler.DidErrorHappen());
        return false;
    }
    SG_ASSERT(nullptr == evaluationio.returnValue);
    return !iErrorHandler.DidErrorHappen();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase, IErrorHandler& iErrorHandler)
{
    ImportDatabase importDatabase;
    bool const rok = ReadObjectScriptROK(iFile, ioObjectDatabase, importDatabase, iErrorHandler);
    return rok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadObjectScriptROK(char const* iFileContent, reflection::ObjectDatabase& ioObjectDatabase, IErrorHandler& iErrorHandler)
{
    ImportDatabase importDatabase;
    bool const rok = ReadObjectScriptROK(iFileContent, ioObjectDatabase, importDatabase, iErrorHandler);
    return rok;
}
//=============================================================================
bool ReadImportROK(FilePath const& iFile, reflection::ObjectDatabase& ioScriptDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler)
{
    std::string const dir = iFile.ParentDirectory().GetSystemFilePath();
    filesystem::PushWorkingDir(dir);

    iErrorHandler.OnOpenFile(iFile);

    SimpleFileReader reader(iFile);
    if(!reader.IsValid())
    {
        PushFileError(iFile, iErrorHandler);
        return false;
    }
    char const* content = (char const*)reader.data();
    std::string const str(content, reader.size());
    bool const rok = ReadImportROK(str.c_str(), ioScriptDatabase, ioImportDatabase, iErrorHandler);

    iErrorHandler.OnCloseFile();

    filesystem::PopWorkingDir();
    return rok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadImportROK(char const* iFileContent, reflection::ObjectDatabase& ioScriptDatabase, ImportDatabase& ioImportDatabase, IErrorHandler& iErrorHandler)
{
    Imported imported;
    GrammarAnalyser grammarAnalyser(iFileContent, &iErrorHandler);
    refptr<semanticTree::Root> root = grammarAnalyser.Run();
    if(iErrorHandler.DidErrorHappen())
        return false;
    SG_ASSERT(nullptr != root);
    semanticTree::EvaluationInputOutput evaluationio(nullptr, &ioScriptDatabase, &ioImportDatabase, &imported, &iErrorHandler);
    refptr<reflection::IPrimitiveData> returnValue;
    bool ok = root->EvaluateImportROK(evaluationio);
    if(!ok)
    {
        SG_ASSERT(iErrorHandler.DidErrorHappen());
        return false;
    }
    SG_ASSERT(nullptr == evaluationio.returnValue);
    return !iErrorHandler.DidErrorHappen();
}
//=============================================================================
}
}

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace objectscript {
//=============================================================================
namespace test {
namespace {
    char const* devFileContent = ""
        "// a little comment"                                               "\n"
        "function g(a, b, c, x)"                                            "\n"
        "namespace math {"                                                  "\n"
        "   const pi = 3.14159265359"                                       "\n"
        "}"                                                                 "\n"
        "var i = 0"                                                         "\n"
        "var tab = [10, 20, 30]"                                            "\n"
        "tab[0] = 11"                                                       "\n"
        "tab += [40]"                                                       "\n"
        "function f(x)"                                                     "\n"
        "{"                                                                 "\n"
        "   const a = 1"                                                    "\n"
        "   const b = -1"                                                   "\n"
        "   const c = 3"                                                    "\n"
        "   return x == 0 ? 0 : g(a, b, c, x)"                              "\n"
        "}"                                                                 "\n"
        "function g(a, b, c, x)"                                            "\n"
        "{"                                                                 "\n"
        "   return a * x*x + b * x + c"                                     "\n"
        "}"                                                                 "\n"
        "private objectC1"                                                  "\n"
        "is ::sg::reflectionTest::TestClass_C {"                            "\n"
        "   u : i"                                                          "\n"
        "   i : -1 - 2 * (3 + 1)"                                           "\n"
        "   f : ["                                                          "\n"
        "           [1.5, 2.3],"                                            "\n"
        "           [math::pi],"                                            "\n"
        "           [2.1, 3.2, 4.3]"                                        "\n"
        "       ] [6-2*3] [1]"                                              "\n"
        "   str : \"\" + true + \"_\" + 24"                                 "\n"
        "   vectoru : [1,2] + [3]"                                          "\n"
        "       + [0xDeadBeef]"                                             "\n"
        "       + (i==0 ? [4] + [5] : [])"                                  "\n"
        "       + tab"                                                      "\n"
        "       + [f(2), f(3)]"                                             "\n"
        "   structB : { u : 1 f : 2.51 }"                                   "\n"
        "   vectorStructB : ["                                              "\n"
        "       { u : 1 f : .1 },"                                          "\n"
        "       { u : 2 f : 10 },"                                          "\n"
        "       { u : 3 f : -3.14 },"                                       "\n"
        "   ]"                                                              "\n"
        "}"                                                                 "\n"
        "i = i + 2"                                                         "\n"
        "assert i==2"                                                       "\n"
        "tab = [4, 6]"                                                      "\n"
        "for(j in tab)"                                                     "\n"
        "   i += j"                                                         "\n"
        "assert i==12, \"i should be 12\""                                  "\n"
        "for(var j = 0; j<4; j++)"                                          "\n"
        "{"                                                                 "\n"
        "   i +=2"                                                          "\n"
        "}"                                                                 "\n"
        "if(i == 20)"                                                       "\n"
        "{"                                                                 "\n"
        "   objectD1"                                                       "\n"
        "   is sg::reflectionTest::TestClass_D {"                           "\n"
        "      u : ++i"                                                     "\n"
        "      object : ::objectC1"                                         "\n"
        "   }"                                                              "\n"
        "}"                                                                 "\n"
        "else if(i == 21)"                                                  "\n"
        "{"                                                                 "\n"
        "   objectD1"                                                       "\n"
        "   is sg::reflectionTest::TestClass_A {"                           "\n"
        "      u : ++i"                                                     "\n"
        "   }"                                                              "\n"
        "}"                                                                 "\n"
        "else"                                                              "\n"
        "{"                                                                 "\n"
        "   objectD1"                                                       "\n"
        "   is sg::reflectionTest::TestClass_C {"                           "\n"
        "      u : ++i"                                                     "\n"
        "   }"                                                              "\n"
        "}"                                                                 "\n"
        "i += 1"                                                            "\n"
        "var j = 0"                                                         "\n"
        "while(j++ < 10)"                                                   "\n"
        "   i++"                                                            "\n"
        "namespace myNamespace {"                                           "\n"
        "    objectD1"                                                      "\n"
        "    is sg::reflectionTest::TestClass_D {"                          "\n"
        "       u : i"                                                      "\n"
        "       object :"                                                   "\n"
        "      sg::reflectionTest::TestClass_C {"                           "\n"
        "           const k = ++i"                                          "\n"
        "           u : k"                                                  "\n"
        "       }"                                                          "\n"
        "    }"                                                             "\n"
        "   template tplObj("                                               "\n"
        "       x,"                                                         "\n"
        "       y = 1"                                                      "\n"
        "   )"                                                              "\n"
        "   is sg::reflectionTest::TestClass_C {"                           "\n"
        "       u : x"                                                      "\n"
        "       i : y"                                                      "\n"
        "       f : 1.*x/y"                                                 "\n"
        "   }"                                                              "\n"
        "}"                                                                 "\n"
        "objectTemplate1 is"                                                "\n"
        "myNamespace::tplObj {"                                             "\n"
        "    x : 3"                                                         "\n"
        "    y : 2"                                                         "\n"
        "}"                                                                 "\n"
        // todo
        //      structure member reference (.)
        //      pragmas
        //      error management
        "";
    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
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
    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    struct ObjectScriptTest { char const* description; char const* fileContent; char const* equivalentFile; bool returnValue; ErrorType firstError; };
    ObjectScriptTest const objectScriptTests[] = {
        { "empty file",         "",                                 "", true, ErrorType::unknown },
#define ObjectScript_Reader_Include_TestImpl
#include "Reader_TestImpl.h"
#include "Reader_FunctionTestImpl.h"
#include "Reader_TemplateTestImpl.h"
#undef ObjectScript_Reader_Include_TestImpl
    };
    size_t const objectScriptTestCount = SG_ARRAYSIZE(objectScriptTests);
    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}
//=============================================================================
SG_TEST((sg, objectscript), Reader, (ObjectScript, quick))
{
    using namespace test;

    reflection::Init();

//#define RUN_ONLY_TEST_NUMBER 144

#ifdef RUN_ONLY_TEST_NUMBER
    size_t const i = RUN_ONLY_TEST_NUMBER;
#else
    for(size_t i=0; i<objectScriptTestCount; ++i)
#endif
    {
        ObjectScriptTest const& test = objectScriptTests[i];
        {
            std::ostringstream oss;
            oss << "Testing " << test.description << " (" << i << ")";
            SG_LOG_INFO("Test", oss.str().c_str());
        }
        ErrorHandler errorHandler;
        reflection::ObjectDatabase db;
        bool const ok = ReadObjectScriptROK(test.fileContent, db, errorHandler);
        SG_LOG_DEBUG("Test", errorHandler.GetErrorMessage().c_str());
        SG_ASSERT(ok == test.returnValue);
        if(!ok)
        {
            std::string const errmsg = errorHandler.GetErrorMessage();
            ArrayView<Error const> errors = errorHandler.GetErrors();
            SG_ASSERT(!errors.empty());
            SG_ASSERT(errors[0].type == test.firstError);
        }
        std::string str;
        WriteObjectScript(str, db);

        ErrorHandler errorHandler_ref;
        reflection::ObjectDatabase db_ref;
        bool const ok_ref = ReadObjectScriptROK(test.equivalentFile, db_ref, errorHandler_ref);
        SG_ASSERT_AND_UNUSED(ok_ref);
        std::string str_ref;
        WriteObjectScript(str_ref, db_ref);
        SG_ASSERT(str == str_ref);
    }

    //for(size_t kk=0; kk<50000; ++kk)
    {
        ErrorHandler errorHandler;
        reflection::ObjectDatabase db;
        {
            SG_SIMPLE_CPU_PERF_LOG_SCOPE("Read Object Script");
            //bool ok = ReadObjectScriptROK(fileContent, db, errorHandler);
            bool const ok = ReadObjectScriptROK(devFileContent, db, errorHandler);
            SG_ASSERT(!errorHandler.DidErrorHappen() || !ok);
            SG_ASSERT_AND_UNUSED(ok);
        }
        SG_LOG_DEBUG("Test", errorHandler.GetErrorMessage().c_str());
        std::string str;
        WriteObjectScript(str, db);
        //for(size_t kk=0; kk<200000; ++kk)
        {
            ErrorHandler errorHandler_2;
            reflection::ObjectDatabase db_2;
            {
                SG_SIMPLE_CPU_PERF_LOG_SCOPE("Read Object Script 2");
                bool const ok_2 = ReadObjectScriptROK(str.c_str(), db_2, errorHandler_2);
                SG_ASSERT(!errorHandler_2.DidErrorHappen() || !ok_2);
                SG_ASSERT_AND_UNUSED(ok_2);
            }
            SG_LOG_DEBUG("Test", errorHandler_2.GetErrorMessage().c_str());
            std::string str_2;
            WriteObjectScript(str_2, db_2);
            SG_ASSERT(str == str_2);
        }
        {
            reflection::ObjectDatabase db_3;
            {
                SG_SIMPLE_CPU_PERF_LOG_SCOPE("Read Simple Object Script");
                bool const ok_3 = simpleobjectscript::ReadObjectScriptROK(str.c_str(), db_3);
                SG_ASSERT_AND_UNUSED(ok_3);
            }
            std::string str_3;
            WriteObjectScript(str_3, db_3);
            SG_ASSERT(str == str_3);
        }
    }

    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();

    {
        ErrorHandler errorHandler;
        reflection::ObjectDatabase db;
        {
            SG_SIMPLE_CPU_PERF_LOG_SCOPE("Read Object Script");
            bool const ok = ReadObjectScriptROK(FilePath("src:/ObjectScript/UnitTests/Test.os"), db, errorHandler);
            SG_ASSERT(!errorHandler.DidErrorHappen() || !ok);
            SG_ASSERT_AND_UNUSED(ok);
        }
        std::string str;
        WriteObjectScript(str, db);
        char const* equivalentFileContent = "";
        ErrorHandler errorHandler_ref;
        reflection::ObjectDatabase db_ref;
        bool const ok_ref = ReadObjectScriptROK(equivalentFileContent, db_ref, errorHandler_ref);
        SG_ASSERT_AND_UNUSED(ok_ref);
        std::string str_ref;
        WriteObjectScript(str_ref, db_ref);
        //SG_ASSERT(str == str_ref);
    }

    char const* filenames_errors[] = {
        "src:/ObjectScript/UnitTests/Errors/Test_Errors.os"
    };
    for(char const* filename : AsArrayView(filenames_errors))
    {
        SG_LOG_INFO("Test", Format("Testing file: %0", filename));
        ErrorHandler errorHandler;
        reflection::ObjectDatabase db;
        {
            bool const ok = ReadObjectScriptROK(FilePath(filename), db, errorHandler);
            SG_ASSERT(errorHandler.DidErrorHappen() && !ok);
        }
        SG_LOG_DEBUG("Test", errorHandler.GetErrorMessage().c_str());
    }

    filesystem::Shutdown();
    reflection::Shutdown();
}
//=============================================================================
}
}
#endif

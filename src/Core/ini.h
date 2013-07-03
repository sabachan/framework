#ifndef Core_ini_H
#define Core_ini_H

#include "IntTypes.h"
#include "FilePath.h"
#include <vector>

namespace sg {
namespace ini {
//=============================================================================
struct StringRef
{
    char const* begin;
    char const* end;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct Value
{
    StringRef srcString;
    StringRef const* asString;
    i64 const* asInt;
    float const* asFloat;
    bool const* asBool;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class IEventHandler
{
public:
    virtual bool VirtualOnEntryROK(StringRef iSection, StringRef iName, Value const& iValue) = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
enum class ErrorType {
    UnclosedSection
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct Error
{
    ErrorType type;
    size_t line;
    size_t col;
    char const* msg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class IErrorHandler
{
public:
    virtual void VirtualOnError(Error const& iError) = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ReadROK(char const* iData, IEventHandler* iEventHandler, IErrorHandler* iErrorHandler = nullptr);
bool ReadROK(FilePath const& iFilePath, IEventHandler* iEventHandler, IErrorHandler* iErrorHandler = nullptr);
//=============================================================================
}
}

#endif

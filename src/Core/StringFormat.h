#ifndef Core_StringFormat_H
#define Core_StringFormat_H

#include "Assert.h"
#include "Config.h"
#include "StringUtils.h"
#include <string>

namespace sg {
//=============================================================================
namespace stringformatimpl {
class IFormatArg
{
public:
    virtual void InsertIn(std::ostream& iStream) const = 0;
    SG_CODE_FOR_ASSERT(virtual bool IsOptional() const { return false; });
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline std::ostream& operator<< (std::ostream& os, IFormatArg const& iFormatArg)
{
    iFormatArg.InsertIn(os);
    return os;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class FormatArg : public IFormatArg
{
    SG_NO_COPY_OPERATOR(FormatArg)
public:
    explicit FormatArg(T const& iArg) : arg(iArg) {}
    FormatArg(FormatArg const&) = default;
    virtual void InsertIn(std::ostream& iStream) const override { iStream << arg; }
private:
private:
    T const& arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <>
class FormatArg<std::wstring> : public IFormatArg
{
    SG_NO_COPY_OPERATOR(FormatArg)
public:
    explicit FormatArg(std::wstring const& iArg) : arg(iArg) {}
    FormatArg(FormatArg const&) = default;
    virtual void InsertIn(std::ostream& iStream) const override { iStream << ConvertUCS2ToUTF8(arg); }
private:
private:
    std::wstring const& arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class FormatOptionalArg
{
    SG_NO_COPY_OPERATOR(FormatOptionalArg)
public:
    explicit FormatOptionalArg(T const& iArg) : arg(iArg) {}
    FormatOptionalArg(FormatOptionalArg const&) = default;
private:
    friend class FormatArg<FormatOptionalArg<T> >;
    T const& arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class FormatArg<FormatOptionalArg<T> > : public IFormatArg
{
    SG_NO_COPY_OPERATOR(FormatArg)
public:
    explicit FormatArg(FormatOptionalArg<T> const& iArg) : arg(iArg.arg) {}
    FormatArg(FormatArg const&) = default;
    virtual void InsertIn(std::ostream& iStream) const override { iStream << arg; }
    SG_CODE_FOR_ASSERT(virtual bool IsOptional() const override { return true; });
private:
    T const& arg;
};
//=============================================================================
std::string Format(char const* iStr, IFormatArg const*const* formatArgs, size_t argCount);
//=============================================================================
template <typename F, typename... T> std::string Format(char const* iStr, IFormatArg** formatArgs, size_t argCount, F iFirstArg, T... iArgs)
{
    FormatArg<F> formatArg(iFirstArg);
    size_t const remainingArgCount = sizeof...(iArgs);
    size_t const index = argCount - remainingArgCount - 1;
    SG_ASSERT(nullptr == formatArgs[index]);
    formatArgs[index] = &formatArg;
    return Format(iStr, formatArgs, argCount, iArgs...);
}
} // stringformat
//=============================================================================
inline std::string Format(char const* iStr)
{
    return std::string(iStr);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename... T> std::string Format(char const* iStr, T... iArgs)
{
    size_t const argCount = sizeof...(iArgs);
    stringformatimpl::IFormatArg* formatArgs[argCount] = {};
    return stringformatimpl::Format(iStr, formatArgs, argCount, iArgs...);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename... T> std::string Format(std::string const& iStr, T... iArgs)
{
    return Format(iStr.c_str(), iArgs...);
}
//=============================================================================
template <typename T>
stringformatimpl::FormatOptionalArg<T> FormatOptional(T const& iArg) { return stringformatimpl::FormatOptionalArg<T>(iArg); }
//=============================================================================
}

#endif
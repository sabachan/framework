#ifndef Reflection_PrimitiveData_H
#define Reflection_PrimitiveData_H

#include <Core/Cast.h>
#include <Core/IntTypes.h>
#include <Core/Preprocessor.h>
#include <Core/SmartPtr.h>
#include <vector>
#include "Identifier.h"

namespace sg {
namespace reflection {
//=============================================================================
class BaseClass;
class IPrimitiveData;
//=============================================================================
typedef std::vector<refptr<IPrimitiveData> > PrimitiveDataList;
typedef std::vector<std::pair<std::string, refptr<IPrimitiveData> > > PrimitiveDataNamedList;
//=============================================================================
// MACRO( NAME,             CPP_TYPE,                   IS_NUMERIC,     IS_INTEGER)
#define SG_REFLECTION_APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(MACRO) \
    MACRO(Null,             nullptr_t,                  false,          false) \
    MACRO(Boolean,          bool,                       false,          false) \
    MACRO(Int32,            i32,                         true,           true) \
    MACRO(UInt32,           u32,                         true,           true) \
    MACRO(Float,            float,                       true,          false) \
    MACRO(String,           std::string,                false,          false) \
    MACRO(List,             PrimitiveDataList,          false,          false) \
    MACRO(NamedList,        PrimitiveDataNamedList,     false,          false) \
    MACRO(Object,           refptr<BaseClass>,          false,          false) \
    MACRO(ObjectReference,  ObjectReference,            false,          false) \
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
enum class PrimitiveDataType
{
    Unknown,
#define DECLARE_ENUM_VALUE(NAME, CPP_TYPE, IS_NUMERIC, IS_INTEGER) NAME,
    SG_REFLECTION_APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(DECLARE_ENUM_VALUE)
#undef DECLARE_ENUM_VALUE
};
//=============================================================================
struct PrimitiveDataTypeInfo
{
    PrimitiveDataType type;
    char const* name;
    bool isNumeric;
    bool isInteger;
};
PrimitiveDataTypeInfo const& GetInfo(PrimitiveDataType type);
//=============================================================================
inline char const* GetPrimitiveDataTypeName(PrimitiveDataType type)
{
    return GetInfo(type).name;
}
//=============================================================================
class IPrimitiveData : public RefAndSafeCountable
{
    SG_NON_COPYABLE(IPrimitiveData)
public:
    IPrimitiveData() = default;
    virtual ~IPrimitiveData() {}
    virtual PrimitiveDataType GetType() const = 0;
    virtual bool IsDefault() const = 0;
    template<typename U> bool AsROK(U* oValue) const;
    template<typename U> void As(U* oValue) const;
    template<typename U> U As() const { U r; As<U>(&r); return r;}
    void CopyTo(refptr<IPrimitiveData>& oCopy) const;
};
//=============================================================================
template<typename T> struct PrimitiveDataTraits { static const bool is_supported_type = false; };
#define DECLARE_PRIMITIVE_DATA_TRAITS(NAME, CPP_TYPE, IS_NUMERIC, IS_INTEGER) \
    template<> struct PrimitiveDataTraits<STRIP_PARENTHESIS(CPP_TYPE)> \
    { \
        static const bool is_supported_type = true; \
        static const bool is_numeric = IS_NUMERIC; \
        static const bool is_integer = IS_INTEGER; \
        typedef STRIP_PARENTHESIS(CPP_TYPE) cpp_type; \
        static const PrimitiveDataType primitive_data_type = PrimitiveDataType::NAME; \
    };
    SG_REFLECTION_APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(DECLARE_PRIMITIVE_DATA_TRAITS)
#undef DECLARE_PRIMITIVE_DATA_TRAITS
//=============================================================================
template<typename T>
class PrimitiveData : public IPrimitiveData
{
public:
    PrimitiveData() : m_value() { }
    explicit PrimitiveData(T const & iValue) : m_value(iValue) { }
    virtual PrimitiveDataType GetType() const override { return PrimitiveDataTraits<T>::primitive_data_type; }
    virtual bool IsDefault() const override { return T() == m_value; }
    T const& Get() const { return m_value; }
    T& GetForWriting() { return m_value; }
    template<typename U> bool AsROK(U* oValue) const;
    template<typename U> void As(U* oValue) const;
    template<typename U> U As() const { U r; As<U>(r); return r;}
private:
    T m_value;
};
//=============================================================================
template<typename U> void IPrimitiveData::As(U* oValue) const
{
    bool ok = AsROK(oValue);
    SG_ASSERT_MSG_AND_UNUSED(ok, "Conversion not supported");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename U> bool IPrimitiveData::AsROK(U* oValue) const
{
    SG_ASSERT(nullptr != oValue);
    switch(GetType())
    {
#define CASE_CAST_AND_CONVERT(NAME, CPP_TYPE, IS_NUMERIC, IS_INTEGER) \
    case PrimitiveDataType::NAME: \
        { \
            PrimitiveData<STRIP_PARENTHESIS(CPP_TYPE)> const* data =  \
                checked_cast<PrimitiveData<STRIP_PARENTHESIS(CPP_TYPE)> const*>(this); \
            return data->AsROK(oValue); \
        } \
        break;
    SG_REFLECTION_APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(CASE_CAST_AND_CONVERT)
#undef CASE_CAST_AND_CONVERT
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return false;
}
//=============================================================================
template<typename T, typename U>
struct PrimitiveDataConversion
{
    static const bool is_supported = false;
    inline bool operator() (T* oValue, U const& iValue) { SG_UNUSED((oValue, iValue)); SG_ASSERT(nullptr != oValue); return is_supported; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
struct PrimitiveDataConversion<T,T>
{
    static const bool is_supported = true;
    inline bool operator() (T* oValue, T const& iValue) { SG_ASSERT(nullptr != oValue); *oValue = iValue ; return is_supported; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CTOR(DST, SRC) \
    template<> \
    struct PrimitiveDataConversion<DST,SRC> \
    { \
        static const bool is_supported = true; \
        inline bool operator() (DST* oValue, SRC const& iValue) \
        { \
            SG_ASSERT(nullptr != oValue); \
            *oValue = DST(iValue); \
            return is_supported; \
        } \
    };
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CTOR(float, i32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CTOR(float, u32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CTOR(i32, u32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CTOR(u32, i32)
#undef DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CTOR
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<>
struct PrimitiveDataConversion<std::string, nullptr_t>
{
    static const bool is_supported = true;
    inline bool operator() (std::string* oValue, nullptr_t)
    {
        SG_ASSERT(nullptr != oValue);
        oValue->clear();
        return is_supported;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<>
struct PrimitiveDataConversion<refptr<BaseClass>, ObjectReference>
{
    static const bool is_supported = true;
    inline bool operator() (refptr<BaseClass>* oValue, ObjectReference const& iValue)
    {
        return iValue.GetROK(oValue);
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool SetToNullROK(refptr<BaseClass>* oValue);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<>
struct PrimitiveDataConversion<refptr<BaseClass>, nullptr_t>
{
    static const bool is_supported = true;
    inline bool operator() (refptr<BaseClass>* oValue, nullptr_t)
    {
        return SetToNullROK(oValue);
    }
};
//=============================================================================
template<typename T> template<typename U> bool PrimitiveData<T>::AsROK(U* oValue) const
{
    return PrimitiveDataConversion<U,T>()(oValue, m_value);
}
//=============================================================================
bool DoesContainObjectReference(IPrimitiveData* iData);
//=============================================================================
#if SG_ENABLE_TOOLS
std::string ToString(IPrimitiveData* iData);
#endif
//=============================================================================
}
}

#endif

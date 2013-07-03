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
#define APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(MACRO) \
    MACRO(Null,             nullptr_t)                 \
    MACRO(Boolean,          bool)                      \
    MACRO(Int32,            i32)                       \
    MACRO(UInt32,           u32)                       \
    MACRO(Float,            float)                     \
    MACRO(String,           std::string)               \
    MACRO(List,             PrimitiveDataList )        \
    MACRO(NamedList,        PrimitiveDataNamedList )   \
    MACRO(Object,           refptr<BaseClass>)         \
    MACRO(ObjectReference,  ObjectReference)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
enum class PrimitiveDataType
{
    Unknown,
#define DECLARE_ENUM_VALUE(ENUM_VALUE_NAME, CPP_TYPE) ENUM_VALUE_NAME,
    APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(DECLARE_ENUM_VALUE)
#undef DECLARE_ENUM_VALUE
    Last
};
//=============================================================================
inline char const* GetPrimitiveDataTypeName(PrimitiveDataType type)
{
    switch(type)
    {
#define CASE_RETURN_TYPE_NAME(ENUM_VALUE_NAME, CPP_TYPE) case PrimitiveDataType::ENUM_VALUE_NAME: return #ENUM_VALUE_NAME;
    APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(CASE_RETURN_TYPE_NAME)
#undef CASE_RETURN_TYPE_NAME
    default:
        return "unknown";
    }
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
#define DECLARE_PRIMITIVE_DATA_TRAITS(ENUM_VALUE_NAME, CPP_TYPE) \
    template<> struct PrimitiveDataTraits<STRIP_PARENTHESIS(CPP_TYPE)> \
    { \
        static const bool is_supported_type = true; \
        typedef STRIP_PARENTHESIS(CPP_TYPE) cpp_type; \
        static const PrimitiveDataType primitive_data_type = PrimitiveDataType::ENUM_VALUE_NAME; \
    };
    APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(DECLARE_PRIMITIVE_DATA_TRAITS)
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
#define CASE_CAST_AND_CONVERT(ENUM_VALUE_NAME, CPP_TYPE) \
    case PrimitiveDataType::ENUM_VALUE_NAME: \
        { \
            PrimitiveData<STRIP_PARENTHESIS(CPP_TYPE)> const* data =  \
                checked_cast<PrimitiveData<STRIP_PARENTHESIS(CPP_TYPE)> const*>(this); \
            return data->AsROK(oValue); \
        } \
        break;
    APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES(CASE_CAST_AND_CONVERT)
#undef CASE_CAST_AND_CONVERT
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return false;
}
#undef APPLY_MACRO_TO_PRIMITIVE_DATA_TYPES
//=============================================================================
template<typename T, typename U>
struct PrimitiveDataConversion
{
    static const bool is_supported = false;
    inline bool operator() (T* oValue, U const& iValue) { SG_UNUSED(iValue); SG_ASSERT(nullptr != oValue); return is_supported; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
struct PrimitiveDataConversion<T,T>
{
    static const bool is_supported = true;
    inline bool operator() (T* oValue, T const& iValue) { SG_ASSERT(nullptr != oValue); *oValue = iValue ; return is_supported; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST(DST, SRC) \
    template<> \
    struct PrimitiveDataConversion<DST,SRC> \
    { \
        static const bool is_supported = true; \
        inline bool operator() (DST* oValue, SRC const& iValue) \
        { \
            SG_ASSERT(nullptr != oValue); \
            *oValue = (DST)iValue ; \
            return is_supported; \
        } \
    };
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST(float, i32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST(float, u32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST(i32, u32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST(u32, i32)
DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST(std::string, nullptr_t)
#undef DEFINE_PRIMITIVE_DATA_CONVERSION_FROM_CAST
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

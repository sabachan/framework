#ifndef Reflection_Property_H
#define Reflection_Property_H

#include "PrimitiveData.h"

#include "Config.h"
#include <Core/ArrayList.h>
#include <Core/HashMap.h>
#include <Core/FastSymbol.h>
#include <Core/FastSymbol.h>
#include <Core/FilePath.h>
#include <Core/For.h>
#include <Core/SmartPtr.h>

namespace sg {
namespace reflection {
//=============================================================================
template <typename T>
class PropertyTraits;
//=============================================================================
template <typename T>
class PropertyTraitsForPrimitiveType
{
public:
    static_assert(PrimitiveDataTraits<T>::is_supported_type, "Unsupported property type");
    static void Box(T const& iValue, refptr<IPrimitiveData>* oValue)
    {
        *oValue = new PrimitiveData<T>(iValue);
    }
    static bool UnBoxIFP(T* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        return iValue->AsROK<T>(oValue);
    }
};
//=============================================================================
namespace is_declared_enum_internal {
    struct No  { char _[1]; };
    struct Yes { char _[2]; };
}
template <typename T> is_declared_enum_internal::No sg_reflection_DeclareEnum(T const*);
template <typename T> struct is_declared_enum
{
    static const bool value = sizeof(sg_reflection_DeclareEnum((T*)nullptr)) == sizeof(is_declared_enum_internal::Yes);
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct EnumValue
{
    i32 value;
    char const* name;
    char const* documentation;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class PropertyTraitsForEnum
{
public:
    static void Box(T const& iValue, refptr<IPrimitiveData>* oValue)
    {
        i32 valueAsInt = static_cast<i32>(iValue);
        *oValue = new PrimitiveData<i32>(valueAsInt);
    }
    static bool UnBoxIFP(T* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        switch(iValue->GetType())
        {
        case PrimitiveDataType::Null:
        case PrimitiveDataType::Boolean:
            SG_ASSERT_NOT_REACHED();
            break;
        case PrimitiveDataType::Int32:
        case PrimitiveDataType::UInt32:
            {
                i32 const valueAsInt = iValue->As<i32>();
#if SG_ENABLE_ASSERT
                EnumValue const* enumValues;
                size_t enumValueCount;
                GetEnumValues(oValue, enumValues, enumValueCount);
                bool valueFound = false;
                for(size_t i = 0; i < enumValueCount; ++i)
                {
                    EnumValue const& enumValue = enumValues[i];
                    if(enumValue.name != nullptr && valueAsInt == enumValue.value)
                    {
                        valueFound = true;
                        break;
                    }
                }
                SG_ASSERT(valueFound);
#endif
                *oValue = static_cast<T>(valueAsInt);
            }
            break;
        case PrimitiveDataType::Float:
            SG_ASSERT_NOT_REACHED();
            break;
        case PrimitiveDataType::String:
            {
                std::string valueAsStr;
                iValue->As<std::string>(&valueAsStr);
                EnumValue const* enumValues;
                size_t enumValueCount;
                GetEnumValues(oValue, enumValues, enumValueCount);
                for(size_t i = 0; i < enumValueCount; ++i)
                {
                    EnumValue const& enumValue = enumValues[i];
                    if(nullptr != enumValue.name && 0 == valueAsStr.compare(enumValue.name))
                    {
                        *oValue = static_cast<T>(enumValue.value);
                        return true;
                    }
                }
                return false;
            }
            break;
        case PrimitiveDataType::List:
        case PrimitiveDataType::Object:
        case PrimitiveDataType::NamedList:
        default:
            SG_ASSERT_NOT_REACHED();
        }
        return true;
    }
};
//=============================================================================
template <typename T>
class PropertyTraitsForUnsupportedType
{
public:
    static_assert(-1 == sizeof(T), "this type is not supported as a property!");
};
//=============================================================================
template <typename T>
class PropertyTraits<T const>
{
public:
    static_assert(!std::is_const<T const>::value, "const member can't be exposed as a property!");
};
//=============================================================================
template <typename T, typename Mimicked, typename Getter, typename Setter>
class PropertyTraitsForMimic
{
    typedef Mimicked primitive_value_type;
public:
    static_assert(PrimitiveDataTraits<primitive_value_type>::is_supported_type, "Unsupported property type");
    static void Box(T const& iValue, refptr<IPrimitiveData>* oValue)
    {
        PropertyTraits<primitive_value_type>::Box(Getter()(iValue), oValue);
    }
    static bool UnBoxIFP(T* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        primitive_value_type value;
        bool ok =  PropertyTraits<primitive_value_type>::UnBoxIFP(&value, iValue);
        if(!ok)
            return false;
        Setter()(oValue, value);
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T> struct IntegerGetter { i32 operator() (T val) { return checked_numcastable(val); } };
template <typename T> struct IntegerSetter { void operator() (T* to, i32 from) { *to = checked_numcastable(from); } };
template <typename T> struct UnsignedGetter { u32 operator() (T val) { return checked_numcastable(val); } };
template <typename T> struct UnsignedSetter { void operator() (T* to, u32 from) { *to = checked_numcastable(from); } };
template <typename T> class PropertyTraitsForInteger : public PropertyTraitsForMimic<T, i32, IntegerGetter<T>, IntegerSetter<T> > {};
template <typename T> class PropertyTraitsForUnsigned : public PropertyTraitsForMimic<T, u32, UnsignedGetter<T>, UnsignedSetter<T> > {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct FilePathGetter { std::string operator() (FilePath const& fp) { return fp.GetPrintableString(); } };
struct FilePathSetter { void operator() (FilePath* fp, std::string const& v) { *fp = FilePath::CreateFromAnyPath(v); } };
template <> class PropertyTraits<FilePath> : public PropertyTraitsForMimic<FilePath, std::string, FilePathGetter, FilePathSetter> {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename domain> struct FastSymbolGetter { std::string operator() ( fastsymbol::TemplateFastSymbol<domain> const& s) { return s.Value(); } };
template <typename domain> struct FastSymbolSetter { void operator() (fastsymbol::TemplateFastSymbol<domain>* s, std::string const& v) { *s = fastsymbol::TemplateFastSymbol<domain>(v); } };
template <typename domain> class PropertyTraits<fastsymbol::TemplateFastSymbol<domain> > : public PropertyTraitsForMimic<fastsymbol::TemplateFastSymbol<domain>, std::string, FastSymbolGetter<domain>, FastSymbolSetter<domain> > {};
//=============================================================================
template <typename T>
class PropertyTraits : public std::conditional<
    PrimitiveDataTraits<T>::is_supported_type,
    PropertyTraitsForPrimitiveType<T>,
    typename std::conditional<
        std::is_integral<T>::value,
        typename std::conditional<
            std::is_unsigned<T>::value,
            PropertyTraitsForUnsigned<T>,
            PropertyTraitsForInteger<T> >::type,
        typename std::conditional<
            is_declared_enum<T>::value,
            PropertyTraitsForEnum<T>,
            PropertyTraitsForUnsupportedType<T> >::type
        >::type
    >::type
{};
//=============================================================================
struct BaseType;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class PropertyTraitsForBaseType
{
    typedef PrimitiveDataList vector_primitive_value_type;
    typedef PrimitiveData<vector_primitive_value_type> vector_primitive_data_type;
    typedef PrimitiveDataNamedList primitive_value_type;
    typedef PrimitiveData<primitive_value_type> primitive_data_type;
public:
    static void Box(T const& iValue, refptr<IPrimitiveData>* oValue)
    {
        Metaclass const* mc = MetaclassGetter<T>::GetMetaclass();
        primitive_data_type* data = new PrimitiveData<primitive_value_type>;
        PrimitiveDataNamedList& namedValueList = data->GetForWriting();
        size_t const size = mc->GetPropertyCount();
        for(size_t i = 0; i < size; ++i)
        {
            IProperty const* prop = mc->GetProperty(i);
            SG_ASSERT(nullptr != prop);
            refptr<IPrimitiveData> eltdata;
            prop->Get((void const*)&iValue, &eltdata);
            namedValueList.push_back(std::make_pair(prop->Name(), eltdata));
        }
        *oValue = data;
    }
    static bool UnBoxIFP(T* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        Metaclass const* mc = MetaclassGetter<T>::GetMetaclass();
        if(iValue->GetType() == PrimitiveDataType::List)
        {
            SG_ASSERT_MSG(mc->EnableListConstruct(), "This type can not be defined using list-like construct.");
            vector_primitive_data_type const* value = checked_cast<vector_primitive_data_type const*>(iValue);
            vector_primitive_value_type const& valueVector = value->Get();
            size_t const size = valueVector.size();
            SG_ASSERT(size == mc->GetPropertyCount());
            for(size_t i = 0; i < size; ++i)
            {
                IProperty const* prop = mc->GetProperty(i);
                bool ok = prop->SetROK((void*)oValue, valueVector[i].get());
                if(!ok)
                    return false;
            }
            return true;
        }
        else if(iValue->GetType() == PrimitiveDataType::NamedList)
        {
            SG_ASSERT_MSG(mc->EnableStructConstruct(), "This type can not be defined using struct-like construct.");
            *oValue = T(); // to reset to default values
            primitive_data_type const* value = checked_cast<primitive_data_type const*>(iValue);
            primitive_value_type const& valueVector = value->Get();
            size_t const size = valueVector.size();
            for(size_t i = 0; i < size; ++i)
            {
                IProperty const* prop = mc->GetPropertyIFP(valueVector[i].first.c_str());
                // TODO: error message: this property doesn't exist in type
                SG_ASSERT(nullptr != prop);
                bool ok = prop->SetROK((void*)oValue, valueVector[i].second.get());
                if(!ok)
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define COMPUTE_PROPERTY_TRAIT(T) \
    std::conditional< \
            std::is_convertible<T, BaseType>::value, \
            PropertyTraitsForBaseType<T>, \
            PropertyTraits<T> \
        >::type
//=============================================================================
template <typename T, size_t N>
class PropertyTraitsForArray
{
    typedef T element_type;
    typedef PrimitiveDataList primitive_value_type;
    typedef PrimitiveData<primitive_value_type> primitive_data_type;
    typedef typename COMPUTE_PROPERTY_TRAIT(element_type) element_property_traits;
public:
    static void Box(T const (&iValue)[N], refptr<IPrimitiveData>* oValue)
    {
        primitive_data_type* data = new primitive_data_type;
        primitive_value_type& v = data->GetForWriting();
        SG_ASSERT(v.empty());
        v.reserve(N);
        for_range_0(size_t, i, N)
        {
            refptr<IPrimitiveData> boxed_it;
            element_property_traits::Box(iValue[i], &boxed_it);
            v.push_back(boxed_it);
        }
        *oValue = data;
    }
    static bool UnBoxIFP(T (*oValue)[N], IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        SG_ASSERT(iValue->GetType() == PrimitiveDataType::List);
        if(iValue->GetType() != PrimitiveDataType::List)
            return false;
        primitive_data_type const* value = checked_cast<primitive_data_type const*>(iValue);
        primitive_value_type const& valueVector = value->Get();
        size_t const size = valueVector.size();
        SG_ASSERT_MSG(N == size, "incorrect number of elements in array");
        if(size != N)
            return false;
        for_range_0(size_t, i, N)
        {
            element_type& unboxed_it = (*oValue)[i];
            bool ok = element_property_traits::UnBoxIFP(&unboxed_it, valueVector[i].get());
            if(!ok)
                return false;
        }
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t N>
class PropertyTraits<T[N]> : public PropertyTraitsForArray<T, N> {};
//=============================================================================
template <typename VectorType, typename ElementType>
class PropertyTraitsForVectorType
{
    typedef VectorType vector_type;
    typedef ElementType element_type;
    typedef PrimitiveDataList primitive_value_type;
    typedef PrimitiveData<primitive_value_type> primitive_data_type;
    typedef typename COMPUTE_PROPERTY_TRAIT(ElementType) element_property_traits;
public:
    static void Box(vector_type const& iValue, refptr<IPrimitiveData>* oValue)
    {
        primitive_data_type* data = new primitive_data_type;
        primitive_value_type& v = data->GetForWriting();
        size_t const size = iValue.size();
        SG_ASSERT(v.empty());
        v.reserve(size);
        for(auto const& it : iValue)
        {
            refptr<IPrimitiveData> boxed_it;
            element_property_traits::Box(it, &boxed_it);
            v.push_back(boxed_it);
        }
        *oValue = data;
    }
    static bool UnBoxIFP(vector_type* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        SG_ASSERT(iValue->GetType() == PrimitiveDataType::List); // TODO: error message
        if(iValue->GetType() != PrimitiveDataType::List)
            return false;
        primitive_data_type const* value = checked_cast<primitive_data_type const*>(iValue);
        primitive_value_type const& valueVector = value->Get();
        size_t const size = valueVector.size();
        oValue->reserve(size);
        for(auto const& it : valueVector)
        {
            oValue->emplace_back();
            element_type& unboxed_it = oValue->back();
            bool ok = element_property_traits::UnBoxIFP(&unboxed_it, it.get());
            if(!ok)
                return false;
        }
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename A>
class PropertyTraits<std::vector<T, A> > : public PropertyTraitsForVectorType<std::vector<T, A>, T> {};
template <typename T, typename A, bool C>
class PropertyTraits<ArrayList<T, A, C> > : public PropertyTraitsForVectorType<ArrayList<T, A, C>, T> {};
//=============================================================================
template <typename MapType, typename KeyType, typename ElementType>
class PropertyTraitsForMapType
{
    typedef MapType map_type;
    typedef KeyType key_type;
    typedef ElementType element_type;
    typedef PrimitiveDataList primitive_value_type_list;
    typedef PrimitiveData<primitive_value_type_list> primitive_data_type_list;
    typedef PrimitiveDataList primitive_value_type_pair;
    typedef PrimitiveData<primitive_value_type_pair> primitive_data_type_pair;
    typedef typename COMPUTE_PROPERTY_TRAIT(KeyType) key_property_traits;
    typedef typename COMPUTE_PROPERTY_TRAIT(ElementType) element_property_traits;
public:
    static void Box(map_type const& iValue, refptr<IPrimitiveData>* oValue)
    {
        primitive_data_type_list* data = new primitive_data_type_list;
        primitive_value_type_list& v = data->GetForWriting();
        size_t const size = iValue.size();
        SG_ASSERT(v.empty());
        v.reserve(size);
        for(auto const& it : iValue)
        {
            primitive_data_type_list* data_pair = new primitive_data_type_list;
            primitive_value_type_list& pair = data_pair->GetForWriting();
            SG_ASSERT(pair.empty());
            pair.reserve(2);
            {
                refptr<IPrimitiveData> boxed_it;
                key_property_traits::Box(it.first, &boxed_it);
                pair.push_back(boxed_it);
            }
            {
                refptr<IPrimitiveData> boxed_it;
                element_property_traits::Box(it.second, &boxed_it);
                pair.push_back(boxed_it);
            }
            v.push_back(data_pair);
        }
        *oValue = data;
    }
    static bool UnBoxIFP(map_type* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        SG_ASSERT(iValue->GetType() == PrimitiveDataType::List);
        if(iValue->GetType() != PrimitiveDataType::List)
            return false;
        primitive_data_type_list const* value = checked_cast<primitive_data_type_list const*>(iValue);
        primitive_value_type_list const& valueVector = value->Get();
        size_t const size = valueVector.size();
        oValue->reserve(size);
        for(auto const& it : valueVector)
        {
            SG_ASSERT(it->GetType() == PrimitiveDataType::List);
            if(it->GetType() != PrimitiveDataType::List)
                return false;
            primitive_data_type_pair const* dataPair = checked_cast<primitive_data_type_pair const*>(it.get());
            primitive_value_type_pair const& valuePair= dataPair->Get();
            size_t const sizePair = valuePair.size();
            SG_ASSERT(2 == sizePair);
            if(2 != sizePair)
                return false;
            key_type key;
            element_type element;
            bool ok = key_property_traits::UnBoxIFP(&key, valuePair[0].get());
            if(!ok)
                return false;
            ok = element_property_traits::UnBoxIFP(&element, valuePair[1].get());
            if(!ok)
                return false;
            auto r = oValue->insert(std::make_pair(key, element));
            SG_ASSERT(r.second);
        }
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename K, typename T, typename A>
class PropertyTraits<std::unordered_map<K, T, A> > : public PropertyTraitsForMapType<std::unordered_map<K, T, A>, K, T> {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A, bool I>
class PropertyTraits<HashMap<K, T, H, C, P, A, I> > : public PropertyTraitsForMapType<HashMap<K, T, H, C, P, A, I>, K, T> {};
//=============================================================================
template <typename T1, typename T2>
class PropertyTraitsForStdPair
{
    typedef std::pair<T1, T2> pair_type;
    typedef T1 first_element_type;
    typedef T2 second_element_type;
    typedef PrimitiveDataList vector_primitive_value_type;
    typedef PrimitiveData<vector_primitive_value_type> vector_primitive_data_type;
    typedef PrimitiveDataNamedList primitive_value_type;
    typedef PrimitiveData<primitive_value_type> primitive_data_type;
    typedef typename COMPUTE_PROPERTY_TRAIT(first_element_type) first_element_property_traits;
    typedef typename COMPUTE_PROPERTY_TRAIT(second_element_type) second_element_property_traits;
public:
    static void Box(pair_type const& iValue, refptr<IPrimitiveData>* oValue)
    {
        primitive_data_type* data = new primitive_data_type;
        PrimitiveDataNamedList& namedValueList = data->GetForWriting();
        SG_ASSERT(namedValueList.empty());
        namedValueList.reserve(2);
        refptr<IPrimitiveData> boxed;
        first_element_property_traits::Box(iValue.first, &boxed);
        namedValueList.push_back(std::make_pair("first", boxed));
        second_element_property_traits::Box(iValue.second, &boxed);
        namedValueList.push_back(std::make_pair("second", boxed));
        *oValue = data;
    }
    static bool UnBoxIFP(pair_type* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        if(iValue->GetType() == PrimitiveDataType::List)
        {
            vector_primitive_data_type const* value = checked_cast<vector_primitive_data_type const*>(iValue);
            vector_primitive_value_type const& valueVector = value->Get();
            size_t const size = valueVector.size();
            SG_ASSERT(size == 2);
            if(size != 2)
                return false;
            {
                bool ok = first_element_property_traits::UnBoxIFP(&oValue->first, valueVector[0].get());
                if(!ok)
                    return false;
            }
            {
                bool ok = second_element_property_traits::UnBoxIFP(&oValue->second, valueVector[1].get());
                if(!ok)
                    return false;
            }
            return true;
        }
        else if(iValue->GetType() == PrimitiveDataType::NamedList)
        {
            *oValue = pair_type(); // to reset to default values
            primitive_data_type const* value = checked_cast<primitive_data_type const*>(iValue);
            primitive_value_type const& valueVector = value->Get();
            size_t const size = valueVector.size();
            SG_ASSERT(size == 2);
            for(size_t i = 0; i < size; ++i)
            {
                if(0 == strcmp(valueVector[i].first.c_str(), "first"))
                {
                    bool ok = first_element_property_traits::UnBoxIFP(&oValue->first, valueVector[0].second.get());
                    if(!ok)
                        return false;
                }
                else if(0 == strcmp(valueVector[i].first.c_str(), "second"))
                {
                bool ok = second_element_property_traits::UnBoxIFP(&oValue->second, valueVector[1].second.get());
                    if(!ok)
                        return false;
                }
                else
                {
                    SG_ASSERT_NOT_REACHED();
                    return false;
                }
            }
            return true;
        }
        else
            return false;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T1, typename T2> class PropertyTraits<std::pair<T1, T2> > : public PropertyTraitsForStdPair<T1, T2> {};
//=============================================================================
template <typename ObjectPtr, typename ObjectType>
class PropertyTraitsForObjectPtr
{
    typedef ObjectPtr ptr_type;
    typedef ObjectType object_type;
    typedef refptr<BaseClass> primitive_value_type;
    typedef PrimitiveData<primitive_value_type> primitive_data_type;
public:
    static void Box(ptr_type const& iValue, refptr<IPrimitiveData>* oValue)
    {
        primitive_data_type* data = new primitive_data_type;
        primitive_value_type& ptr = data->GetForWriting();
        ptr = (BaseClass*)iValue.get();
        *oValue = data;
    }
    static bool UnBoxIFP(ptr_type* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        //SG_ASSERT(iValue->GetType() == PrimitiveDataType::Object || iValue->GetType() == PrimitiveDataType::ObjectReference || iValue->GetType() == PrimitiveDataType::Null);
        if(iValue->GetType() == PrimitiveDataType::Object)
        {
            primitive_data_type const* value = checked_cast<primitive_data_type const*>(iValue);
            primitive_value_type const& ptr = value->Get();
            *oValue = checked_cast<object_type*>(ptr.get());
            return true;
        }
        else if(iValue->GetType() == PrimitiveDataType::ObjectReference)
        {
            primitive_value_type ptr;
            bool ok = iValue->AsROK<refptr<BaseClass> >(&ptr);
            SG_ASSERT_AND_UNUSED(ok);
            SG_ASSERT(object_type::StaticGetMetaclass()->IsBaseOf(ptr->GetMetaclass())); // TODO: error message
            *oValue = checked_cast<object_type*>(ptr.get());
            return true;
        }
        else if(iValue->GetType() == PrimitiveDataType::Null)
        {
            *oValue = nullptr;
            return true;
        }
        else
            return false;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T> class PropertyTraits<refptr<T> > : public PropertyTraitsForObjectPtr<refptr<T>, T > { };
template <typename T> class PropertyTraits<safeptr<T> > : public PropertyTraitsForObjectPtr<safeptr<T>, T > { };
//=============================================================================
template <typename PrimitiveDataPtr>
class PropertyTraitsForPrimitiveData
{
    typedef PrimitiveDataPtr ptr_type;
public:
    static void Box(ptr_type const& iValue, refptr<IPrimitiveData>* oValue)
    {
        iValue->CopyTo(*oValue);
    }
    static bool UnBoxIFP(ptr_type* oValue, IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        *oValue = iValue;
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<> class PropertyTraits<refptr<IPrimitiveData> > : public PropertyTraitsForPrimitiveData<refptr<IPrimitiveData> > { };
template<> class PropertyTraits<refptr<IPrimitiveData const> > : public PropertyTraitsForPrimitiveData<refptr<IPrimitiveData const> > { };
//=============================================================================
class IProperty : public RefAndSafeCountable
{
public:
    IProperty()
        : m_name(nullptr) 
#if ENABLE_REFLECTION_DOCUMENTATION
        , m_documentation(nullptr)
#endif
    {
    }
    virtual ~IProperty() {}
    char const* Name() const { return m_name; }

    virtual bool SetROK(void* iObject, IPrimitiveData const* iValue) const = 0;
    virtual void Get(void const* iObject, refptr<IPrimitiveData>* oValue) const = 0;

    void SetName(char const* iName) { m_name = iName; }
private:
    char const* m_name;
#if ENABLE_REFLECTION_DOCUMENTATION
public:
    char const* Documentation() const { return m_documentation; }
    void SetDocumentation(char const* iDocumentation) { m_documentation = iDocumentation; }
private:
    char const* m_documentation;
#endif
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class PropertyWithOffset : public IProperty
{
    SG_NON_COPYABLE(PropertyWithOffset)
    typedef typename COMPUTE_PROPERTY_TRAIT(T) property_traits;
public:
    PropertyWithOffset(size_t iOffset) : m_offset(iOffset) {}
    virtual bool SetROK(void* iObject, IPrimitiveData const* iValue) const override
    {
        T& dst = *((T*)(void*)((char*)iObject + m_offset));
        return property_traits::UnBoxIFP(&dst, iValue);
    }
    virtual void Get(void const* iObject, refptr<IPrimitiveData>* oValue) const override
    {
        T const& src = *((T*)(void const*)((char const*)iObject + m_offset));
        property_traits::Box(src, oValue);
    }
private:
    size_t const m_offset;
};
//=============================================================================
}
}

#endif

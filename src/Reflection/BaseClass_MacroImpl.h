#ifndef Reflection_BaseClass_Include_MacroImpl
#error "this file must be included only by BaseClass.h"
#endif

#include <Core/Log.h>
#include <Core/StringFormat.h>

#include <Core/Preprocessor.h>
#include <cstddef>
#include <type_traits>

//=============================================================================

#define SG_REFLECTION_IMPL_NAMESPACE_AS_STR_ARRAY_ITEM(ITEM) #ITEM SG_COMMA
#define SG_REFLECTION_IMPL_NAMESPACE_AS_STR_ARRAY(NS) { APPLY_MACRO_FOR_EACH(SG_REFLECTION_IMPL_NAMESPACE_AS_STR_ARRAY_ITEM, NS) nullptr }

#define SG_REFLECTION_IMPL_NAMESPACE_ITEM(ITEM) :: ITEM
#define SG_REFLECTION_IMPL_NAMESPACE(NS) APPLY_MACRO_FOR_EACH(SG_REFLECTION_IMPL_NAMESPACE_ITEM, NS)

#define SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) SG_REFLECTION_IMPL_NAMESPACE(NS) :: STRIP_PARENTHESIS(TYPE)

#define SG_REFLECTION_IMPL_NAMESPACE_OPEN_ITEM(ITEM) namespace ITEM {
#define SG_REFLECTION_IMPL_NAMESPACE_CLOSE_ITEM(ITEM) }
#define SG_REFLECTION_IMPL_NAMESPACE_SCOPED(NS, SCOPED) \
    APPLY_MACRO_FOR_EACH(SG_REFLECTION_IMPL_NAMESPACE_OPEN_ITEM, NS) \
    STRIP_PARENTHESIS(SCOPED) \
    APPLY_MACRO_FOR_EACH(SG_REFLECTION_IMPL_NAMESPACE_CLOSE_ITEM, NS)

//=============================================================================

#define SG_REFLECTION_IMPL_ENUM_HEADER(TYPE) \
    ::sg::reflection::is_declared_enum_internal::Yes sg_reflection_DeclareEnum(TYPE const*); \
    void GetEnumValues(TYPE const*, ::sg::reflection::EnumValue const*& oEnumValues, size_t& oEnumValueCount);

#define SG_REFLECTION_IMPL_ENUM_BEGIN(TYPE, IDENTIFIER) \
    namespace namespace_EnumTraits_##IDENTIFIER { \
        void GetEnumValues(::sg::reflection::EnumValue const*& enumValues, size_t& enumValueCount); \
    } \
    void GetEnumValues(TYPE const*, ::sg::reflection::EnumValue const*& oEnumValues, size_t& oEnumValueCount) \
    { \
        namespace_EnumTraits_##IDENTIFIER::GetEnumValues(oEnumValues, oEnumValueCount); \
    } \
    namespace namespace_EnumTraits_##IDENTIFIER { \
        typedef TYPE enum_type; \
        ::sg::reflection::EnumValue enumValues[] = {

#define SG_REFLECTION_IMPL_ENUM_DOC(DOC) \
            { 0, nullptr/*No name means enum doc*/, #DOC },

#define SG_REFLECTION_IMPL_REFLECTION_ENUM_NAMED_VALUE_DOC(VALUE, NAME, DOC) \
            { static_cast<i32>(enum_type::VALUE), #NAME, #DOC },

#define SG_REFLECTION_IMPL_REFLECTION_C_ENUM_NAMED_VALUE_DOC(VALUE, NAME, DOC) \
            { static_cast<i32>(VALUE), #NAME, #DOC },

#define SG_REFLECTION_IMPL_ENUM_END \
        }; \
        SG_FORCE_INLINE void GetEnumValues(::sg::reflection::EnumValue const*& oEnumValues, size_t& oEnumValueCount) \
        { \
            oEnumValues = enumValues; \
            oEnumValueCount = SG_ARRAYSIZE(enumValues); \
        } \
    }

//=============================================================================

#define SG_REFLECTION_IMPL_COMMON_HEADER(TYPE, PARENT) \
    public: \
        static ::sg::reflection::Metaclass const* StaticGetMetaclass() { return s_sg_reflection_metaclassRegistrator.metaclass; } \
        static void sg_reflection_DeclareMetaclass() { \
            SG_LOG_INFO("reflection", "declare " #TYPE); \
            SG_ASSERT_MSG(!s_sg_reflection_isMetaclassDeclared, "metaclass already declared"); \
            s_sg_reflection_isMetaclassDeclared = true; } \
    private: \
        typedef STRIP_PARENTHESIS(TYPE) reflection_this_type; \
        typedef STRIP_PARENTHESIS(PARENT) reflection_parent_type; \
        static ::sg::reflection::Metaclass* sg_reflection_CreateMetaclass(::sg::reflection::MetaclassRegistrator* iRegistrator); \
        static bool s_sg_reflection_isMetaclassDeclared; \
    protected: \
        static ::sg::reflection::MetaclassRegistrator s_sg_reflection_metaclassRegistrator; \
    private:

#define SG_REFLECTION_IMPL_TYPE_HEADER(TYPE, PARENT) \
    SG_REFLECTION_IMPL_COMMON_HEADER(TYPE, PARENT)

#define SG_REFLECTION_IMPL_CLASS_HEADER(CLASS, PARENT) \
    SG_REFLECTION_IMPL_COMMON_HEADER(CLASS, PARENT) \
    public: \
        template <typename... T> \
        static ::sg::reflection::BaseClass* sg_reflection_StaticCreateInstance(T... param) \
        { \
            static_assert(0 == sizeof...(param), \
                "This param is just here to have a valid code for abstract class that would have no default constructor. " \
                "It is not meant to be called with parameters."); \
            return new STRIP_PARENTHESIS(CLASS)(param...); \
        } \
    protected: \
       virtual ::sg::reflection::Metaclass const* VirtualGetMetaclass() const override { return StaticGetMetaclass(); } \
    private:

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

#define SG_REFLECTION_IMPL_FORCE_METACLASS_REGISTRATION_IN_NAMESPACE(NAME) \
    ::sg::reflection::Metaclass const* sg_reflection_force_metaclass_registration_for_##NAME(); \
    ::sg::reflection::Metaclass const* const sg_reflection_force_metaclass_registration_for_##NAME##_mc = \
        sg_reflection_force_metaclass_registration_for_##NAME();

#define SG_REFLECTION_IMPL_FORCE_METACLASS_REGISTRATION(NS, NAME) \
    SG_REFLECTION_IMPL_NAMESPACE_SCOPED(NS, SG_REFLECTION_IMPL_FORCE_METACLASS_REGISTRATION_IN_NAMESPACE(NAME)) \

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

#define SG_REFLECTION_IMPL_DEFINE_CreateObject(TYPE, NAME) \
    ::sg::reflection::BaseClass* sg_reflection_CreateObject_##NAME() { return STRIP_PARENTHESIS(TYPE)::sg_reflection_StaticCreateInstance(); }

#define SG_REFLECTION_IMPL_DEFINE_force_registration(TYPE, NAME) \
    ::sg::reflection::Metaclass const* sg_reflection_force_metaclass_registration_for_##NAME() \
    { \
        STRIP_PARENTHESIS(TYPE)::sg_reflection_DeclareMetaclass(); \
        return STRIP_PARENTHESIS(TYPE)::StaticGetMetaclass(); \
    } \

#define SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST(TEMPLATE_PREFIX, NS, TYPE, NAME, ISCLASS, ISCONCRETE, ENABLE_LIST, ENABLE_STRUCT) \
    static_assert(ISCLASS || (!ISCONCRETE), "ISCONCRETE must be set to false when ISCLASS is false" ); \
    static_assert(!std::is_polymorphic<SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) >::value || ISCLASS, \
        "A polymorphic class must derive from ::sg::reflection::BaseClass" ); \
    namespace { \
    char const*const sg_reflection_namespace_##NAME [] = SG_REFLECTION_IMPL_NAMESPACE_AS_STR_ARRAY(NS); \
    ::sg::reflection::MetaclassInfo const sg_reflection_metaclass_info_##NAME = { \
        sg_reflection_namespace_##NAME, \
        SG_ARRAYSIZE(sg_reflection_namespace_##NAME)-1, \
        #NAME, \
        ISCLASS, \
        ISCONCRETE, \
        ENABLE_LIST, \
        ENABLE_STRUCT \
    }; \
    } \
    TEMPLATE_PREFIX bool STRIP_PARENTHESIS(TYPE)::s_sg_reflection_isMetaclassDeclared = false; \
    TEMPLATE_PREFIX ::sg::reflection::MetaclassRegistrator STRIP_PARENTHESIS(TYPE)::s_sg_reflection_metaclassRegistrator( \
        &SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE)::reflection_parent_type::s_sg_reflection_metaclassRegistrator, \
        SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE)::sg_reflection_CreateMetaclass, \
        sg_reflection_metaclass_info_##NAME, \
        SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE)::s_sg_reflection_isMetaclassDeclared \
    ); \
    SG_REFLECTION_IMPL_DEFINE_force_registration(TYPE, NAME) \
    IF_THEN(ISCONCRETE, SG_REFLECTION_IMPL_DEFINE_CreateObject(TYPE, NAME) ) \
    TEMPLATE_PREFIX ::sg::reflection::Metaclass* SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE)::sg_reflection_CreateMetaclass( \
        ::sg::reflection::MetaclassRegistrator* iRegistrator) \
    { \
        static_assert(std::is_same<SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE), SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE)::reflection_this_type>::value, \
            "Incorrect type name in macro for reflection in header!"); \
        reflection_this_type* dummyObject = (reflection_this_type*)0; \
        SG_UNUSED(dummyObject); /* sometimes unused */ \
        ::sg::reflection::Metaclass* mc = new ::sg::reflection::Metaclass( \
            iRegistrator, \
            IF_THEN_ELSE(ISCONCRETE, &sg_reflection_CreateObject_##NAME, nullptr ) \
        );

#define SG_REFLECTION_IMPL_TYPE_BEGIN(NS, TYPE) \
    SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST( , NS, TYPE, TYPE, false, false, true, true)

#define SG_REFLECTION_IMPL_TYPE_BEGIN_WITH_CONSTRUCTS(NS, TYPE, ENABLE_LIST, ENABLE_STRUCT) \
    SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST( , NS, TYPE, TYPE, false, false, ENABLE_LIST, ENABLE_STRUCT)

#define SG_REFLECTION_IMPL_CLASS_BEGIN(NS, CLASS) \
    SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST( , NS, CLASS, CLASS, true, true, false, true)

#define SG_REFLECTION_IMPL_ABSTRACT_CLASS_BEGIN(NS, CLASS) \
    SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST( , NS, CLASS, CLASS, true, false, false, true)

#define SG_REFLECTION_IMPL_TEMPLATE_CLASS_BEGIN(NS, CLASS, NAME) \
    SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST(template<>, NS, CLASS, NAME, true, true, false, true)

#define SG_REFLECTION_IMPL_TEMPLATE_ABSTRACT_CLASS_BEGIN(NS, CLASS, NAME) \
    SG_REFLECTION_IMPL_TYPE_BEGIN_ISCLASS_ISCONCRETE_ENABLELIST_ENABLENAMEDLIST(template<>, NS, CLASS, NAME, true, false, false, true)

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

#if ENABLE_REFLECTION_DOCUMENTATION
#  define SG_REFLECTION_IMPL_TYPE_DOC(DOC) \
        char const* typeDoc = DOC; \
        mc->SetDocumentation(typeDoc);
#  define SG_REFLECTION_IMPL_PROPERTY_DOC(DOC) \
            char const* propDoc = DOC; \
            prop->SetDocumentation(propDoc);
#else
#  define SG_REFLECTION_IMPL_TYPE_DOC(DOC)
#  define SG_REFLECTION_IMPL_PROPERTY_DOC(DOC)
#endif

#define SG_REFLECTION_IMPL_REFLECTION_NAMED_PROPERTY_DOC(MEMBER, NAME, DOC) \
        { \
            ::sg::reflection::IProperty* prop = \
                new ::sg::reflection::PropertyWithOffset<std::remove_reference<decltype(dummyObject->MEMBER)>::type>( \
                    offsetof(reflection_this_type, MEMBER) ); \
            char const* propName = #NAME; \
            prop->SetName(propName); \
            SG_REFLECTION_IMPL_PROPERTY_DOC(DOC) \
            mc->RegisterProperty(prop); \
        }

#define SG_REFLECTION_IMPL_CLASS_DOC(DOC) SG_REFLECTION_IMPL_TYPE_DOC(DOC)

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

#define SG_REFLECTION_IMPL_TYPE_END \
        return mc; \
    }

#define SG_REFLECTION_IMPL_CLASS_END SG_REFLECTION_IMPL_TYPE_END

//=============================================================================

enum class is_in_namespace_sg_reflection { value = 0 };
namespace sg {
namespace reflection {
    enum class is_in_namespace_sg_reflection { value = 1 };
}
}

#define SG_REFLECTION_IMPL_DETAIL_TYPE_WRAPPER_HEADER(NS, TYPE, NAME) \
    typedef SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) sg_reflection_wrapped_type_##NAME; \
    inline std::pair<char const*const*, size_t> sg_reflection_wrapped_type_namespace_##NAME() \
    { \
        static char const* const ns[] = SG_REFLECTION_IMPL_NAMESPACE_AS_STR_ARRAY(NS); \
        return std::make_pair(ns, SG_ARRAYSIZE(ns)-1); \
    } \
    struct sg_reflection_metaclassRegistratorWrapper_##NAME \
    { \
        static ::sg::reflection::MetaclassRegistrator metaclassRegistrator; \
    }; \
    static_assert(!std::is_polymorphic<SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) >::value, \
        "A polymorphic type must derive from ::sg::reflection::BaseClass" ); \
    template <> \
    class PropertyTraits<SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) > : public PropertyTraitsForBaseType<SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) > {}; \
    template <> struct MetaclassGetter<SG_REFLECTION_IMPL_FULL_TYPE(NS, TYPE) > \
    { \
        static Metaclass const* GetMetaclass() \
        { \
            return sg_reflection_metaclassRegistratorWrapper_##NAME::metaclassRegistrator.metaclass; \
        } \
    };

#define SG_REFLECTION_IMPL_TYPE_WRAPPER_HEADER(NS, TYPE, NAME) SG_REFLECTION_IMPL_DETAIL_TYPE_WRAPPER_HEADER(NS, TYPE, NAME)

// TODO: How to remove the call for sg_reflection_wrapped_type_namespace_?

#define SG_REFLECTION_IMPL_DETAIL_TYPE_WRAPPER_BEGIN(NAME) \
    ::sg::reflection::Metaclass* sg_reflection_CreateMetaclass_##NAME(MetaclassRegistrator* iRegistrator); \
    namespace { \
        ::sg::reflection::MetaclassInfo const sg_reflection_metaclass_info_##NAME = { \
            sg_reflection_wrapped_type_namespace_##NAME().first, \
            sg_reflection_wrapped_type_namespace_##NAME().second, \
            #NAME, \
            false, \
            false, \
            true, \
            true \
        }; \
        bool s_sg_reflection_isMetaclassDeclared_##NAME = false; \
    } \
    ::sg::reflection::MetaclassRegistrator sg_reflection_metaclassRegistratorWrapper_##NAME::metaclassRegistrator( \
        &::sg::reflection::BaseType::s_sg_reflection_metaclassRegistrator, \
        sg_reflection_CreateMetaclass_##NAME, \
        sg_reflection_metaclass_info_##NAME, \
        s_sg_reflection_isMetaclassDeclared_##NAME \
    ); \
    ::sg::reflection::Metaclass* sg_reflection_CreateMetaclass_##NAME(MetaclassRegistrator* iRegistrator) \
    { \
        typedef sg_reflection_wrapped_type_##NAME reflection_this_type; \
        reflection_this_type* dummyObject = (reflection_this_type*)0; \
        SG_UNUSED(dummyObject); /* sometimes unused */ \
        ::sg::reflection::Metaclass* mc = new Metaclass( \
            iRegistrator, \
            nullptr \
        ); \

#define SG_REFLECTION_IMPL_TYPE_WRAPPER_BEGIN(NAME) SG_REFLECTION_IMPL_DETAIL_TYPE_WRAPPER_BEGIN(NAME)

#define SG_REFLECTION_IMPL_TYPE_WRAPPER_DOC(DOC) SG_REFLECTION_IMPL_TYPE_DOC(DOC)

#define SG_REFLECTION_IMPL_TYPE_WRAPPER_END SG_REFLECTION_IMPL_TYPE_END

//=============================================================================

// to improve with a dedicated message mentionning class name, instanciation site, etc.
#define SG_REFLECTION_IMPL_CHECK_PROPERTY(NAME, TEST)               SG_ASSERT(TEST)
#define SG_REFLECTION_IMPL_CHECK_PROPERTY_MSG(NAME, TEST, MSG)      SG_ASSERT_MSG(TEST, MSG)
// to improve with retreival of value from property name, as it may be not a "m_" member.
#define SG_REFLECTION_IMPL_CHECK_PROPERTY_NotNull(NAME)             SG_ASSERT(nullptr != m_##NAME)
#define SG_REFLECTION_IMPL_CHECK_PROPERTY_Greater(NAME, VAL)        SG_ASSERT(m_##NAME > VAL)
#define SG_REFLECTION_IMPL_CHECK_PROPERTY_GreaterEqual(NAME, VAL)   SG_ASSERT(m_##NAME >= VAL)
#define SG_REFLECTION_IMPL_CHECK_PROPERTY_Less(NAME, VAL)           SG_ASSERT(m_##NAME < VAL)
#define SG_REFLECTION_IMPL_CHECK_PROPERTY_LessEqual(NAME, VAL)      SG_ASSERT(m_##NAME <= VAL)
//=============================================================================

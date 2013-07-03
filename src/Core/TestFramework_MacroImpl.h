#ifndef Core_TestFramework_Include_MacroImpl
#error "this file must be included only by TestFramework.h"
#endif

#include <Core/Preprocessor.h>

//=============================================================================

#define SG_TEST_FRAMEWORK_IMPL_LIST_AS_STR_ARRAY_ITEM(ITEM) #ITEM SG_COMMA
#define SG_TEST_FRAMEWORK_IMPL_LIST_AS_STR_ARRAY(NS) { APPLY_MACRO_FOR_EACH(SG_TEST_FRAMEWORK_IMPL_LIST_AS_STR_ARRAY_ITEM, NS) nullptr }


#define SG_TEST_FRAMEWORK_IMPL_DEFINE_force_registration(NAME) \
    ::sg::testframework::TestRegistrator const* sg_testframework_force_registration_##NAME() \
    { \
        sg_testframework_declared_##NAME = true; \
        return &sg_testframework_registrator_##NAME; \
    } \

#define SG_TEST_FRAMEWORK_IMPL(NAMESPACE, NAME, TAG_LIST) \
    char const*const sg_testframework_namespace_##NAME [] = SG_TEST_FRAMEWORK_IMPL_LIST_AS_STR_ARRAY(NAMESPACE); \
    char const*const sg_testframework_taglist_##NAME [] = SG_TEST_FRAMEWORK_IMPL_LIST_AS_STR_ARRAY(TAG_LIST); \
    bool sg_testframework_declared_##NAME = false; \
    void sg_testframework_function_##NAME(); \
    ::sg::testframework::TestRegistrator const sg_testframework_registrator_##NAME = \
        ::sg::testframework::TestRegistrator( \
            sg_testframework_namespace_##NAME, \
            SG_ARRAYSIZE(sg_testframework_namespace_##NAME)-1, \
            #NAME, \
            sg_testframework_taglist_##NAME, \
            SG_ARRAYSIZE(sg_testframework_taglist_##NAME)-1, \
            sg_testframework_function_##NAME, \
            &sg_testframework_declared_##NAME \
        ); \
    SG_TEST_FRAMEWORK_IMPL_DEFINE_force_registration(NAME) \
    void sg_testframework_function_##NAME()


#define SG_TEST_FRAMEWORK_IMPL_FORCE_TEST_REGISTRATION(NAME) \
    ::sg::testframework::TestRegistrator const* sg_testframework_force_registration_##NAME(); \
    ::sg::testframework::TestRegistrator const* const sg_testframework_force_registration_##NAME##_registrator = \
        sg_testframework_force_registration_##NAME();

//=============================================================================



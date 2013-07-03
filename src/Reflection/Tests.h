#ifndef Reflection_Tests_H
#define Reflection_Tests_H

#include <Core/Config.h>

#if !SG_ENABLE_UNIT_TESTS
#error "should not be included when SG_ENABLE_UNIT_TESTS is 0"
#endif

#include "BaseClass.h"
#include "Metaclass.h"

namespace sg {
namespace reflectionTestExternal {
    struct TestExternalStruct
    {
    public:
        float a;
        float b;
    };
    template<typename T, typename U>
    class TestExternalTemplate
    {
    public:
        T a;
        U b;
    };
}
}
namespace sg {
namespace reflection {
    REFLECTION_TYPE_WRAPPER_HEADER((sg,reflectionTestExternal), TestExternalStruct, TestExternalStruct)
    REFLECTION_TYPE_WRAPPER_HEADER((sg,reflectionTestExternal), (TestExternalTemplate<u32, bool>), TestExternalTemplate_u32_bool)
}
}

namespace sg {
namespace reflectionTest {
//=============================================================================
enum class TestEnumA
{
    Value0 = 0,
    Value1,
    Value2,
    Value10 = 10,
    Value10bis = 10,
    Valuem1 = -1,
    Valuem5 = -5,
    InvisibleValue = 42,
};
REFLECTION_ENUM_HEADER(TestEnumA)
//=============================================================================
enum TestEnumB
{
    TestEnumB_Value0 = 0,
    TestEnumB_Value1,
    TestEnumB_Value2,
    TestEnumB_Valuem5 = -5,
};
REFLECTION_C_ENUM_HEADER(TestEnumB)
//=============================================================================
class TestClass_A;
//=============================================================================
class TestStruct_A : public reflection::BaseType
{
public:
    u32 u;

    TestStruct_A() : u() {}
    REFLECTION_TYPE_HEADER(TestStruct_A, BaseType)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestStruct_B : public TestStruct_A
{
public:
    float f;

    TestStruct_B() : f() {}
    REFLECTION_TYPE_HEADER(TestStruct_B, TestStruct_A)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestClass_A : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(TestClass_A, BaseClass)
protected:
    // Note that the constructor is protected to forbid instanciation by other
    // than reflection code.
    TestClass_A() : m_u() {}
protected:
    u32 m_u;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestClassWithSameProperty_A : public TestClass_A
{
    REFLECTION_CLASS_HEADER(TestClassWithSameProperty_A, TestClass_A)
protected:
    TestClassWithSameProperty_A() : m_u() {}
protected:
    u32 m_u;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestClass_B : public TestClass_A
{
    REFLECTION_CLASS_HEADER(TestClass_B, TestClass_A)
public:
    virtual void f() = 0;
protected:
    TestClass_B() : m_i() {}
protected:
    i32 m_i;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestClass_C : public TestClass_B
{
    REFLECTION_CLASS_HEADER(TestClass_C, TestClass_B)
public:
    // Having this constructor public can lead to an Assert (cf. cpp).
    TestClass_C() : m_b(), m_f(), m_str(), m_pair(), m_vectoru(), m_structB(), m_structExt(), m_templateExt_u32_bool(), m_vectorStructB() {}

    // Note how to implement a constructor that can be exposed to generic code.
    TestClass_C(auto_initialized_t) : TestClass_C() { reflection::ObjectCreationContext context; EndCreationIFN(context); }

    virtual void f() override { }
    bool m_b;
    float m_f;
    std::string m_str;
    std::pair<std::string, float> m_pair;
    std::vector<u32> m_vectoru;
    TestStruct_B m_structB;
    sg::reflectionTestExternal::TestExternalStruct m_structExt;
    sg::reflectionTestExternal::TestExternalTemplate<u32, bool> m_templateExt_u32_bool;
    std::vector<TestStruct_B> m_vectorStructB;
    std::vector<std::pair<int, float> > m_vectorPair;
    std::unordered_map<int, TestStruct_B> m_mapIntStructB;
    TestEnumA m_enumA;
    TestEnumB m_enumB;
    enum class TestEnumC
    {
        TestEnumC_Value0 = 0,
        TestEnumC_Value1,
        TestEnumC_Value2,
    };
    TestEnumC m_enumC;
};
REFLECTION_ENUM_HEADER(TestClass_C::TestEnumC)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestClass_D : public TestClass_B
{
    REFLECTION_CLASS_HEADER(TestClass_D, TestClass_B)
public:
    virtual void f() override { }
    refptr<TestClass_B> m_object;
    std::vector<refptr<TestClass_A> > m_objectlist;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
class TestTemplate_A : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(TestTemplate_A, BaseClass)
public:
    T m_varT;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename U>
class TestTemplate_B : public TestTemplate_A<T>
{
    REFLECTION_CLASS_HEADER(TestTemplate_B, TestTemplate_A)
public:
    virtual void f() = 0;
    U m_varU;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename U, typename V>
class TestTemplate_C : public TestTemplate_B<T, U>
{
    REFLECTION_CLASS_HEADER(TestTemplate_C, TestTemplate_B)
public:
    virtual void f() override { }
    V m_varV;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestClassDerivedFromStruct_A : public reflection::BaseClass
                                   , public TestStruct_A
{
    REFLECTION_CLASS_HEADER(TestClassDerivedFromStruct_A, BaseClass)
protected:
    TestClassDerivedFromStruct_A() : m_v() {}
protected:
    u32 m_v;
};
//=============================================================================
}
}

#endif

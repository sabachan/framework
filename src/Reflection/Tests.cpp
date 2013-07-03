#include "stdafx.h"

#include <Core/Config.h>

#if SG_ENABLE_UNIT_TESTS

#include "Tests.h"

#include "BaseClass.h"
#include "ObjectDatabase.h"
#include "PrimitiveData.h"
#include <Core/Log.h>
#include <Core/IntTypes.h>
#include <Core/TestFramework.h>

namespace sg {
namespace reflection {
    REFLECTION_TYPE_WRAPPER_BEGIN(TestExternalStruct)
        REFLECTION_TYPE_DOC("This type is for unit tests")
        REFLECTION_PROPERTY_DOC(a, "Property for unit test")
        REFLECTION_PROPERTY_DOC(b, "Property for unit test")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(TestExternalTemplate_u32_bool)
        REFLECTION_TYPE_DOC("This type is for unit tests")
        REFLECTION_PROPERTY_DOC(a, "Property for unit test")
        REFLECTION_PROPERTY_DOC(b, "Property for unit test")
    REFLECTION_TYPE_WRAPPER_END
}
}


namespace sg {
namespace reflectionTest {
//=============================================================================
REFLECTION_ENUM_BEGIN(TestEnumA)
    REFLECTION_ENUM_DOC("This enum is for unit tests")
    REFLECTION_ENUM_VALUE_DOC(Value0, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(Value1, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(Value2, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(Value10, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(Value10bis, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(Valuem1, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(Valuem5, "enum value for unit test")
REFLECTION_ENUM_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_C_ENUM_BEGIN(TestEnumB)
    REFLECTION_C_ENUM_DOC("This enum is for unit tests")
    REFLECTION_C_ENUM_VALUE_DOC(TestEnumB_Value0, "enum value for unit test")
    REFLECTION_C_ENUM_VALUE_DOC(TestEnumB_Value1, "enum value for unit test")
    REFLECTION_C_ENUM_VALUE_DOC(TestEnumB_Value2, "enum value for unit test")
    REFLECTION_C_ENUM_VALUE_DOC(TestEnumB_Valuem5, "enum value for unit test")
REFLECTION_C_ENUM_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_TYPE_BEGIN((sg,reflectionTest), TestStruct_A)
    REFLECTION_TYPE_DOC("This type is for unit tests")
    REFLECTION_PROPERTY_DOC(u, "Property for unit test")
REFLECTION_TYPE_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_TYPE_BEGIN((sg,reflectionTest), TestStruct_B)
    REFLECTION_TYPE_DOC("This type is for unit tests")
    REFLECTION_PROPERTY_DOC(f, "Property for unit test")
REFLECTION_TYPE_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,reflectionTest), TestClass_A)
    REFLECTION_CLASS_DOC("This class is for unit tests")
    REFLECTION_m_PROPERTY_DOC(u, "Property for unit test")
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,reflectionTest), TestClassWithSameProperty_A)
    REFLECTION_CLASS_DOC("This class is for unit tests")
    REFLECTION_m_PROPERTY_DOC(u, "Property for unit test (conflicting name with parent class)")
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg,reflectionTest), TestClass_B)
    REFLECTION_CLASS_DOC("This class is for unit tests")
    REFLECTION_m_PROPERTY_DOC(i, "Property for unit test")
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_QUALIFIED_ENUM_BEGIN(TestClass_C::TestEnumC, TestClass_C_TestEnumC)
    REFLECTION_ENUM_DOC("This enum is for unit tests")
    REFLECTION_ENUM_VALUE_DOC(TestEnumC_Value0, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(TestEnumC_Value1, "enum value for unit test")
    REFLECTION_ENUM_VALUE_DOC(TestEnumC_Value2, "enum value for unit test")
REFLECTION_ENUM_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,reflectionTest), TestClass_C)
    REFLECTION_CLASS_DOC("This class is for unit tests")
    REFLECTION_m_PROPERTY_DOC(b, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(f, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(str, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(pair, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(vectoru, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(structB, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(structExt, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(templateExt_u32_bool, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(vectorStructB, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(vectorPair, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(mapIntStructB, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(enumA, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(enumB, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(enumC, "Property for unit test")
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,reflectionTest), TestClass_D)
    REFLECTION_CLASS_DOC("This class is for unit tests");
    REFLECTION_m_PROPERTY_DOC(object, "Property for unit test")
    REFLECTION_m_PROPERTY_DOC(objectlist, "Property for unit test")
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_TEMPLATE_CLASS_BEGIN((sg,reflectionTest), (TestTemplate_A<int>), TestTemplate_A_int)
    REFLECTION_CLASS_DOC("This class is for unit tests");
REFLECTION_CLASS_END
REFLECTION_TEMPLATE_CLASS_BEGIN((sg,reflectionTest), (TestTemplate_A<float>), TestTemplate_A_float)
    REFLECTION_CLASS_DOC("This class is for unit tests");
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_TEMPLATE_ABSTRACT_CLASS_BEGIN((sg,reflectionTest), (TestTemplate_B<int, int>), TestTemplate_B_int_int)
    REFLECTION_CLASS_DOC("This class is for unit tests");
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_TEMPLATE_CLASS_BEGIN((sg,reflectionTest), (TestTemplate_C<int, int, int>), TestTemplate_C_int_int_int)
    REFLECTION_CLASS_DOC("This class is for unit tests");
REFLECTION_CLASS_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,reflectionTest), TestClassDerivedFromStruct_A)
    REFLECTION_CLASS_DOC("This class is for unit tests")
    REFLECTION_m_PROPERTY_DOC(v, "Property for unit test")
REFLECTION_CLASS_END
//=============================================================================
}
}
namespace sg {
namespace reflection {
//=============================================================================
SG_TEST((sg, reflection), Reflection, (Reflection, quick))
{
    IdentifierSymbol::Init();
    InitMetaclasses();

    {
        refptr<IPrimitiveData> data = new PrimitiveData<float>(2.56f);
        float v = data->As<float>();
        data = new PrimitiveData<i32>(-6);
        u32 v_u = data->As<u32>();
        i32 v_i = data->As<i32>();
        float v_f = data->As<float>();
        bool v_b = false;
        bool isConversionSupported = data->AsROK(&v_b);
        SG_ASSERT(!isConversionSupported);
        std::vector<refptr<IPrimitiveData> > v_l;
        isConversionSupported = data->AsROK(&v_l);
        SG_ASSERT(!isConversionSupported);
        SG_UNUSED((v, v_u, v_i, v_f));
    }

    {
        {
            Metaclass const* mcsa = GetMetaclass(Identifier("::sg::reflectionTest::TestStruct_A"));
            SG_ASSERT_AND_UNUSED(nullptr != mcsa);
            Metaclass const* mcsb = GetMetaclass(Identifier("::sg::reflectionTest::TestStruct_B"));
            SG_ASSERT_AND_UNUSED(nullptr != mcsb);
        }

        {
            Metaclass const* mca = GetMetaclassIFP(Identifier("::sg::reflection"), Identifier("reflectionTest::TestClass_A"));
            SG_ASSERT_AND_UNUSED(nullptr != mca);
            Metaclass const* mca2 = GetMetaclassIFP(Identifier("::sg::reflection"), Identifier("TestClass_A"));
            SG_ASSERT_AND_UNUSED(nullptr == mca2);
        }

        {
            Metaclass const* mca = GetMetaclass(Identifier("::sg::reflectionTest::TestClass_A"));
            SG_ASSERT(nullptr != mca);
            refptr<BaseClass> a = mca->CreateObject();
            ObjectCreationContext creationContext;
            a->EndCreationIFN(creationContext);
            Metaclass const* mc = a->GetMetaclass();
            SG_ASSERT_AND_UNUSED(mc == mca);

            {
                refptr<IPrimitiveData> data = new PrimitiveData<u32>(42);
                a->BeginModification();
                bool ok = a->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                ObjectModificationContext modificationContext;
                a->EndModification(modificationContext);
                refptr<IPrimitiveData> dataget;
                a->GetPropertyIFP("u", &dataget);
                SG_ASSERT(nullptr != dataget);
                SG_ASSERT(dataget->As<u32>() == 42);
            }
            {
                IProperty const* prop_u = mca->GetPropertyIFP("u");
                SG_ASSERT(nullptr != prop_u);

                a->BeginModification();
                refptr<IPrimitiveData> data = new PrimitiveData<i32>(-1);
                bool ok = a->SetPropertyROK(prop_u, data.get());
                SG_ASSERT_AND_UNUSED(ok);
                refptr<IPrimitiveData> dataget;
                a->GetProperty(prop_u, &dataget);
                SG_ASSERT(nullptr != dataget);
                SG_ASSERT(dataget->As<u32>() == 0xFFFFFFFF);
                ObjectModificationContext context;
                a->EndModification(context);
            }
            reflectionTest::TestClass_A* pa = checked_cast<reflectionTest::TestClass_A*>(a.get());
            SG_UNUSED(pa);
            a = nullptr;
        }

        // Activating following code should raise an error message
#if 0
        {
            Metaclass const* mca = GetMetaclass(Identifier("::sg::reflectionTest::TestClassWithSameProperty_A"));
            SG_ASSERT(nullptr != mca);
            refptr<BaseClass> a = mca->CreateObject();
            a->EndCreationIFN(ObjectCreationContext());
            Metaclass const* mc = a->GetMetaclass();
            SG_ASSERT(a->GetMetaclass() == mca);
            {
                refptr<IPrimitiveData> data = new PrimitiveData<u32>(42);
                a->BeginModification();
                bool ok = a->SetPropertyROK("u", data.get());
                SG_ASSERT(ok);
                a->EndModification(ObjectModificationContext());
                refptr<IPrimitiveData> dataget;
                a->GetPropertyIFP("u", &dataget);
                SG_ASSERT(nullptr != dataget);
                SG_ASSERT(dataget->As<u32>() == 42);
            }
            reflectionTest::TestClass_A* pa = checked_cast<reflectionTest::TestClass_A*>(a.get());
            a = nullptr;
        }
#endif

        {
            Metaclass const* mcb = GetMetaclass(Identifier("::sg::reflectionTest::TestClass_B"));
            SG_ASSERT_AND_UNUSED(nullptr != mcb);
        }

        {
            refptr<BaseClass> c = new reflectionTest::TestClass_C(auto_initialized);
        }
#if 0
        {
            refptr<BaseClass> thisShouldAssert = new reflectionTest::TestClass_C();
        }
#endif

        {
            Metaclass const* mcc = GetMetaclass(Identifier("::sg::reflectionTest::TestClass_C"));
            SG_ASSERT(nullptr != mcc);
            refptr<BaseClass> c = mcc->CreateObject();
            ObjectCreationContext context;
            c->EndCreationIFN(context);
            SG_ASSERT(c->GetMetaclass() == mcc);
            reflectionTest::TestClass_C* pc = checked_cast<reflectionTest::TestClass_C*>(c.get());
            SG_UNUSED(pc);

            {
                refptr<IPrimitiveData> data;
                c->BeginModification();
                data = new PrimitiveData<u32>(42);
                bool ok = c->SetPropertyROK("u", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<i32>(-1);
                ok = c->SetPropertyROK("i", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<bool>(true);
                ok = c->SetPropertyROK("b", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<float>(3.14f);
                ok = c->SetPropertyROK("f", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<std::string>("Hello world!");
                ok = c->SetPropertyROK("str", data.get());
                SG_ASSERT(ok);
                {
                    PrimitiveData<std::vector<refptr<IPrimitiveData> > >* dataPair = new PrimitiveData<std::vector<refptr<IPrimitiveData> > >;
                    std::vector<refptr<IPrimitiveData> >& v = dataPair->GetForWriting();
                    v.push_back(new PrimitiveData<std::string>("pi"));
                    v.push_back(new PrimitiveData<float>(3.14159265359f));
                    data = dataPair;
                    ok = c->SetPropertyROK("pair", data.get());
                    SG_ASSERT(ok);
                }
                {
                    PrimitiveData<std::vector<refptr<IPrimitiveData> > >* dataVectoru = new PrimitiveData<std::vector<refptr<IPrimitiveData> > >;
                    std::vector<refptr<IPrimitiveData> >& v = dataVectoru->GetForWriting();
                    v.push_back(new PrimitiveData<u32>(1));
                    v.push_back(new PrimitiveData<u32>(2));
                    v.push_back(new PrimitiveData<u32>(3));
                    data = dataVectoru;
                    ok = c->SetPropertyROK("vectoru", data.get());
                    SG_ASSERT(ok);
                }
                {
                    PrimitiveData<std::vector<refptr<IPrimitiveData> > >* dataStruct =
                        new PrimitiveData<std::vector<refptr<IPrimitiveData> > >;
                    std::vector<refptr<IPrimitiveData> >& v = dataStruct->GetForWriting();
                    v.push_back(new PrimitiveData<u32>(42));
                    v.push_back(new PrimitiveData<float>(3.14f));
                    data = dataStruct;
                    ok = c->SetPropertyROK("structB", data.get());
                    SG_ASSERT(ok);
                }
                {
                    PrimitiveData<std::vector<std::pair<std::string, refptr<IPrimitiveData> > > >* dataStruct =
                        new PrimitiveData<std::vector<std::pair<std::string, refptr<IPrimitiveData> > > >;
                    std::vector<std::pair<std::string, refptr<IPrimitiveData> > >& v = dataStruct->GetForWriting();
                    v.push_back(std::make_pair("a", new PrimitiveData<u32>(5)));
                    v.push_back(std::make_pair("b", new PrimitiveData<float>(5.1f)));
                    data = dataStruct;
                    ok = c->SetPropertyROK("structExt", data.get());
                    SG_ASSERT(ok);
                }
                {
                    PrimitiveData<std::vector<std::pair<std::string, refptr<IPrimitiveData> > > >* dataStruct =
                        new PrimitiveData<std::vector<std::pair<std::string, refptr<IPrimitiveData> > > >;
                    std::vector<std::pair<std::string, refptr<IPrimitiveData> > >& v = dataStruct->GetForWriting();
                    v.push_back(std::make_pair("a", new PrimitiveData<u32>(13)));
                    v.push_back(std::make_pair("b", new PrimitiveData<bool>(true)));
                    data = dataStruct;
                    ok = c->SetPropertyROK("templateExt_u32_bool", data.get());
                    SG_ASSERT(ok);
                }
                {
                    PrimitiveData<PrimitiveDataList>* dataVectorStructB = new PrimitiveData<PrimitiveDataList>;
                    PrimitiveDataList& v = dataVectorStructB->GetForWriting();
                    {
                        PrimitiveData<PrimitiveDataNamedList>* dataStruct = new PrimitiveData<PrimitiveDataNamedList>;
                        PrimitiveDataNamedList& l = dataStruct->GetForWriting();
                        l.push_back(std::make_pair("u", new PrimitiveData<u32>(1)));
                        l.push_back(std::make_pair("f", new PrimitiveData<float>(0.1f)));
                        v.push_back(dataStruct);
                    }
                    {
                        PrimitiveData<PrimitiveDataNamedList>* dataStruct = new PrimitiveData<PrimitiveDataNamedList>;
                        PrimitiveDataNamedList& l = dataStruct->GetForWriting();
                        l.push_back(std::make_pair("u", new PrimitiveData<u32>(2)));
                        l.push_back(std::make_pair("f", new PrimitiveData<float>(10)));
                        v.push_back(dataStruct);
                    }
                    {
                        PrimitiveData<PrimitiveDataNamedList>* dataStruct = new PrimitiveData<PrimitiveDataNamedList>;
                        PrimitiveDataNamedList& l = dataStruct->GetForWriting();
                        l.push_back(std::make_pair("u", new PrimitiveData<u32>(3)));
                        l.push_back(std::make_pair("f", new PrimitiveData<float>(-3.14f)));
                        v.push_back(dataStruct);
                    }
                    data = dataVectorStructB;
                    ok = c->SetPropertyROK("vectorStructB", data.get());
                    SG_ASSERT(ok);
                }
                {
                    PrimitiveData<PrimitiveDataList>* dataMapIntStructB = new PrimitiveData<PrimitiveDataList>;
                    PrimitiveDataList& v = dataMapIntStructB->GetForWriting();
                    {
                        PrimitiveData<PrimitiveDataList>* dataPair = new PrimitiveData<PrimitiveDataList>;
                        PrimitiveDataList& pair = dataPair->GetForWriting();
                        pair.push_back(new PrimitiveData<i32>(3));
                        PrimitiveData<PrimitiveDataNamedList>* dataStruct = new PrimitiveData<PrimitiveDataNamedList>;
                        PrimitiveDataNamedList& l = dataStruct->GetForWriting();
                        l.push_back(std::make_pair("u", new PrimitiveData<u32>(1)));
                        l.push_back(std::make_pair("f", new PrimitiveData<float>(0.1f)));
                        pair.push_back(dataStruct);
                        v.push_back(dataPair);
                    }
                    {
                        PrimitiveData<PrimitiveDataList>* dataPair = new PrimitiveData<PrimitiveDataList>;
                        PrimitiveDataList& pair = dataPair->GetForWriting();
                        pair.push_back(new PrimitiveData<i32>(5));
                        PrimitiveData<PrimitiveDataNamedList>* dataStruct = new PrimitiveData<PrimitiveDataNamedList>;
                        PrimitiveDataNamedList& l = dataStruct->GetForWriting();
                        l.push_back(std::make_pair("u", new PrimitiveData<u32>(2)));
                        l.push_back(std::make_pair("f", new PrimitiveData<float>(10)));
                        pair.push_back(dataStruct);
                        v.push_back(dataPair);
                    }
                    {
                        PrimitiveData<PrimitiveDataList>* dataPair = new PrimitiveData<PrimitiveDataList>;
                        PrimitiveDataList& pair = dataPair->GetForWriting();
                        pair.push_back(new PrimitiveData<i32>(7));
                        PrimitiveData<PrimitiveDataNamedList>* dataStruct = new PrimitiveData<PrimitiveDataNamedList>;
                        PrimitiveDataNamedList& l = dataStruct->GetForWriting();
                        l.push_back(std::make_pair("u", new PrimitiveData<u32>(3)));
                        l.push_back(std::make_pair("f", new PrimitiveData<float>(-3.14f)));
                        pair.push_back(dataStruct);
                        v.push_back(dataPair);
                    }
                    data = dataMapIntStructB;
                    ok = c->SetPropertyROK("mapIntStructB", data.get());
                    SG_ASSERT(ok);

                    refptr<IPrimitiveData> readData;
                    c->GetPropertyIFP("mapIntStructB", &readData);
                    SG_ASSERT(readData->GetType() == PrimitiveDataType::List);
                    PrimitiveDataList readList;
                    readData->As(&readList);
                    SG_ASSERT(readList.size() == v.size());
                }

                {
                    data = new PrimitiveData<i32>(0);
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<i32>(2);
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(ok);
                    // Activating following code should raise asserts
#if 0
                    data = new PrimitiveData<i32>(3);
                    c->SetPropertyROK("enumA", data.get());
                    data = new PrimitiveData<i32>(42);
                    c->SetPropertyROK("enumA", data.get());
#endif
                    data = new PrimitiveData<std::string>("Value1");
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<std::string>("Value10");
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<std::string>("Valuem5");
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<std::string>("Value10bis");
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<std::string>("Hello world!");
                    ok = c->SetPropertyROK("enumA", data.get());
                    SG_ASSERT(!ok);
                }

                {
                    data = new PrimitiveData<i32>(2);
                    ok = c->SetPropertyROK("enumB", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<std::string>("TestEnumB_Valuem5");
                    ok = c->SetPropertyROK("enumB", data.get());
                    SG_ASSERT(ok);
                }

                {
                    data = new PrimitiveData<i32>(2);
                    ok = c->SetPropertyROK("enumC", data.get());
                    SG_ASSERT(ok);
                    data = new PrimitiveData<std::string>("TestEnumC_Value1");
                    ok = c->SetPropertyROK("enumC", data.get());
                    SG_ASSERT(ok);
                }

                ObjectModificationContext modificationContext;
                c->EndModification(modificationContext);
            }
            {
                refptr<IPrimitiveData> dataget;
                c->GetPropertyIFP("u", &dataget);
                SG_ASSERT(nullptr != dataget);
                SG_ASSERT(dataget->As<u32>() == 42);
                c->GetPropertyIFP("vectoru", &dataget);
                SG_ASSERT(nullptr != dataget);
                SG_ASSERT(dataget->GetType() == PrimitiveDataType::List);
                c->GetPropertyIFP("structB", &dataget);
                SG_ASSERT(nullptr != dataget);
                SG_ASSERT(dataget->GetType() == PrimitiveDataType::NamedList);
            }
            c = nullptr;
        }

        {
            Metaclass const* mcc = GetMetaclass(Identifier("::sg::reflectionTest::TestClass_C"));
            SG_ASSERT(nullptr != mcc);
            refptr<BaseClass> c = mcc->CreateObject();
            Metaclass const* mcd = GetMetaclass(Identifier("::sg::reflectionTest::TestClass_D"));
            SG_ASSERT(nullptr != mcd);
            refptr<BaseClass> d1 = mcd->CreateObject();
            SG_ASSERT(d1->GetMetaclass() == mcd);
            reflectionTest::TestClass_D* pd1 = checked_cast<reflectionTest::TestClass_D*>(d1.get());
            refptr<BaseClass> d2 = mcd->CreateObject();
            reflectionTest::TestClass_D* pd2 = checked_cast<reflectionTest::TestClass_D*>(d2.get());
            SG_UNUSED((pd1, pd2));

            refptr<IPrimitiveData> data;
            {
                data = new PrimitiveData<u32 >(1);
                bool ok = c->SetPropertyROK("u", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<i32 >(-10);
                ok = c->SetPropertyROK("i", data.get());
                SG_ASSERT(ok);
            }
            {
                data = new PrimitiveData<u32 >(2);
                bool ok = d1->SetPropertyROK("u", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<i32 >(-20);
                ok = d1->SetPropertyROK("i", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<refptr<BaseClass> >(c.get());
                ok = d1->SetPropertyROK("object", data.get());
                SG_ASSERT(ok);
            }
            {
                data = new PrimitiveData<u32 >(3);
                bool ok = d2->SetPropertyROK("u", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<i32 >(-30);
                ok = d2->SetPropertyROK("i", data.get());
                SG_ASSERT(ok);
                data = new PrimitiveData<refptr<BaseClass> >(d1.get());
                ok = d2->SetPropertyROK("object", data.get());
                SG_ASSERT(ok);
            }
            ObjectCreationContext context;
            d2->EndCreationIFN(context);
            d1->EndCreationIFN(context);
            c->EndCreationIFN(context);
        }

        {
            Metaclass const* mcta_i = GetMetaclass(Identifier("::sg::reflectionTest::TestTemplate_A_int"));
            SG_ASSERT(nullptr != mcta_i);
            refptr<BaseClass> ta_i = mcta_i->CreateObject();
            ObjectCreationContext context;
            ta_i->EndCreationIFN(context);
            SG_ASSERT(ta_i->GetMetaclass() == mcta_i);
        }

        {
            Metaclass const* mcta_f = GetMetaclass(Identifier("::sg::reflectionTest::TestTemplate_A_float"));
            SG_ASSERT(nullptr != mcta_f);
            refptr<BaseClass> ta_f = mcta_f->CreateObject();
            ObjectCreationContext context;
            ta_f->EndCreationIFN(context);
            SG_ASSERT(ta_f->GetMetaclass() == mcta_f);
        }

        {
            Metaclass const* mctb_ii = GetMetaclass(Identifier("::sg::reflectionTest::TestTemplate_B_int_int"));
            SG_ASSERT_AND_UNUSED(nullptr != mctb_ii);
        }

        {
            Metaclass const* mctc_iii = GetMetaclass(Identifier("::sg::reflectionTest::TestTemplate_C_int_int_int"));
            SG_ASSERT(nullptr != mctc_iii);
            refptr<BaseClass> tc_iii = mctc_iii->CreateObject();
            ObjectCreationContext context;
            tc_iii->EndCreationIFN(context);
            SG_ASSERT(tc_iii->GetMetaclass() == mctc_iii);
        }
    }

    {
        Metaclass const* mcd = GetMetaclass(Identifier("::sg::reflectionTest::TestClass_D"));
        SG_ASSERT(nullptr != mcd);
        IProperty const* prop_object = mcd->GetPropertyIFP("object");
        SG_ASSERT(nullptr != prop_object);

        refptr<BaseClass> d1 = mcd->CreateObject();
        refptr<BaseClass> d2 = mcd->CreateObject();
        refptr<BaseClass> d3 = mcd->CreateObject();
        refptr<BaseClass> d4 = mcd->CreateObject();
        refptr<BaseClass> d5 = mcd->CreateObject();
        refptr<BaseClass> d6 = mcd->CreateObject();
        reflectionTest::TestClass_D* pd1 = checked_cast<reflectionTest::TestClass_D*>(d1.get());
        reflectionTest::TestClass_D* pd2 = checked_cast<reflectionTest::TestClass_D*>(d2.get());
        reflectionTest::TestClass_D* pd3 = checked_cast<reflectionTest::TestClass_D*>(d3.get());
        reflectionTest::TestClass_D* pd4 = checked_cast<reflectionTest::TestClass_D*>(d4.get());
        reflectionTest::TestClass_D* pd5 = checked_cast<reflectionTest::TestClass_D*>(d5.get());
        reflectionTest::TestClass_D* pd6 = checked_cast<reflectionTest::TestClass_D*>(d6.get());
        SG_UNUSED((pd1, pd2, pd3, pd4, pd5, pd6));

        ObjectDatabase db;

        {
            db.BeginTransaction();
            db.Add(ObjectVisibility::Export, Identifier("::ns::subns::object"), d1.get());
            {
                refptr<IPrimitiveData> data;
                data = new PrimitiveData<u32 >(1);
                bool ok = d1->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                db.AddDeferredProperty(Identifier("::ns::subns::object"), prop_object, new PrimitiveData<ObjectReference>(ObjectReference(&db, Identifier("::ns::subns::object"), Identifier("::ns::subns2::object"))));
            }
            db.Add(ObjectVisibility::Public, Identifier("::ns::subns2::object"), d2.get());
            {
                refptr<IPrimitiveData> data;
                data = new PrimitiveData<u32 >(2);
                bool ok = d2->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                db.AddDeferredProperty(Identifier("::ns::subns2::object"), prop_object, new PrimitiveData<ObjectReference>(ObjectReference(&db, Identifier("::ns::subns2::object"), Identifier("subns3::object"))));
            }
            db.Add(ObjectVisibility::Protected, Identifier("::ns::subns3::object"), d3.get());
            {
                refptr<IPrimitiveData> data;
                data = new PrimitiveData<u32 >(3);
                bool ok = d3->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                // must fail (in EndTransaction()): db.AddDeferredProperty(Identifier("::ns::subns3::object"), prop_object, new PrimitiveData<ObjectReference>(ObjectReference(&db, Identifier("::ns::subns3::object"), Identifier("subns4::object"))));
            }
            db.Add(ObjectVisibility::Private, Identifier("::ns::subns4::object"), d4.get());
            {
                refptr<IPrimitiveData> data;
                data = new PrimitiveData<u32 >(4);
                bool ok = d4->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                db.AddDeferredProperty(Identifier("::ns::subns4::object"), prop_object, new PrimitiveData<ObjectReference>(ObjectReference(&db, Identifier("::ns::subns4::object"), Identifier("subns::object"))));
            }
            db.EndTransaction();
        }
        {
            db.BeginTransaction();
            db.Add(ObjectVisibility::Protected, Identifier("::subns::object"), d5.get());
            {
                refptr<IPrimitiveData> data;
                data = new PrimitiveData<u32 >(5);
                bool ok = d5->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                db.AddDeferredProperty(Identifier("::subns::object"), prop_object, new PrimitiveData<ObjectReference>(ObjectReference(&db, Identifier("::subns::object"), Identifier("::ns::subns2::object"))));
            }
            db.Add(ObjectVisibility::Protected, Identifier("::ns::subns6::object"), d6.get());
            {
                refptr<IPrimitiveData> data;
                data = new PrimitiveData<u32 >(6);
                bool ok = d6->SetPropertyROK("u", data.get());
                SG_ASSERT_AND_UNUSED(ok);
                db.AddDeferredProperty(Identifier("::ns::subns6::object"), prop_object, new PrimitiveData<ObjectReference>(ObjectReference(&db, Identifier("::ns::subns6::object"), Identifier("subns::object"))));
            }
            db.EndTransaction();
        }
        SG_ASSERT(pd1->m_object == d2);
        SG_ASSERT(pd2->m_object == d3);
        SG_ASSERT(pd3->m_object == nullptr);
        SG_ASSERT(pd4->m_object == d1);
        SG_ASSERT(pd5->m_object == d2);
        if(SG_CONSTANT_CONDITION(ObjectDatabase::enable_partial_reference_inter_transaction))
        {
            if(SG_CONSTANT_CONDITION(ObjectDatabase::prefer_intra_transaction_reference))
                SG_ASSERT(pd6->m_object == d5);
            else
                SG_ASSERT(pd6->m_object == d1);
        }
        else
            SG_ASSERT(pd6->m_object == d5);
    }

     // TODO: Enable possibility to derive from BaseClass and multiple struct with properties
#if 0
    {
        Metaclass const* mca = GetMetaclass(Identifier("::sg::reflectionTest::TestClassDerivedFromStruct_A"));
        SG_ASSERT(nullptr != mca);
        refptr<BaseClass> a = mca->CreateObject();
        a->EndCreationIFN(ObjectCreationContext());
        Metaclass const* mc = a->GetMetaclass();
        SG_ASSERT(a->GetMetaclass() == mca);

        {
            refptr<IPrimitiveData> data = new PrimitiveData<u32>(42);
            a->BeginModification();
            bool ok = a->SetPropertyROK("v", data.get());
            SG_ASSERT(ok);
            a->EndModification(ObjectModificationContext());
            refptr<IPrimitiveData> dataget;
            a->GetPropertyIFP("v", &dataget);
            SG_ASSERT(nullptr != dataget);
            SG_ASSERT(dataget->As<u32>() == 42);
        }
        {
            refptr<IPrimitiveData> data = new PrimitiveData<u32>(42);
            a->BeginModification();
            bool ok = a->SetPropertyROK("u", data.get());
            SG_ASSERT(ok);
            a->EndModification(ObjectModificationContext());
            refptr<IPrimitiveData> dataget;
            a->GetPropertyIFP("u", &dataget);
            SG_ASSERT(nullptr != dataget);
            SG_ASSERT(dataget->As<u32>() == 42);
        }
        reflectionTest::TestClass_A* pa = checked_cast<reflectionTest::TestClass_A*>(a.get());
        a = nullptr;
    }
#endif

    ShutdownMetaclasses();
    IdentifierSymbol::Shutdown();
}
//=============================================================================
}
}

#endif

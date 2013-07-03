#include "stdafx.h"

#include "Tool.h"

#if SG_ENABLE_TOOLS

#include "Cast.h"
#include "TestFramework.h"

namespace sg {
//=============================================================================
namespace {
template <typename T>
T GetEditableValueImpl(char const* iName, char const* iDoc, T iDefault, T iMin, T iMax, T iStep)
{
    using namespace tools;
    Toolbox* toolbox = Toolbox::GetIFP();
    if(nullptr != toolbox)
    {
        ITool* tool = toolbox->GetToolIFP(iName);
        if(nullptr == tool)
        {
            EditableValue<T>* editableValue = new EditableValue<T>(ForwardString(iName), ForwardString(iDoc), iDefault, iMin, iMax, iStep);
            toolbox->RegisterTool(editableValue);
            return editableValue->Value();
        }
        SG_ASSERT_MSG(tool->GetType() == EditableValueTraits<T>::tool_type, "A tool with same name already exists!");
        if(tool->GetType() != EditableValueTraits<T>::tool_type)
            return iDefault;
        EditableValue<T> const* editableValue = checked_cast<EditableValue<T> const*>(tool);
        return editableValue->Value();
    }
    else
        return iDefault;
}
}
//=============================================================================
bool GetEditableValue(char const* iName, char const* iDoc, bool iDefault)
{
    using namespace tools;
    Toolbox* toolbox = Toolbox::GetIFP();
    if(nullptr != toolbox)
    {
        ITool* tool = toolbox->GetToolIFP(iName);
        if(nullptr == tool)
        {
            EditableValueBool* editableValue = new EditableValueBool(ForwardString(iName), ForwardString(iDoc), iDefault);
            toolbox->RegisterTool(editableValue);
            return editableValue->Value();
        }
        SG_ASSERT_MSG(tool->GetType() == ToolType::EditableValue_bool, "A tool with same name already exists!");
        if(tool->GetType() != ToolType::EditableValue_bool)
            return iDefault;
        EditableValueBool const* editableValue = checked_cast<EditableValueBool const*>(tool);
        return editableValue->Value();
    }
    else
        return iDefault;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int GetEditableValue(char const* iName, char const* iDoc, int iDefault, int iMin, int iMax, int iStep)
{
    return GetEditableValueImpl(iName, iDoc, iDefault, iMin, iMax, iStep);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t GetEditableValue(char const* iName, char const* iDoc, size_t iDefault, size_t iMin, size_t iMax, size_t iStep)
{
    return GetEditableValueImpl(iName, iDoc, iDefault, iMin, iMax, iStep);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float GetEditableValue(char const* iName, char const* iDoc, float iDefault, float iMin, float iMax, float iStep)
{
    return GetEditableValueImpl(iName, iDoc, iDefault, iMin, iMax, iStep);
}
//=============================================================================
}

namespace sg {
namespace tools {
//=============================================================================
void Init()
{
    SG_ASSERT(nullptr == Toolbox::GetIFP());
    //ToolNameNode::Init();
    new Toolbox();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IsInitialized()
{
    return nullptr != Toolbox::GetIFP();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    SG_ASSERT(nullptr != Toolbox::GetIFP());
    delete Toolbox::GetIFP();
    //ToolNameNode::Shutdown();
}
//=============================================================================
Toolbox::Toolbox()
    : m_tools()
    , m_toolFromName(0, Hash(this), Pred(this))
    , m_modificationStamp(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Toolbox::~Toolbox()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::RegisterTool(ITool* iTool)
{
    size_t const index = m_tools.size();
    m_tools.push_back(iTool);
    auto r = m_toolFromName.emplace(Key(index SG_CODE_FOR_ASSERT(SG_COMMA iTool->Name().c_str())), index);
    SG_ASSERT_MSG(r.second, "a tool with same name is already registered!");
    ++m_modificationStamp;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::UnregisterTool(ITool* iTool)
{
    SG_UNUSED(iTool);
    SG_ASSERT_NOT_IMPLEMENTED();
    ++m_modificationStamp;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ITool* Toolbox::GetToolIFP(char const* iName)
{
    auto f = m_toolFromName.find(Key(iName));
    if(m_toolFromName.end() == f)
        return nullptr;
    size_t const index = f->second;
    SG_ASSERT(index < m_tools.size());
    return m_tools[index].get();
}
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
#if !SG_ENABLE_TOOLS
#error
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace testtools {
namespace {
    bool GetValue1() { return GetEditableValue("core/test/value1", "just a test", false); }
    bool GetValue2() { return GetEditableValue("core/test/value2", "just another test", true); }
    int GetValue3() { return GetEditableValue("core/test/value3", "", 0, -10, 10, 2); }
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg,tools), Tools, (Core, quick))
{
    using namespace testtools;
    {
        bool const value1 = GetValue1();
        SG_ASSERT_AND_UNUSED(false == value1);
        bool const value2 = GetValue2();
        SG_ASSERT_AND_UNUSED(true == value2);
        int const value3 = GetValue3();
        SG_ASSERT_AND_UNUSED(0 == value3);
    }

    Init();

    {
        Toolbox* toolbox = Toolbox::GetIFP();
        SG_ASSERT(nullptr != toolbox);
        SG_ASSERT(nullptr == toolbox->GetToolIFP("core/test/value1"));

        bool const a = GetValue1();
        SG_ASSERT_AND_UNUSED(false == a);
        bool const b = GetValue2();
        SG_ASSERT_AND_UNUSED(true == b);
        int const c = GetValue3();
        SG_ASSERT_AND_UNUSED(0 == c);

        ITool* toolValue1 = toolbox->GetToolIFP("core/test/value1");
        SG_ASSERT(nullptr != toolValue1);
        SG_ASSERT(ToolType::EditableValue_bool == toolValue1->GetType());
        EditableValueBool* editableValue1 = checked_cast<EditableValueBool*>(toolValue1);
        editableValue1->SetValue(true);

        bool const d = GetValue1();
        SG_ASSERT_AND_UNUSED(true == d);

        ITool* toolValue3 = toolbox->GetToolIFP("core/test/value3");
        SG_ASSERT(nullptr != toolValue3);
        SG_ASSERT(ToolType::EditableValue_int == toolValue3->GetType());
        EditableValue<int>* editableValue3 = checked_cast<EditableValue<int>*>(toolValue3);
        editableValue3->SetValue(editableValue3->Value() + 2* editableValue3->Step());

        int const e = GetValue3();
        SG_ASSERT_AND_UNUSED(4 == e);
    }

    Shutdown();

    {
        bool const value1 = GetValue1();
        SG_ASSERT_AND_UNUSED(false == value1);
        bool const value2 = GetValue2();
        SG_ASSERT_AND_UNUSED(true == value2);
        int const value3 = GetValue3();
        SG_ASSERT_AND_UNUSED(0 == value3);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
#endif
//=============================================================================
}

#endif

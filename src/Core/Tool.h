#ifndef Core_Tool_H
#define Core_Tool_H

#include "Config.h"

namespace sg {
#if SG_ENABLE_TOOLS
// iName is a path separated by '/'
bool GetEditableValue(char const* iName, char const* iDoc, bool iDefault);
int GetEditableValue(char const* iName, char const* iDoc, int iDefault, int iMin, int iMax, int iStep);
size_t GetEditableValue(char const* iName, char const* iDoc, size_t iDefault, size_t iMin, size_t iMax, size_t iStep);
float GetEditableValue(char const* iName, char const* iDoc, float iDefault, float iMin, float iMax, float iStep);
#else
SG_FORCE_INLINE bool GetEditableValue(char const* iName, char const* iDoc, bool iDefault) { SG_UNUSED((iName, iDoc)); return iDefault; }
SG_FORCE_INLINE int GetEditableValue(char const* iName, char const* iDoc, int iDefault, int iMin, int iMax, int iStep) { SG_UNUSED((iName, iDoc, iMin, iMax, iStep)); return iDefault; }
SG_FORCE_INLINE size_t GetEditableValue(char const* iName, char const* iDoc, size_t iDefault, size_t iMin, size_t iMax, size_t iStep) { SG_UNUSED((iName, iDoc, iMin, iMax, iStep)); return iDefault; }
SG_FORCE_INLINE float GetEditableValue(char const* iName, char const* iDoc, float iDefault, float iMin, float iMax, float iStep) { SG_UNUSED((iName, iDoc, iMin, iMax, iStep)); return iDefault; }
#endif
}

#if SG_ENABLE_TOOLS

#include "ArrayView.h"
#include "FastSymbol.h"
#include "Observer.h"
#include "Singleton.h"
#include "SmartPtr.h"
#include <string>
#include <vector>
#include <unordered_set>

// - What is a tool ?
// It can take a lot of multiple forms, the more elaborated ones including
// complex UIs that allow to perform complex actions. However, the simplest
// ones provides the possibility to change the value of a parameter at runtime,
// or to launch a predefined action.
// Interaction with tools can also be of multiple forms. It can be through
// graphical user interfaces, keyboard shortcut, console, ini files, etc.
//
// The following provides a method to declare a tool, and to completely define
// the simplest ones (editable values, triggers). The more complex tools will
// be defined with other specialized tools (ie: UI). However, they can be
// integrated in the toolbox as launcher that open/close the corresponding UI.
//
// - Why is this toolbox useful ?
// Because it separates the tool from the way it is activated. It provides
// a way to list all the available tools and their means of action, and
// allowing parameterisation of UI (for instance, binding of shortcuts to tools).
//
// - What kind of tools can be expressed ?
// * a trigger, that notify its observers when activated.
//      can be interacted with as button, shortcut, etc.
//      ex: reload feature.
// * a boolean, used as an editable value or an on/off trigger.
//      can be interacted with as checkbox, shortcut, etc.
//      ex: open tool window.
// * an editable primitive value (int, float) with or without observers.
//      Constraints can be added: valid range, step, etc.
//      can be interacted with as UI cursor, 2 shortcuts for increase/decrease, etc.
//      ex: fov value.
// * an editable user value (color, etc.) with or without observers.
//      can be described as a set of primitive values (or not).
//      can be interacted with using user specified widgets, etc.
//      ex: open tool window.
// * a command line feature.
//      this is a method that takes argc, argv as arguments

namespace sg {
namespace tools {
//=============================================================================
class ToolBox;
//=============================================================================
void Init();
bool IsInitialized();
void Shutdown();
//=============================================================================
template <typename T> struct ForwardString_t {};
template <> struct ForwardString_t<char const*>        { SG_NO_COPY_OPERATOR(ForwardString_t) ForwardString_t(char const*        iValue) : value(iValue) {} ForwardString_t(ForwardString_t&& iOther) : value(iOther.value) {} char const* Value() const        { return value; }            private: char const* value; };
template <> struct ForwardString_t<std::string const&> { SG_NO_COPY_OPERATOR(ForwardString_t) ForwardString_t(std::string const& iValue) : value(iValue) {} ForwardString_t(ForwardString_t&& iOther) : value(iOther.value) {} std::string const& Value() const { return value; }            private: std::string const& value; };
template <> struct ForwardString_t<std::string&&>      { SG_NO_COPY_OPERATOR(ForwardString_t) ForwardString_t(std::string&&      iValue) : value(iValue) {} ForwardString_t(ForwardString_t&& iOther) : value(iOther.value) {} std::string&& Value() const      { return std::move(value); } private: std::string& value; };
template <typename T> ForwardString_t<T> ForwardString(T iString) { return ForwardString_t<T>(iString); }
//=============================================================================
enum class ToolType { Trigger, EditableValue_bool, EditableValue_int, EditableValue_size_t, EditableValue_float, Command };
//=============================================================================
class ITool : public RefAndSafeCountable
{
    SG_NON_COPYABLE(ITool)
public:
    template<typename T, typename U>
    ITool(ToolType iType, ForwardString_t<T> const& iName, ForwardString_t<U> const& iDocumentation) : m_type(iType),  m_name(iName.Value()), m_documentation(iDocumentation.Value()) {}
    virtual ~ITool() {}
    std::string const& Name() const { return m_name; }
    ToolType GetType() const { return m_type; }
private:
    ToolType const m_type;
    std::string const m_name;
    std::string const m_documentation;
};
//=============================================================================
class Trigger : public ITool, public Observable<Trigger>
{
    template<typename T, typename U>
    Trigger(ForwardString_t<T> const& iName, ForwardString_t<U> const& iDocumentation) : ITool(ToolType::Trigger, iName, iDocumentation) {}
};
//=============================================================================
class EditableValueBool : public ITool
                        , public Observable<EditableValueBool>
{
    PARENT_SAFE_COUNTABLE(ITool)
public:
    template<typename T, typename U>
    EditableValueBool(ForwardString_t<T> const& iName, ForwardString_t<U> const& iDocumentation, bool iDefault)
        : ITool(ToolType::EditableValue_bool, iName, iDocumentation)
        , m_default(iDefault)
        , m_value(iDefault)
    {}
    void SetValue(bool iValue) { if(iValue != m_value) { m_value = iValue; NotifyObservers(); } }
    bool Value() const { return m_value; }
private:
    bool const m_default;
    bool m_value;
};
//=============================================================================
template <typename T> class TypeTraits {};
template <> class TypeTraits<bool> { static const bool isBool = true; };
//=============================================================================
template <typename T> struct EditableValueTraits { static bool const is_supported_type = false; };
template <> struct EditableValueTraits<int>     { static bool const is_supported_type = true; static ToolType const tool_type = ToolType::EditableValue_int; };
template <> struct EditableValueTraits<size_t>  { static bool const is_supported_type = true; static ToolType const tool_type = ToolType::EditableValue_size_t; };
template <> struct EditableValueTraits<float>   { static bool const is_supported_type = true; static ToolType const tool_type = ToolType::EditableValue_float; };
//=============================================================================
template <typename T>
class EditableValue : public ITool
                    , public Observable<EditableValue<T>>
{
    PARENT_SAFE_COUNTABLE(ITool)
    static_assert(EditableValueTraits<T>::is_supported_type,
        "An editable value con only support one of the following types: int, size_t, float.");
public:
    template<typename U, typename V>
    EditableValue(ForwardString_t<U> const& iName, ForwardString_t<V> const& iDocumentation, T iDefault, T iMin, T iMax, T iStep)
        : ITool(EditableValueTraits<T>::tool_type, iName, iDocumentation)
        , m_default(iDefault)
        , m_min(iMin)
        , m_max(iMax)
        , m_step(iStep)
        , m_value(iDefault)
    {}
    void SetValue(T iValue)
    {
        SG_ASSERT(m_min <= iValue);
        SG_ASSERT(m_max >= iValue);
        if(iValue != m_value) { m_value = iValue; NotifyObservers(); }
    }
    T const& Value() const { return m_value; }
    T const& Min() const { return m_min; }
    T const& Max() const { return m_max; }
    T const& Step() const { return m_step; }
private:
    T const m_default;
    T const m_min;
    T const m_max;
    T const m_step;
    T m_value;
};
//=============================================================================
class Toolbox : public Singleton<Toolbox>
{
public:
    Toolbox();
    ~Toolbox();
    void RegisterTool(ITool* iTool);
    void UnregisterTool(ITool* iTool);
    ITool* GetToolIFP(char const* iName);
    ArrayView<refptr<ITool> const> Tools() { return ArrayView<refptr<ITool>>(m_tools.data(), m_tools.size()); }
    size_t const GetModificationStamp() const { return m_modificationStamp; }
private:
    struct Key
    {
        static_assert(sizeof(void*) == sizeof(size_t), "");
        explicit Key(size_t iIndex SG_CODE_FOR_ASSERT(SG_COMMA char const* iRefString))
            : indexOrString((iIndex << 1) + 1)
            SG_CODE_FOR_ASSERT(SG_COMMA refString(iRefString))
        {}
        explicit Key(char const* iString)
            : indexOrString(size_t(iString))
            SG_CODE_FOR_ASSERT(SG_COMMA refString(iString))
        {
            SG_ASSERT(0 == (size_t(iString) & 1));
        }
        size_t indexOrString; // 2 * index + 1 or char const*, depending on the lower bit.
        SG_CODE_FOR_ASSERT(std::string refString;)
    };
    struct Hash
    {
        Hash(Toolbox* iToolBox) : m_toolbox(iToolBox) {}
        size_t operator() (Key SG_CODE_FOR_ASSERT(const&) k) const
        {
            Toolbox* toolbox = m_toolbox.get();
            char const* s = nullptr;
            if(0 == (k.indexOrString & 1))
                s = (char const*)k.indexOrString;
            else
                s = toolbox->m_tools[k.indexOrString >> 1]->Name().c_str();
            SG_ASSERT(k.refString == s);
            size_t hash = 0;
            while(0 != *s)
            {
                size_t const lscount = sizeof(size_t)*8-7;
                hash = (hash << 7) | ( ( (hash >> lscount) & 0x7F ) ^ *s );
                ++s;
            }
            return hash;
        }
        safeptr<Toolbox> m_toolbox;
    };
    struct Pred
    {
        Pred(Toolbox* iToolBox) : m_toolbox(iToolBox) {}
        bool operator() (Key SG_CODE_FOR_ASSERT(const&) a, Key SG_CODE_FOR_ASSERT(const&) b) const
        {
            Toolbox* toolbox = m_toolbox.get();
            char const* sa = nullptr;
            char const* sb = nullptr;
            if(0 == (a.indexOrString & 1))
                sa = (char const*)a.indexOrString;
            else
                sa = toolbox->m_tools[a.indexOrString >> 1]->Name().c_str();
            if(0 == (b.indexOrString & 1))
                sb = (char const*)b.indexOrString;
            else
                sb = toolbox->m_tools[b.indexOrString >> 1]->Name().c_str();
            SG_ASSERT(a.refString == sa);
            SG_ASSERT(b.refString == sb);
            while(0 != *sa)
            {
                if(*sa != *sb)
                    return false;
                ++sa;
                ++sb;
            }
            return (*sa == *sb);
        }
        safeptr<Toolbox> m_toolbox;
    };
private:
    std::vector<refptr<ITool>> m_tools;
    std::unordered_map<Key, size_t, Hash, Pred> m_toolFromName;
    size_t m_modificationStamp;
};
//=============================================================================
}
}

#endif

#endif

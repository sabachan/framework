#ifndef Reflection_Identifier_H
#define Reflection_Identifier_H

#include <Core/Assert.h>
#include <Core/FastSymbol.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>
#include <string>

namespace sg {
namespace reflection {
//=============================================================================
class BaseClass;
class ObjectDatabase;
//=============================================================================
FAST_SYMBOL_TYPE_HEADER(IdentifierSymbol)
//=============================================================================
class IdentifierNode
{
public:
    IdentifierNode();
    IdentifierNode(IdentifierSymbol iSymbol);
    explicit IdentifierNode(char const* iSymbol);
    explicit IdentifierNode(std::string const& iSymbol);
    bool IsAnonymous() const { SG_ASSERT(m_symbol.IsValid() || 0 != m_anonymousIndex); return 0 != m_anonymousIndex; }
    IdentifierSymbol Symbol() const { SG_ASSERT(0 == m_anonymousIndex); return m_symbol; }
    u32 AnonymousIndex() const { SG_ASSERT(0 != m_anonymousIndex); return m_anonymousIndex; }
private:
    friend bool operator == (IdentifierNode const& a, IdentifierNode const& b);
    static u32 s_anonymousIndex;
    u32 m_anonymousIndex;
    IdentifierSymbol m_symbol;
};
bool operator == (IdentifierNode const& a, IdentifierNode const& b);
inline bool operator != (IdentifierNode const& a, IdentifierNode const& b) { return !(a == b); }
//=============================================================================
class Identifier : public SafeCountable
{
public:
    enum class Mode { Absolute, Relative };
    Identifier(Mode iMode = Mode::Absolute);
    Identifier(Identifier const& iIdentifier);
    Identifier(Identifier&& iIdentifier);
    Identifier(std::string const& iQualifiedName);
    Identifier(Identifier const& iNamespace, IdentifierNode const& iName);
    Identifier(Identifier&& iNamespace, IdentifierNode const& iName);
    Identifier(Identifier const& iNamespace, char const* iName);
    Identifier(Identifier&& iNamespace, char const* iName);
    Identifier(Identifier const& iNamespace, std::string const& iName);
    Identifier(Identifier&& iNamespace, std::string const& iName);
    Identifier const& operator=(Identifier const& iIdentifier);
    Identifier const& operator=(Identifier&& iIdentifier);
    bool IsAbsolute() const { return m_absolute; }
    size_t Size() const { return m_path.size(); }
    bool Empty() const { return 0 == m_path.size(); }
    IdentifierNode const& Back() const { return m_path.back(); }
    IdentifierNode const& operator[] (size_t i) const { return m_path[i]; }
    Identifier ParentNamespace() const;
    bool Contains(Identifier const& iId) const;
    void PushBack(IdentifierNode const& iName);
    void PushBack(std::string const& iName);
    void PushBack(char const* iName);
    std::string AsString() const;
private:
    bool m_absolute;
    std::vector<IdentifierNode> m_path;
};
bool operator == (Identifier const& a, Identifier const& b);
inline bool operator != (Identifier const& a, Identifier const& b) { return !(a == b); }
//=============================================================================
class ObjectReference
{
    friend bool operator == (ObjectReference const&, ObjectReference const&);
public:
    ObjectReference();
    ObjectReference(ObjectDatabase const* iDatabase, Identifier const& iCurrentNamespace, Identifier const& iName);
    ~ObjectReference();
    ObjectReference(ObjectReference const& iOther);
    ObjectReference const& operator=(ObjectReference const& iOther);
    bool GetROK(refptr<BaseClass>* oValue) const;
#if SG_ENABLE_TOOLS
    std::string AsString() { return m_name.AsString(); }
#endif
private:
    safeptr<ObjectDatabase const> m_database;
    Identifier m_currentNamespace;
    Identifier m_name;
};
bool operator == (ObjectReference const& a, ObjectReference const& b);
inline bool operator != (ObjectReference const& a, ObjectReference const& b) { return !(a == b); }
//=============================================================================
}
}

#endif

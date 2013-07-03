#include "stdafx.h"

#include "Identifier.h"

#include <Core/Assert.h>
#include <Core/Log.h>
#include "BaseClass.h"
#include "ObjectDatabase.h"
#include <sstream>


namespace sg {
namespace reflection {
//=============================================================================
FAST_SYMBOL_TYPE_IMPL(IdentifierSymbol)
//=============================================================================
u32 IdentifierNode::s_anonymousIndex = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IdentifierNode::IdentifierNode()
    : m_anonymousIndex(++s_anonymousIndex)
    , m_symbol()
{
    SG_ASSERT(0 != m_anonymousIndex);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IdentifierNode::IdentifierNode(IdentifierSymbol iSymbol)
    : m_anonymousIndex(0)
    , m_symbol(iSymbol)
{
    SG_ASSERT(m_symbol.IsValid());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IdentifierNode::IdentifierNode(char const* iName)
    : m_anonymousIndex(0)
    , m_symbol(iName)
{
    SG_ASSERT(nullptr == strstr(iName, "::"));
    SG_ASSERT(m_symbol.IsValid());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IdentifierNode::IdentifierNode(std::string const& iName)
    : m_anonymousIndex(0)
    , m_symbol(iName)
{
    SG_ASSERT(std::string::npos == iName.find("::"));
    SG_ASSERT(m_symbol.IsValid());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool operator == (IdentifierNode const& a, IdentifierNode const& b)
{
    if(a.IsAnonymous())
    {
        if(b.IsAnonymous()) { return a.m_anonymousIndex == b.m_anonymousIndex; }
        else { return false; }
    }
    else
    {
        if(b.IsAnonymous()) { return false; }
        else { return a.Symbol() == b.Symbol(); }
    }
}
//=============================================================================
Identifier::Identifier(Mode iMode)
    : m_absolute(Mode::Absolute == iMode)
    , m_path()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier const& iIdentifier)
    : m_absolute(iIdentifier.m_absolute)
    , m_path(iIdentifier.m_path)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier&& iIdentifier)
    : m_absolute(iIdentifier.m_absolute)
    , m_path()
{
    m_path.swap(iIdentifier.m_path);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(std::string const& iQualifiedName)
    : m_absolute()
    , m_path()
{
    size_t f = iQualifiedName.find("::", 0);
    size_t b = 0;
    if(0 == f)
    {
        m_absolute = true;
        SG_ASSERT(':' != iQualifiedName[f+2]);
        b = f+2;
        f = iQualifiedName.find("::", b);
    }
    while(std::string::npos != f)
    {
        m_path.push_back(IdentifierNode(iQualifiedName.substr(b, f-b)));
        SG_ASSERT(':' != iQualifiedName[f+2]);
        b = f+2;
        f = iQualifiedName.find("::", b);
    }
    SG_ASSERT(b < iQualifiedName.length());
    m_path.push_back(IdentifierNode(iQualifiedName.substr(b)));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier const& iNamespace, IdentifierNode const& iName)
    : m_absolute(iNamespace.m_absolute)
    , m_path(iNamespace.m_path)
{
    m_path.push_back(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier&& iNamespace, IdentifierNode const& iName)
    : m_absolute(iNamespace.m_absolute)
    , m_path()
{
    m_path.swap(iNamespace.m_path);
    m_path.push_back(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier const& iNamespace, char const* iName)
    : m_absolute(iNamespace.m_absolute)
    , m_path(iNamespace.m_path)
{
    m_path.push_back(IdentifierNode(iName));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier&& iNamespace, char const* iName)
    : m_absolute(iNamespace.m_absolute)
    , m_path()
{
    m_path.swap(iNamespace.m_path);
    m_path.push_back(IdentifierNode(iName));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier const& iNamespace, std::string const& iName)
    : m_absolute(iNamespace.m_absolute)
    , m_path(iNamespace.m_path)
{
    m_path.push_back(IdentifierNode(iName));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier::Identifier(Identifier&& iNamespace, std::string const& iName)
    : m_absolute(iNamespace.m_absolute)
    , m_path()
{
    m_path.swap(iNamespace.m_path);
    m_path.push_back(IdentifierNode(iName));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier const& Identifier::operator=(Identifier const& iIdentifier)
{
    m_absolute = iIdentifier.m_absolute;
    m_path = iIdentifier.m_path;
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier const& Identifier::operator=(Identifier&& iIdentifier)
{
    m_absolute = iIdentifier.m_absolute;
    m_path.swap(iIdentifier.m_path);
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Identifier Identifier::ParentNamespace() const
{
    Identifier ns;
    SG_ASSERT(!m_path.empty());
    size_t const outputSize = m_path.size()-1;
    ns.m_path.reserve(outputSize);
    for(size_t i = 0; i < outputSize; ++i)
    {
        ns.m_path.push_back(m_path[i]);
    }
    return ns;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Identifier::Contains(Identifier const& iId) const
{
    size_t size = Size();
    if(iId.Size() < size)
        return false;
    for(size_t i = 0; i < size; ++i)
    {
        if(iId.m_path[i] != m_path[i])
            return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Identifier::PushBack(IdentifierNode const& iName)
{
    m_path.push_back(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Identifier::PushBack(std::string const& iName)
{
    m_path.push_back(IdentifierNode(iName));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Identifier::PushBack(char const* iName)
{
    m_path.push_back(IdentifierNode(iName));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string Identifier::AsString() const
{
    std::ostringstream oss;
    if(IsAbsolute())
        oss << "::";
    size_t size = Size();
    for(size_t i = 0; i < size; ++i)
    {
        if(i != 0)
            oss << "::";
        if(m_path[i].IsAnonymous())
            oss << "(anonymous)";
        else
            oss << m_path[i].Symbol().Value();
    }
    return oss.str();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool operator == (Identifier const& a, Identifier const& b)
{
    size_t const size = a.Size();
    if(size != b.Size())
        return false;
    for(size_t i = 0; i < size; ++i)
    {
        if(a[i] != b[i])
            return false;
    }
    return true;
}
//=============================================================================
ObjectReference::ObjectReference()
    : m_database()
    , m_currentNamespace()
    , m_name()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectReference::ObjectReference(ObjectDatabase const* iDatabase, Identifier const& iCurrentNamespace, Identifier const& iName)
    : m_database(iDatabase)
    , m_currentNamespace(iCurrentNamespace)
    , m_name(iName)
{
    SG_ASSERT(nullptr != m_database);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectReference::~ObjectReference()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectReference::ObjectReference(ObjectReference const& iOther)
    : m_database(iOther.m_database)
    , m_currentNamespace(iOther.m_currentNamespace)
    , m_name(iOther.m_name)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectReference const& ObjectReference::operator=(ObjectReference const& iOther)
{
    m_database = iOther.m_database;
    m_currentNamespace = iOther.m_currentNamespace;
    m_name = iOther.m_name;
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ObjectReference::GetROK(refptr<BaseClass>* oValue) const
{
    SG_ASSERT(nullptr != oValue);
    SG_ASSERT(nullptr != m_database);
    *oValue = m_database->Get(m_currentNamespace, m_name);
    return nullptr != oValue;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool operator == (ObjectReference const& a, ObjectReference const& b)
{
    if(a.m_name != b.m_name) return false;
    if(a.m_currentNamespace != b.m_currentNamespace) return false;
    if(a.m_database != b.m_database) return false;
    return true;
}
//=============================================================================
}
}



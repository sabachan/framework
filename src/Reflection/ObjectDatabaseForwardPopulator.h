#ifndef Reflection_ObjectDatabaseForwardPopulator_H
#define Reflection_ObjectDatabaseForwardPopulator_H

#include <Core/SmartPtr.h>
#include "ObjectDatabase.h"

namespace sg {
namespace reflection {
//=============================================================================
class BaseClass;
class ObjectDatabase;
//=============================================================================
// Helper class to help writing parser code for object description files.
class ObjectDatabaseForwardPopulator : public SafeCountable
{
public:
    ObjectDatabaseForwardPopulator(ObjectDatabase* iDatabase);
    ~ObjectDatabaseForwardPopulator();
    void BeginNamespace(char const* iName);
    void EndNamespace();
    void BeginObject(ObjectVisibility iVisibility, char const* iName, Identifier const& iType);
    void EndObject();
    void Property(char const* iName);
    void Value(bool iValue);
    void Value(int iValue);
    void Value(float iValue);
    void Value(char const* iValue);
    void Value(nullptr_t);
    void Value(Identifier const& iValue);
    void BeginList();
    void EndList();
    void BeginBloc();
    void EndBloc();
private:
    void BeginValue();
    void EndValue(IPrimitiveData* iData);
private:
    struct StackNode
    {
        enum class Type { Object, List, NamedList };
        Type type;
        refptr<BaseClass> object;
        refptr<IPrimitiveData> data;
        char const* propertyName;
    public:
        StackNode(BaseClass* iObject)
            : type(Type::Object)
            , object(iObject)
            , propertyName(nullptr)
        {
            SG_ASSERT(nullptr != object);
        }
        StackNode(Type iType)
            : type(iType)
            , object()
            , propertyName(nullptr)
        {
            SG_ASSERT(Type::Object != type);
        }
    };
    std::vector<StackNode> m_stack;
    Identifier m_path;
    safeptr<ObjectDatabase> m_database;
    bool m_propertyContainsIdentifier;
};
//=============================================================================
}
}

#endif

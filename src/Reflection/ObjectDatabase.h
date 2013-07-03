#ifndef Reflection_ObjectDatabase_H
#define Reflection_ObjectDatabase_H

#include <Core/Assert.h>
#include <Core/FastSymbol.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>
#include <string>
#include <vector>
#include "Identifier.h"

namespace sg {
namespace objectscript {
    class Writer;
}
}

namespace sg {
namespace reflection {
//=============================================================================
class BaseClass;
class IPrimitiveData;
class IProperty;
class ObjectDatabase;
//=============================================================================
enum class ObjectVisibility
{
    Private,   // visible only from same namespace and transaction.
    Protected, // visible only from same transaction. (default)
    Public,    // visible across transactions but not from client code.
    Export,    // visible by client code (through method Get).
};
//=============================================================================
class ObjectDatabase : public SafeCountable
{
    SG_NON_COPYABLE(ObjectDatabase)
    friend class objectscript::Writer;
public:
    static const bool enable_partial_reference_inter_transaction = true;
    static const bool prefer_intra_transaction_reference = false;
    typedef std::pair<Identifier, safeptr<BaseClass> > named_object;
    typedef std::vector<std::pair<Identifier, safeptr<BaseClass> > > named_object_list;
public:
    // * AllowForwardReference is used for database for objects, where objects can
    //      reference other objects that can be defined after them.
    //      In this case, transaction must be ended before beginning another one.
    // * BackwardReferenceOnly is used for script. It allows references only on
    //      previously defined objects (as in most of programming languages).
    //      Moreover, it is possible to begin a trasaction as another is already
    //      in progress to implement scoping. In this case, all scoped objects are
    //      released at the end of the scoped transaction.
    enum class ReferencingMode { AllowForwardReference, BackwardReferenceOnly };
    ObjectDatabase(ReferencingMode iReferencingMode = ReferencingMode::AllowForwardReference);
    ~ObjectDatabase();
    BaseClass const* Get(Identifier const& iObjectName) const;
    BaseClass const* GetIFP(Identifier const& iObjectName) const;
    bool IsIdentifierUsed(Identifier const& iObjectName) const;
    void BeginTransaction();
    void Add(ObjectVisibility iVisibility, Identifier const& iObjectName, BaseClass* iObject);
    void AddDeferredProperty(Identifier const& iObjectName, IProperty const* iProperty, IPrimitiveData const* iValue);
    BaseClass* Get(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const;
    BaseClass* GetIFP(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const;
    named_object GetWithIdentifierIFP(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const;
    void AbortTransaction();
    void EndTransaction();
    void PopTransation();
    void BeginScopedTransaction();
    void EndScopedTransaction();
    void GetExportedObjects(named_object_list& oObjects) const;
    void GetLastTransactionObjects(named_object_list& oObjects) const;
private:
    struct ObjectEntry
    {
        size_t transaction;
        ObjectVisibility visibility;
        Identifier id;
        refptr<BaseClass> object;
    public:
        ObjectEntry(size_t iTransaction, ObjectVisibility iVisibility, Identifier const& iId, BaseClass* iObject);
        ObjectEntry();
        ~ObjectEntry();
        ObjectEntry(ObjectEntry const& iOther);
        ObjectEntry const& operator=(ObjectEntry const& iOther);
    };
    ObjectEntry const* GetObjectEntryIFP(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const;
    ObjectEntry const* GetObjectEntryIFP(Identifier const& iObjectName) const;
    void ApplyDeferredProperties();
private:
    struct DeferredProperty
    {
        Identifier objectname;
        safeptr<IProperty const> property;
        refptr<IPrimitiveData const> value;
    public:
        DeferredProperty(Identifier const& iObjectname, IProperty const* iProperty, IPrimitiveData const* iValue);
    };
    std::vector<DeferredProperty> m_deferredProperties;
    std::vector<ObjectEntry> m_objects;
    std::unordered_multimap<IdentifierSymbol, size_t> m_lastSymbolToObjects;
    std::unordered_map<size_t, size_t> m_anonymousIndexToObjects;
    size_t m_transactionCount;
    size_t m_transactionBegin;
    ReferencingMode const m_referencingMode;
#if SG_ENABLE_ASSERT
    enum class State { ReadOnly, AddObjects, Link, PopTransaction };
    State m_state;
    size_t m_scopedTransactionCount;
#endif
#if SG_ENABLE_UNIT_TESTS
public:
    static void Test();
#endif
};
//=============================================================================
class ObjectDatabaseScopedTransaction
{
public:
    ObjectDatabaseScopedTransaction(ObjectDatabase* idb) : m_db(idb) { m_db->BeginScopedTransaction(); }
    ~ObjectDatabaseScopedTransaction() { m_db->EndScopedTransaction(); }
private:
    safeptr<ObjectDatabase> m_db;
};
//=============================================================================
}
}

#endif

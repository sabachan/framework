#include "stdafx.h"

#include "ObjectDatabase.h"

#include "BaseClass.h"
#include "Identifier.h"
#include <Core/Assert.h>
#include <Core/TestFramework.h>
#include <algorithm>


namespace sg {
namespace reflection {
//=============================================================================
ObjectDatabase::ObjectEntry::ObjectEntry(size_t iTransaction, ObjectVisibility iVisibility, Identifier const& iId, BaseClass* iObject)
    : transaction(iTransaction)
    , visibility(iVisibility)
    , id(iId)
    , object(iObject)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::ObjectEntry::ObjectEntry()
    : transaction(all_ones)
    , visibility(ObjectVisibility::Private)
    , id()
    , object()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::ObjectEntry::~ObjectEntry()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::ObjectEntry::ObjectEntry(ObjectEntry const& iOther)
    : transaction(iOther.transaction)
    , visibility(iOther.visibility)
    , id(iOther.id)
    , object(iOther.object)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::ObjectEntry const& ObjectDatabase::ObjectEntry::operator=(ObjectEntry const& iOther)
{
    transaction = iOther.transaction;
    visibility = iOther.visibility;
    id = iOther.id;
    object = iOther.object;
    return *this;
}
//=============================================================================
ObjectDatabase::ObjectDatabase(ReferencingMode iReferencingMode)
    : m_deferredProperties()
    , m_objects()
    , m_lastSymbolToObjects()
    , m_transactionCount(0)
    , m_transactionBegin(0)
    , m_referencingMode(iReferencingMode)
    SG_CODE_FOR_ASSERT(SG_COMMA m_state(State::ReadOnly))
    SG_CODE_FOR_ASSERT(SG_COMMA m_scopedTransactionCount(0))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::~ObjectDatabase()
{
    SG_ASSERT(m_state == State::ReadOnly);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseClass const* ObjectDatabase::Get(Identifier const& iId) const
{
    BaseClass const* const o = GetIFP(iId);
    SG_ASSERT(nullptr != o);
    return o;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseClass const* ObjectDatabase::GetIFP(Identifier const& iId) const
{
    SG_ASSERT(m_state == State::ReadOnly);
    ObjectEntry const* const objectEntry = GetObjectEntryIFP(iId);
    if(nullptr == objectEntry) { return nullptr; }
    if(ObjectVisibility::Export != objectEntry->visibility) { return nullptr; }
    SG_ASSERT(nullptr != objectEntry->object);
    return objectEntry->object.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ObjectDatabase::IsIdentifierUsed(Identifier const& iObjectName) const
{
    SG_ASSERT(iObjectName.IsAbsolute());
    return nullptr != GetObjectEntryIFP(iObjectName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::BeginTransaction()
{
    SG_ASSERT(m_state == State::ReadOnly);
    SG_CODE_FOR_ASSERT(m_state = State::AddObjects);
    SG_ASSERT(m_objects.size() == m_transactionBegin);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::AbortTransaction()
{
    SG_ASSERT(m_state == State::AddObjects || m_state == State::Linked);
    if(ReferencingMode::AllowForwardReference == m_referencingMode)
    {
        SG_CODE_FOR_ASSERT(m_state = State::Link);
        m_deferredProperties.clear();
    }
    else
    {
        SG_ASSERT(m_deferredProperties.empty());
    }
    size_t const objectCount = m_objects.size();
    for(size_t i = m_transactionBegin; i < objectCount; ++i)
    {
        ObjectEntry const& objectEntry = m_objects[i];
        objectEntry.object->AbortCreation();
    }
    m_transactionBegin = m_objects.size();
    ++m_transactionCount;
    SG_CODE_FOR_ASSERT(m_state = State::ReadOnly);
    PopTransation();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ArrayView<ObjectDatabase::DeferredProperty> ObjectDatabase::LinkTransactionReturnInvalidDeferredProperties()
{
    SG_ASSERT(m_state == State::AddObjects);
    ArrayView<DeferredProperty> invalidDeferredProperties;
    if(ReferencingMode::AllowForwardReference == m_referencingMode)
    {
        SG_CODE_FOR_ASSERT(m_state = State::Link);
        invalidDeferredProperties = ApplyDeferredPropertiesReturnInvalid();
    }
    else
    {
        SG_ASSERT(m_deferredProperties.empty());
    }
    SG_CODE_FOR_ASSERT(m_state = State::Linked);
    return invalidDeferredProperties;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::LinkTransaction()
{
    ArrayView<DeferredProperty> invalid = LinkTransactionReturnInvalidDeferredProperties();
    SG_ASSERT(invalid.empty());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::CheckTransaction()
{
    SG_ASSERT(m_state == State::Linked);
#if ENABLE_REFLECTION_PROPERTY_CHECK
    ObjectPropertyCheckContext context;
    size_t const objectCount = m_objects.size();
    for(size_t i = m_transactionBegin; i < objectCount; ++i)
    {
        ObjectEntry const& objectEntry = m_objects[i];
        objectEntry.object->CheckProperties(context);
    }
#endif
    SG_CODE_FOR_ASSERT(m_state = State::Checked);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::EndTransaction()
{
    SG_ASSERT(m_state == State::Checked);
    ObjectCreationContext context;
    size_t const objectCount = m_objects.size();
    for(size_t i = m_transactionBegin; i < objectCount; ++i)
    {
        ObjectEntry const& objectEntry = m_objects[i];
        objectEntry.object->EndCreationIFN(context);
        if(ObjectVisibility::Private == objectEntry.visibility || ObjectVisibility::Protected == objectEntry.visibility)
        {
            IdentifierNode const lastNode = objectEntry.id.Back();
            if(!lastNode.IsAnonymous())
            {
                auto begin_end = m_lastSymbolToObjects.equal_range(lastNode.Symbol());
                SG_ASSERT(begin_end.first != m_lastSymbolToObjects.end());
                for(auto it = begin_end.first; it != begin_end.second; ++it)
                {
                    if(&(m_objects[it->second]) == &objectEntry)
                    {
                        m_lastSymbolToObjects.erase(it);
                        break;
                    }
                }
            }
        }
    }
    m_transactionBegin = m_objects.size();
    ++m_transactionCount;
    SG_CODE_FOR_ASSERT(m_state = State::ReadOnly);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::PopTransation()
{
    SG_ASSERT(m_state == State::ReadOnly);
    SG_CODE_FOR_ASSERT(m_state = State::PopTransaction);
    --m_transactionCount;
    while(m_transactionBegin != 0)
    {
        ObjectEntry const& objectEntry = m_objects[m_transactionBegin-1];
        if(objectEntry.transaction != m_transactionCount)
            break;

        IdentifierNode const lastNode = objectEntry.id.Back();
        if(!lastNode.IsAnonymous())
        {
            auto begin_end = m_lastSymbolToObjects.equal_range(lastNode.Symbol());
            SG_ASSERT(begin_end.first != m_lastSymbolToObjects.end());
            for(auto it = begin_end.first; it != begin_end.second; ++it)
            {
                if(&(m_objects[it->second]) == &objectEntry)
                {
                    m_lastSymbolToObjects.erase(it);
                    break;
                }
            }
        }

        --m_transactionBegin;
    }
    SG_ASSERT(m_objects.size() >= m_transactionBegin);
    m_objects.resize(m_transactionBegin);
    SG_CODE_FOR_ASSERT(m_state = State::ReadOnly);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::BeginScopedTransaction()
{
    SG_ASSERT(m_state == State::AddObjects);
    SG_ASSERT(ReferencingMode::BackwardReferenceOnly == m_referencingMode);
    ++m_transactionCount;
    SG_CODE_FOR_ASSERT(++m_scopedTransactionCount);
    m_transactionBegin = m_objects.size();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::EndScopedTransaction()
{
    SG_ASSERT(m_state == State::AddObjects);
    SG_ASSERT(ReferencingMode::BackwardReferenceOnly == m_referencingMode);
    SG_ASSERT(m_deferredProperties.empty());
    SG_ASSERT(0 < m_scopedTransactionCount);
#if ENABLE_REFLECTION_PROPERTY_CHECK
    ObjectPropertyCheckContext propertyCheckContext;
#endif
    ObjectCreationContext context;
    size_t const objectCount = m_objects.size();
    for(size_t i = m_transactionBegin; i < objectCount; ++i)
    {
        ObjectEntry const& objectEntry = m_objects[i];
        SG_ASSERT(objectEntry.transaction == m_transactionCount);
#if ENABLE_REFLECTION_PROPERTY_CHECK
        objectEntry.object->CheckProperties(propertyCheckContext);
#endif
        objectEntry.object->EndCreationIFN(context);

        IdentifierNode const lastNode = objectEntry.id.Back();
        SG_ASSERT(!lastNode.IsAnonymous());
        auto begin_end = m_lastSymbolToObjects.equal_range(lastNode.Symbol());
        SG_ASSERT(begin_end.first != m_lastSymbolToObjects.end());
        for(auto it = begin_end.first; it != begin_end.second; ++it)
        {
            if(&(m_objects[it->second]) == &objectEntry)
            {
                m_lastSymbolToObjects.erase(it);
                break;
            }
        }
    }

    SG_ASSERT(m_objects.size() >= m_transactionBegin);
    m_objects.resize(m_transactionBegin);

    --m_transactionCount;
    SG_CODE_FOR_ASSERT(--m_scopedTransactionCount);

    while(m_transactionBegin != 0)
    {
        if(m_objects[m_transactionBegin-1].transaction != m_transactionCount)
            break;
        --m_transactionBegin;
    }

    SG_CODE_FOR_ASSERT(m_state = State::AddObjects);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::ObjectEntry const* ObjectDatabase::GetObjectEntryIFP(Identifier const& iId) const
{
    SG_ASSERT(iId.IsAbsolute());
    size_t const iIdSize = iId.Size();
    IdentifierNode const lastNode = iId.Back();
    if(!lastNode.IsAnonymous())
    {
        auto begin_end = m_lastSymbolToObjects.equal_range(lastNode.Symbol());
        if(begin_end.first == m_lastSymbolToObjects.end())
            return nullptr;
        for(auto it = begin_end.first; it != begin_end.second; ++it)
        {
            auto const& objectEntry = m_objects[it->second];
            Identifier const& id = objectEntry.id;
            SG_ASSERT(id.IsAbsolute());
            if(id.Size() != iIdSize)
                continue;
            bool match = true;
            SG_ASSERT(id[iIdSize-1] == iId[iIdSize-1]);
            for(size_t i = 0; i < iIdSize-1; ++i)
            {
                if(id[i] != iId[i])
                {
                    match = false;
                    break;
                }
            }
            if(match)
                return &objectEntry;
        }
    }
    else
    {
        auto f = m_anonymousIndexToObjects.find(lastNode.AnonymousIndex());
        if(f == m_anonymousIndexToObjects.end())
            return nullptr;
        auto const& objectEntry = m_objects[f->second];
        SG_ASSERT(objectEntry.id == iId);
        return &objectEntry;
    }
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseClass* ObjectDatabase::Get(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const
{
    BaseClass* o = GetIFP(iCurrentNamespace, iObjectName);
    SG_ASSERT_MSG(nullptr != o, "Object not found !");
    return o;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseClass* ObjectDatabase::GetIFP(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const
{
    ObjectEntry const* entry = GetObjectEntryIFP(iCurrentNamespace, iObjectName);
    if(nullptr == entry)
        return nullptr;
    return entry->object.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::named_object ObjectDatabase::GetWithIdentifierIFP(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const
{
    ObjectEntry const* entry = GetObjectEntryIFP(iCurrentNamespace, iObjectName);
    if(nullptr == entry)
        return std::make_pair(Identifier(), nullptr);
    return std::make_pair(entry->id, entry->object.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::ObjectEntry const* ObjectDatabase::GetObjectEntryIFP(Identifier const& iCurrentNamespace, Identifier const& iObjectName) const
{
    SG_ASSERT(m_state == State::Link || m_referencingMode != ReferencingMode::AllowForwardReference);
    SG_ASSERT(iCurrentNamespace.IsAbsolute());
    SG_CODE_FOR_ASSERT(std::string DEBUG_iId = iObjectName.AsString();)
    if(iObjectName.IsAbsolute())
    {
        ObjectEntry const* const objectEntry = GetObjectEntryIFP(iObjectName);
        if(nullptr == objectEntry)
            return nullptr;
        switch(objectEntry->visibility)
        {
        case ObjectVisibility::Private:
            SG_ASSERT(objectEntry->transaction == m_transactionCount);
            SG_ASSERT(objectEntry->id.ParentNamespace().Contains(iCurrentNamespace));
            if(!(objectEntry->id.ParentNamespace().Contains(iCurrentNamespace)))
                return nullptr;
            break;
        case ObjectVisibility::Protected:
            SG_ASSERT(objectEntry->transaction == m_transactionCount);
            break;
        case ObjectVisibility::Public:
            break;
        case ObjectVisibility::Export:
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
        SG_ASSERT(nullptr != objectEntry->object);
        return objectEntry;
    }
    size_t const iIdSize = iObjectName.Size();
    size_t const namespaceSize = iCurrentNamespace.Size();
    bool matchingInTransaction = false;
    int bestPrefixMatchingSize = -1;
    size_t bestAllMatchingSize = iIdSize;
    size_t bestObjectIndex = all_ones;
    IdentifierNode const lastNode = iObjectName.Back();
    SG_ASSERT(!lastNode.IsAnonymous());
    auto begin_end = m_lastSymbolToObjects.equal_range(lastNode.Symbol());
    for(auto it = begin_end.first; it != begin_end.second; ++it)
    {
        ObjectEntry const& objectEntry = m_objects[it->second];
        if(SG_CONSTANT_CONDITION(!enable_partial_reference_inter_transaction))
        {
            if(objectEntry.transaction != m_transactionCount)
                continue;
        }
        Identifier const id = objectEntry.id;
        SG_ASSERT(id.IsAbsolute());
        size_t const idSize = id.Size();
        SG_ASSERT(id[idSize-1] == iObjectName[iIdSize-1]);
        if(idSize < iIdSize)
            continue;
        bool matchSuffix = true;
        for(size_t i = 2; i <= iIdSize; ++i)
        {
            SG_ASSERT(!iObjectName[iIdSize-i].IsAnonymous());
            if(id[idSize-i] != iObjectName[iIdSize-i])
            {
                matchSuffix = false;
                break;
            }
        }
        if(!matchSuffix)
            continue;
        size_t prefixMatchingSize = std::min(namespaceSize, idSize);
        for(size_t i = 0; i < prefixMatchingSize; ++i)
        {
            if(id[i] != iCurrentNamespace[i])
            {
                prefixMatchingSize = i;
                break;
            }
        }
        if(prefixMatchingSize + iIdSize < idSize)
            continue;
        switch(objectEntry.visibility)
        {
        case ObjectVisibility::Private:
            SG_ASSERT(objectEntry.transaction >= m_transactionCount - m_scopedTransactionCount);
            if(!(objectEntry.id.ParentNamespace().Contains(iCurrentNamespace)))
                continue;
            break;
        case ObjectVisibility::Protected:
            SG_ASSERT(objectEntry.transaction >= m_transactionCount - m_scopedTransactionCount);
            break;
        case ObjectVisibility::Public:
            break;
        case ObjectVisibility::Export:
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
        if(SG_CONSTANT_CONDITION(prefer_intra_transaction_reference))
        {
            if(!matchingInTransaction && objectEntry.transaction == m_transactionCount)
            {
                matchingInTransaction = true;
                bestPrefixMatchingSize = (int)prefixMatchingSize;
                bestAllMatchingSize = idSize;
                bestObjectIndex = it->second;
                continue;
            }
            else if(matchingInTransaction && objectEntry.transaction != m_transactionCount)
            {
                continue;
            }
        }
        if((int)prefixMatchingSize > bestPrefixMatchingSize)
        {
            bestPrefixMatchingSize = (int)prefixMatchingSize;
            bestAllMatchingSize = idSize;
            bestObjectIndex = it->second;
        }
        else if((int)idSize > bestAllMatchingSize)
        {
            bestAllMatchingSize = (int)idSize;
            bestObjectIndex = it->second;
        }
    }
    if(-1 != bestObjectIndex)
        return &(m_objects[bestObjectIndex]);
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::Add(ObjectVisibility iVisibility, Identifier const& iObjectName, BaseClass* iObject)
{
    SG_ASSERT(State::AddObjects == m_state);
    SG_ASSERT(iObjectName.IsAbsolute());
    SG_ASSERT(iObjectName.Back().IsAnonymous() || nullptr == GetObjectEntryIFP(iObjectName)); // TODO - Error message
    IdentifierNode const lastNode = iObjectName.Back();
    if(!lastNode.IsAnonymous())
    {
        m_lastSymbolToObjects.emplace(lastNode.Symbol(), m_objects.size());
    }
    else
    {
        auto r = m_anonymousIndexToObjects.emplace(lastNode.AnonymousIndex(), m_objects.size());
        SG_ASSERT_MSG(r.second, "anonymous index already in database");
    }
    m_objects.emplace_back(m_transactionCount, iVisibility, iObjectName, iObject);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::AddDeferredProperty(Identifier const& iObjectName, IProperty const* iProperty, IPrimitiveData const* iValue)
{
    SG_ASSERT(m_referencingMode == ReferencingMode::AllowForwardReference);
    SG_ASSERT(State::AddObjects == m_state);
    SG_ASSERT(iObjectName.IsAbsolute());
    m_deferredProperties.emplace_back(iObjectName, iProperty, iValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ArrayView<ObjectDatabase::DeferredProperty> ObjectDatabase::ApplyDeferredPropertiesReturnInvalid()
{
    SG_ASSERT(m_referencingMode == ReferencingMode::AllowForwardReference);
    SG_ASSERT(State::Link == m_state);
    refptr<IPrimitiveData> data;
    std::vector<DeferredProperty> m_invalidDeferredProperties;
    for(auto const& it : m_deferredProperties)
    {
        ObjectEntry const* const objectEntry = GetObjectEntryIFP(it.objectname);
        SG_ASSERT(nullptr != objectEntry);
        BaseClass* o = objectEntry->object.get();
        SG_ASSERT(nullptr != o);
        bool const ok = o->SetPropertyROK(it.property.get(), it.value.get());
        if(!ok)
            m_invalidDeferredProperties.emplace_back(it);
    }
    swap(m_deferredProperties, m_invalidDeferredProperties);
    return AsArrayView(m_deferredProperties);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::GetExportedObjects(named_object_list& oObjects) const
{
    for(auto const& it : m_objects)
    {
        if(ObjectVisibility::Export == it.visibility)
            oObjects.emplace_back(it.id, it.object.get());
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ObjectDatabase::GetLastTransactionObjects(named_object_list& oObjects) const
{
    oObjects.clear();
    size_t const transactionIndex = m_transactionCount - 1;
    size_t const end = m_objects.size();
    size_t b = 0;
    size_t e = end-1;
    if(m_objects[e].transaction != transactionIndex)
        return;
    while(b < e)
    {
        size_t const m = (b+e)/2;
        if(m_objects[m].transaction == transactionIndex)
            e = m;
        else
            b = m+1;
    }
    SG_ASSERT(m_objects[e].transaction == transactionIndex);
    for(size_t i = b; i < end; ++i)
    {
        ObjectEntry const& it = m_objects[i];
        SG_ASSERT(ObjectVisibility::Public == it.visibility);
        oObjects.emplace_back(it.id, it.object.get());
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ObjectDatabase::DeferredProperty::DeferredProperty(Identifier const& iObjectname,
                                                   IProperty const* iProperty,
                                                   IPrimitiveData const* iValue)
    : objectname(iObjectname)
    , property(iProperty)
    , value(iValue)
{ }
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
void ObjectDatabase::Test()
{
    IdentifierSymbol::Init();
    InitMetaclasses();

    {
        ObjectDatabase db;
        db.BeginTransaction();
        {
            refptr<BaseClass> a_A = new BaseClass;
            db.Add(ObjectVisibility::Export, Identifier("::A::Object"), a_A.get());
            refptr<BaseClass> a_AB = new BaseClass;
            db.Add(ObjectVisibility::Export, Identifier("::A::B::Object"), a_AB.get());
            refptr<BaseClass> a_ABA = new BaseClass;
            db.Add(ObjectVisibility::Export, Identifier("::A::B::A::Object"), a_ABA.get());
            refptr<BaseClass> a_B = new BaseClass;
            db.Add(ObjectVisibility::Export, Identifier("::B::Object"), a_B.get());
            refptr<BaseClass> a_BA = new BaseClass;
            db.Add(ObjectVisibility::Export, Identifier("::B::A::Object"), a_BA.get());
            refptr<BaseClass> a_BB = new BaseClass;
            db.Add(ObjectVisibility::Export, Identifier("::B::B::Object"), a_BB.get());

            SG_ASSERT(State::AddObjects == db.m_state);
            SG_CODE_FOR_ASSERT(db.m_state = State::Link);

            safeptr<BaseClass> o = nullptr;
            o = db.Get(Identifier(), Identifier("::A::B::A::Object"));
            SG_ASSERT(a_ABA.get() == o);
            o = db.Get(Identifier(), Identifier("A::Object"));
            SG_ASSERT(a_A.get() == o);
            o = db.Get(Identifier("::B"), Identifier("A::Object"));
            SG_ASSERT(a_BA.get() == o);
            o = db.Get(Identifier("::B"), Identifier("B::Object"));
            SG_ASSERT(a_BB.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("A::Object"));
            SG_ASSERT(a_ABA.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("B::Object"));
            SG_ASSERT(a_AB.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("B::A::Object"));
            SG_ASSERT(a_ABA.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("::B::A::Object"));
            SG_ASSERT(a_BA.get() == o);

            SG_ASSERT(State::Link == db.m_state);
            SG_CODE_FOR_ASSERT(db.m_state = State::AddObjects);
        }
        db.LinkCheckEndTransaction();
    }

    {
        ObjectDatabase db;

        refptr<BaseClass> a_A = new BaseClass;
        refptr<BaseClass> a_AB = new BaseClass;
        refptr<BaseClass> a_ABA = new BaseClass;
        refptr<BaseClass> a_ABB = new BaseClass;
        refptr<BaseClass> a_B = new BaseClass;
        refptr<BaseClass> a_BA = new BaseClass;
        refptr<BaseClass> a_BB = new BaseClass;
        refptr<BaseClass> a_CCC = new BaseClass;
        refptr<BaseClass> a_ABA_2 = new BaseClass;
        refptr<BaseClass> a_BA_2 = new BaseClass;
        refptr<BaseClass> a_C_2 = new BaseClass;

        db.BeginTransaction();
        {
            db.Add(ObjectVisibility::Export, Identifier("::A::Object"), a_A.get());
            db.Add(ObjectVisibility::Public, Identifier("::A::B::Object"), a_AB.get());
            db.Add(ObjectVisibility::Private, Identifier("::A::B::A::Object"), a_ABA.get());
            db.Add(ObjectVisibility::Protected, Identifier("::A::B::B::Object"), a_ABB.get());
            db.Add(ObjectVisibility::Public, Identifier("::B::Object"), a_B.get());
            db.Add(ObjectVisibility::Protected, Identifier("::B::A::Object"), a_BA.get());
            db.Add(ObjectVisibility::Protected, Identifier("::B::B::Object"), a_BB.get());
            db.Add(ObjectVisibility::Public, Identifier("::C::C::C::Object"), a_CCC.get());

            SG_ASSERT(State::AddObjects == db.m_state);
            SG_CODE_FOR_ASSERT(db.m_state = State::Link);

            safeptr<BaseClass> o = nullptr;
            // must fail: o = db.Get(Identifier(), Identifier("::A::B::A::Object"));
            o = db.Get(Identifier(), Identifier("A::Object"));
            SG_ASSERT(a_A.get() == o);
            o = db.Get(Identifier("::B"), Identifier("A::Object"));
            SG_ASSERT(a_BA.get() == o);
            o = db.Get(Identifier("::B"), Identifier("B::Object"));
            SG_ASSERT(a_BB.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("A::Object"));
            SG_ASSERT(a_A.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("B::Object"));
            SG_ASSERT(a_ABB.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("B::A::Object"));
            SG_ASSERT(a_BA.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("B::B::Object"));
            SG_ASSERT(a_ABB.get() == o);
            o = db.Get(Identifier("::A::B"), Identifier("::B::A::Object"));
            SG_ASSERT(a_BA.get() == o);

            SG_ASSERT(State::Link == db.m_state);
            SG_CODE_FOR_ASSERT(db.m_state = State::AddObjects);
        }
        db.LinkCheckEndTransaction();
        db.BeginTransaction();
        {
            // must fail: db.Add(ObjectVisibility::Export, Identifier("::A::Object"), new BaseClass);
            // must fail: db.Add(ObjectVisibility::Public, Identifier("::A::B::Object"), new BaseClass);
            db.Add(ObjectVisibility::Export, Identifier("::A::B::A::Object"), a_ABA_2.get());
            db.Add(ObjectVisibility::Export, Identifier("::B::A::Object"), a_BA_2.get());
            db.Add(ObjectVisibility::Protected, Identifier("::C::Object"), a_C_2.get());

            SG_ASSERT(State::AddObjects == db.m_state);
            SG_CODE_FOR_ASSERT(db.m_state = State::Link);

            safeptr<BaseClass> o = nullptr;
            if(SG_CONSTANT_CONDITION(ObjectDatabase::enable_partial_reference_inter_transaction))
            {
                // o = db.Get(Identifier(), Identifier("::A::B::A::Object"));
                o = db.Get(Identifier(), Identifier("A::Object"));
                SG_ASSERT(a_A.get() == o);
                o = db.Get(Identifier("::B"), Identifier("A::Object"));
                SG_ASSERT(a_BA_2.get() == o);
                o = db.Get(Identifier("::B"), Identifier("B::Object"));
                SG_ASSERT(a_B.get() == o);
                o = db.Get(Identifier("::A::B"), Identifier("A::Object"));
                SG_ASSERT(a_ABA_2.get() == o);
                o = db.Get(Identifier("::A::B"), Identifier("B::Object"));
                SG_ASSERT(a_AB.get() == o);
                o = db.Get(Identifier("::A::B"), Identifier("B::A::Object"));
                SG_ASSERT(a_ABA_2.get() == o);
                // must fail: o = db.Get(Identifier("::A::B"), Identifier("B::B::Object"));
                o = db.Get(Identifier("::A::B"), Identifier("::B::A::Object"));
                SG_ASSERT(a_BA_2.get() == o);
                o = db.Get(Identifier("::C::C"), Identifier("C::Object"));
                if(SG_CONSTANT_CONDITION(ObjectDatabase::prefer_intra_transaction_reference))
                    SG_ASSERT(a_C_2.get() == o);
                else
                    SG_ASSERT(a_CCC.get() == o);
                o = db.Get(Identifier("::C"), Identifier("C::C::Object"));
                SG_ASSERT(a_CCC.get() == o);
            }

            SG_ASSERT(State::Link == db.m_state);
            SG_CODE_FOR_ASSERT(db.m_state = State::AddObjects);
        }
        db.LinkCheckEndTransaction();
        db.PopTransation();
        db.PopTransation();
    }

    ShutdownMetaclasses();
    IdentifierSymbol::Shutdown();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg, reflection), ObjectDatabase, (Reflection, quick))
{
    ObjectDatabase::Test();
}
#endif
//=============================================================================
}
}



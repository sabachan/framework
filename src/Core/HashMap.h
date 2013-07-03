#ifndef Core_HashMap_H
#define Core_HashMap_H

#include "Allocators.h"
#include "ArrayView.h"
#include "Assert.h"
#include "Cast.h"
#include "For.h"
#include "Intrinsics.h"
#include "StridedArrayView.h"
#include <algorithm>
#include <functional>
#include <type_traits>

// The following hash map implementation follows some design principles:
// - There is no linked list, or bucket. When a collision occurs, we try next
//   positions in order to store the item. Having the item stored near to its
//   first try index is better for cache coherence.
// - Collisions with the same hash are always contiguous.
// - In order to benefit from the cache, we store the entry (a portion of the
//   hash and some bits for knowing if there is an element or if there is
//   already a collision) in a separate buffer from keys and values.

namespace sg {
//=============================================================================
namespace hashmap_internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename A>
struct HashMapAutomaticPolicy
{
    // keys_and_values_in_entry can be set to true when keys and values are
    // light objects with almost 0 move cost.
    static bool const keys_and_values_in_entry = false;
    // In case values or keys need specific alignment, it may be better to
    // separate buffers.
    static bool const separate_keys_from_values = false;
    // if keys and values are big and are not in entries, we can allocate 2
    // times more entries to reduce risks of collisions. In that case, a max
    // size allocator will be able to store more items.
    static bool const allocate_double_entries = true;
    static size_t const max_load_factor_size = 1;
    static size_t const max_load_factor_capacity = 2;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct HashMapDefaultPolicy
{
    static bool const keys_and_values_in_entry = false;
    static bool const separate_keys_from_values = false;
#if 1
    static bool const allocate_double_entries = true;
    static size_t const max_load_factor_size = 1;
    static size_t const max_load_factor_capacity = 2;
#else
    static bool const allocate_double_entries = false;
    static size_t const max_load_factor_size = 15;
    static size_t const max_load_factor_capacity = 16;
#endif
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
struct HashMapPool
{
    typedef typename std::conditional<sizeof(size_t) <= sizeof(T), size_t, unsigned int>::type index_type;
    static_assert(sizeof(index_type) <= sizeof(T), "");
public:
    HashMapPool() : m_freeIndex(0), m_endIndex() {}

    template<typename... Args>
    //TODO: Why does MSVC does not manage to inline this method in FastDebug?
    /*SG_FORCE_INLINE*/ std::pair<T*, size_t> Emplace_GetPtrAndIndex(void* ioBuffer, Args&&... iArgs)
    {
        index_type const index = m_freeIndex;
        void* ptr = reinterpret_cast<T*>(ioBuffer) + index;
        SG_ASSERT(index <= m_endIndex);
        m_freeIndex = index >= m_endIndex ? ++m_endIndex : *reinterpret_cast<index_type const*>(ptr);
        T* object = new(ptr) T(std::forward<Args>(iArgs)...);
        return std::make_pair(object, size_t(index));
    }
#if 0
    // Why does MSVC can inline this one?
    template<typename... Args>
    SG_FORCE_INLINE T* Emplace_GetPtrAndIndex2(void* ioBuffer, size_t& oIndex, Args&&... iArgs)
    {
        index_type const index = m_freeIndex;
        void* ptr = reinterpret_cast<T*>(ioBuffer) + index;
        SG_ASSERT(index <= m_endIndex);
        m_freeIndex = index >= m_endIndex ? ++m_endIndex : *reinterpret_cast<index_type const*>(ptr);
        T* object = new(ptr) T(std::forward<Args>(iArgs)...);
        oIndex = size_t(index);
        return object;
    }
#endif
    SG_FORCE_INLINE void Erase(void* ioBuffer, size_t iIndex)
    {
        SG_ASSERT(iIndex < m_endIndex);
        SG_ASSERT(iIndex != m_freeIndex);
        void* ptr = reinterpret_cast<T*>(ioBuffer) + iIndex;
        reinterpret_cast<T*>(ptr)->~T();
        *reinterpret_cast<index_type*>(ptr) = m_freeIndex;
        m_freeIndex = checked_numcastable(iIndex);
    }
    SG_FORCE_INLINE T* AtIndex(void* iBuffer, size_t iIndex)
    {
        SG_ASSERT(iIndex < m_endIndex);
        SG_ASSERT(iIndex != m_freeIndex);
        return reinterpret_cast<T*>(iBuffer) + iIndex;
    }
    SG_FORCE_INLINE void CopyFreeChain(void* oBuffer, void const* iBuffer) const
    {
        index_type index = m_freeIndex;
        while(index < m_endIndex)
        {
            void const* src = reinterpret_cast<T const*>(iBuffer) + index;
            void* dst = reinterpret_cast<T*>(oBuffer) + index;
            index = *reinterpret_cast<index_type const*>(src);
            *reinterpret_cast<index_type*>(dst) = index;
        }
        SG_ASSERT(index == m_endIndex);
    }
    SG_FORCE_INLINE void MoveItem(void* oBuffer, void* iBuffer, size_t iIndex) const
    {
        T* src = reinterpret_cast<T*>(iBuffer) + iIndex;
        T* dst = reinterpret_cast<T*>(oBuffer) + iIndex;
        Relocate_AssumeDisjoint(dst, src, 1);
    }

    index_type m_freeIndex;
    index_type m_endIndex;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T>
struct HashMapPackedItemType
{
    template<typename KLike, typename... Args>
    SG_FORCE_INLINE HashMapPackedItemType(KLike&& iKey, Args&&... iArgs) : key(std::forward<KLike>(iKey)), value(std::forward<Args>(iArgs)...) {}
    K key;
    T value;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K>
struct HashMapPackedItemType<K, void>
{
    K key;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename E, typename K>
struct HashMapKeyInEntryType
{
    E entry;
    K key;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename E>
struct HashMapDoubleEntryType
{
    E _0;
    E _1;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
enum class HashMapAllocationPolicy
{
    OneBuffer,
    SeparateEntryFromKeyValues,
    SeparateKeyInEntryFromValues,
    ThreeBuffers,
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename E, typename K, typename T, typename P, typename A, HashMapAllocationPolicy AllocationPolicy>
class HashMapAllocator
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename E, typename K, typename T, typename P, typename A>
class HashMapAllocator<E, K, T, P, A, HashMapAllocationPolicy::SeparateEntryFromKeyValues>
{
public:
    typedef E entry_type;
    typedef K key_type;
    typedef T value_type;
    typedef P policy_type;
    typedef HashMapPackedItemType<key_type, value_type> item_type;
    typedef HashMapPool<item_type> item_pool_type;
    typedef typename A::template Rebind<item_type>::type item_allocator_type;
    typedef typename std::conditional<policy_type::allocate_double_entries, HashMapDoubleEntryType<entry_type>, entry_type>::type type_for_entry_allocator;
    typedef typename A::template Rebind<type_for_entry_allocator>::type entry_allocator_type;

    struct Buffers
    {
    public:
        SG_FORCE_INLINE ArrayView<entry_type> ContiguousEntries() const { return AsArrayView(entries, entryCapacity); }
        SG_FORCE_INLINE StridedArrayView<entry_type> Entries() const { return StridedArrayView<entry_type>(entries, entryCapacity, sizeof(entry_type)); }
        SG_FORCE_INLINE size_t EntryCapacity() const { return entryCapacity; }
        SG_FORCE_INLINE StridedArrayView<key_type> Keys() const { return StridedArrayView<key_type>(&items->key, itemCapacity, sizeof(item_type)); }
        SG_FORCE_INLINE size_t KeyCapacity() const { return itemCapacity; }
        SG_FORCE_INLINE StridedArrayView<value_type> Values() const { return StridedArrayView<value_type>(&items->value, itemCapacity, sizeof(item_type)); }
        SG_FORCE_INLINE size_t ValueCapacity() const { return itemCapacity; }
        SG_FORCE_INLINE size_t ItemCapacity() const { return itemCapacity; }
    public:
        entry_type* entries;
        item_type* items;
        size_t entryCapacity;
        size_t itemCapacity;
        item_pool_type itemPool;
    };
public:
    SG_FORCE_INLINE static size_t KeyIndex(size_t iEntryIndex, size_t iItemIndex) { SG_UNUSED(iEntryIndex); return iItemIndex; }
    SG_FORCE_INLINE static size_t ValueIndex(size_t iEntryIndex, size_t iItemIndex) { SG_UNUSED(iEntryIndex); return iItemIndex; }
    SG_FORCE_INLINE Buffers AllocateAtLeast(size_t iEntryCapacity, size_t iItemCapacity)
    {
        Buffers r;
        if(SG_CONSTANT_CONDITION(policy_type::allocate_double_entries))
            iEntryCapacity >>= 1;
        r.entries = reinterpret_cast<entry_type*>(m_entryAllocator.AllocateAtLeast(iEntryCapacity, r.entryCapacity));
        if(SG_CONSTANT_CONDITION(policy_type::allocate_double_entries))
            r.entryCapacity <<= 1;
        r.items = reinterpret_cast<item_type*>(m_itemAllocator.AllocateAtLeast(iItemCapacity, r.itemCapacity));
        return r;
    }
    SG_FORCE_INLINE void Deallocate(Buffers& iBuffers)
    {
        size_t entryCapacity = iBuffers.entryCapacity;
        if(SG_CONSTANT_CONDITION(policy_type::allocate_double_entries))
            entryCapacity >>= 1;
        m_entryAllocator.Deallocate(iBuffers.entries, entryCapacity);
        m_itemAllocator.Deallocate(iBuffers.items, iBuffers.itemCapacity);
        iBuffers.entries = nullptr;
        iBuffers.items = nullptr;
        iBuffers.entryCapacity = 0;
        iBuffers.itemCapacity = 0;
    }
    template<typename KLike, typename... Args>
    SG_FORCE_INLINE std::pair<value_type*, size_t> Emplace_GetValuePtrAndIndex(Buffers& ioBuffers, size_t iEntryIndex, KLike&& iKey, Args&&... iArgs)
    {
        SG_UNUSED(iEntryIndex);
#if 1
        auto r = ioBuffers.itemPool.Emplace_GetPtrAndIndex(ioBuffers.items, iKey, iArgs...);
        value_type* value = &(r.first->value);
        return std::make_pair(value, r.second);
#else
        size_t index;
        item_type* ptr = ioBuffers.itemPool.Emplace_GetPtrAndIndex2(ioBuffers.items, index, iKey, iArgs...);
        value_type* value = &(ptr->value);
        return std::make_pair(value, index);
#endif
    }
    SG_FORCE_INLINE void EraseItem(Buffers& ioBuffers, size_t iEntryIndex, size_t iItemIndex)
    {
        SG_UNUSED(iEntryIndex);
        ioBuffers.itemPool.Erase(ioBuffers.items, iItemIndex);
    }
private:
    entry_allocator_type m_entryAllocator;
    item_allocator_type m_itemAllocator;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename E, typename K, typename T, typename P, typename A>
class HashMapAllocator<E, K, T, P, A, HashMapAllocationPolicy::SeparateKeyInEntryFromValues>
{
public:
    typedef E entry_type;
    typedef K key_type;
    typedef T value_type;
    typedef P policy_type;
    typedef HashMapKeyInEntryType<entry_type, key_type> keyinentry_type;
    typedef value_type item_type;
    typedef HashMapPool<item_type> item_pool_type;
    typedef typename A::template Rebind<item_type>::type item_allocator_type;
    typedef typename std::conditional<policy_type::allocate_double_entries, HashMapDoubleEntryType<keyinentry_type>, keyinentry_type>::type type_for_entry_allocator;
    typedef typename A::template Rebind<type_for_entry_allocator>::type entry_allocator_type;

    struct Buffers
    {
    public:
        SG_FORCE_INLINE ArrayView<keyinentry_type> ContiguousEntries() const { return AsArrayView(entries, entryCapacity); }
        SG_FORCE_INLINE StridedArrayView<entry_type> Entries() const { return StridedArrayView<entry_type>(&entries->entry, entryCapacity, sizeof(keyinentry_type)); }
        SG_FORCE_INLINE size_t EntryCapacity() const { return entryCapacity; }
        SG_FORCE_INLINE StridedArrayView<key_type> Keys() const { return StridedArrayView<key_type>(&entries->key, entryCapacity, sizeof(keyinentry_type)); }
        SG_FORCE_INLINE size_t KeyCapacity() const { return itemCapacity; }
        SG_FORCE_INLINE StridedArrayView<value_type> Values() const { return StridedArrayView<value_type>(items, itemCapacity, sizeof(value_type)); }
        SG_FORCE_INLINE size_t ValueCapacity() const { return itemCapacity; }
        SG_FORCE_INLINE size_t ItemCapacity() const { return itemCapacity; }
    public:
        keyinentry_type* entries;
        item_type* items;
        size_t entryCapacity;
        size_t itemCapacity;
        item_pool_type itemPool;
    };
public:
    SG_FORCE_INLINE static size_t KeyIndex(size_t iEntryIndex, size_t iItemIndex) { SG_UNUSED(iItemIndex); return iEntryIndex; }
    SG_FORCE_INLINE static size_t ValueIndex(size_t iEntryIndex, size_t iItemIndex) { SG_UNUSED(iEntryIndex); return iItemIndex; }
    SG_FORCE_INLINE Buffers AllocateAtLeast(size_t iEntryCapacity, size_t iItemCapacity)
    {
        Buffers r;
        if(SG_CONSTANT_CONDITION(policy_type::allocate_double_entries))
            iEntryCapacity >>= 1;
        r.entries = reinterpret_cast<keyinentry_type*>(m_entryAllocator.AllocateAtLeast(iEntryCapacity, r.entryCapacity));
        if(SG_CONSTANT_CONDITION(policy_type::allocate_double_entries))
            r.entryCapacity <<= 1;
        r.items = reinterpret_cast<item_type*>(m_itemAllocator.AllocateAtLeast(iItemCapacity, r.itemCapacity));
        return r;
    }
    SG_FORCE_INLINE void Deallocate(Buffers& iBuffers)
    {
        size_t entryCapacity = iBuffers.entryCapacity;
        if(SG_CONSTANT_CONDITION(policy_type::allocate_double_entries))
            entryCapacity >>= 1;
        m_entryAllocator.Deallocate(iBuffers.entries, entryCapacity);
        m_itemAllocator.Deallocate(iBuffers.items, iBuffers.itemCapacity);
        iBuffers.entries = nullptr;
        iBuffers.items = nullptr;
        iBuffers.entryCapacity = 0;
        iBuffers.itemCapacity = 0;
    }
    template<typename KLike, typename... Args>
    /*SG_FORCE_INLINE*/ std::pair<value_type*, size_t> Emplace_GetValuePtrAndIndex(Buffers& ioBuffers, size_t iEntryIndex, KLike&& iKey, Args&&... iArgs)
    {
        new(&(ioBuffers.entries[iEntryIndex].key)) key_type(iKey);
        auto r = ioBuffers.itemPool.Emplace_GetPtrAndIndex(ioBuffers.items, iArgs...);
        return r;
    }
    SG_FORCE_INLINE void EraseItem(Buffers& ioBuffers, size_t iEntryIndex, size_t iItemIndex)
    {
        ioBuffers.entries[iEntryIndex].key.~key_type();
        ioBuffers.itemPool.Erase(ioBuffers.items, iItemIndex);
    }
private:
    entry_allocator_type m_entryAllocator;
    item_allocator_type m_itemAllocator;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
class HashMapImpl
{
    typedef K key_type;
    typedef T value_type;
    typedef H hasher_type;
    typedef C comparer_type;
    typedef P policy_type;
    typedef size_t entry_type;
    typedef HashMapPackedItemType<key_type, value_type> item_type;

    typedef HashMapAllocator<entry_type, key_type, value_type, policy_type, A, HashMapAllocationPolicy::SeparateEntryFromKeyValues> allocator_type;
    //typedef HashMapAllocator<entry_type, key_type, value_type, policy_type, A, HashMapAllocationPolicy::SeparateKeyInEntryFromValues> allocator_type;
    typedef typename allocator_type::Buffers buffers_type;

    typedef typename ConstPassing<key_type>::type key_type_for_const_passing;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;

    template<typename K, typename T, typename H, typename C, typename P, typename A, bool IsConst>
    friend class HashMapIterator;
    typedef HashMapIterator<K, T, H, C, P, A, false> iterator_type;
    typedef HashMapIterator<K, T, H, C, P, A, true> const_iterator_type;
public:
    HashMapImpl() : m_buffers(), m_allocator(), m_size(0), m_entryConfig() SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp(0)) { GrowIfLoadFactorTooBig(); }

    HashMapImpl(HashMapImpl const& other);
    HashMapImpl(HashMapImpl&& other);
    HashMapImpl& operator=(HashMapImpl const& other);
    HashMapImpl& operator=(HashMapImpl&& other);
    ~HashMapImpl() { Clear(); m_allocator.Deallocate(m_buffers); }

    SG_FORCE_INLINE bool Empty() const { return 0 == m_size; }
    SG_FORCE_INLINE size_t Size() const { return m_size; }
    void Clear();
    void Rehash(size_t iCapacity);
    void Reserve(size_t iCapacity);

    template<typename KLike, typename... Args> std::pair<value_type*, bool> Emplace_AssumeCapacity(KLike&& iKey, Args&&... iArgs);
    template<typename KLike, typename... Args> std::pair<value_type*, bool> Emplace(KLike&& iKey, Args&&... iArgs) { GrowIfLoadFactorTooBig(); return Emplace_AssumeCapacity(iKey, iArgs...); }

    template<typename KLike> value_type* Find(KLike&& iKey) const;

    value_type& operator[] (key_type_for_const_passing iKey);
    value_type_for_const_passing operator[] (key_type_for_const_passing iKey) const;
    template<typename KLike> value_type& operator[] (KLike&& iKey);
    template<typename KLike> value_type_for_const_passing operator[] (KLike&& iKey) const;

    template<typename KLike> bool Erase(KLike&& iKey);

#if SG_CONTAINERS_EXPOSE_STD_NAMES
    bool empty() const { return Empty(); }
    size_t size() const { return Size(); }
    void clear() { Clear(); }
    void rehash(size_t iCapacity) { return Rehash(iCapacity); }
    void reserve(size_t iCapacity) { return Reserve(iCapacity); }
    template<typename KLike> iterator_type find(KLike&& iKey) const;

    template<typename KLike, typename... Args> std::pair<iterator_type, bool> emplace(KLike&& iKey, Args&&... iArgs);
    std::pair<iterator_type, bool> insert(std::pair<K, T> const& iKeyValue);
    iterator_type erase(iterator_type iIterator);
    iterator_type begin() { return iterator_type(this, 0 SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp)); }
    iterator_type end() { return iterator_type(this, m_entryConfig.entryIndexMask SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp)); }
    const_iterator_type begin() const { return const_iterator_type(this, 0 SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp)); }
    const_iterator_type end() const { return const_iterator_type(this, m_entryConfig.entryIndexMask SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp)); }
#endif
private:
    void GrowIfLoadFactorTooBig();
    void ChangeCapacity(size_t iEntryCapacity, size_t iItemCapacity);

    struct FindResult
    {
        size_t entryIndex;
        size_t itemIndex;
        bool isFound;
        bool isEntryUsed;
    };
    template<typename KLike> SG_FORCE_INLINE FindResult FindImpl(KLike&& iKey, size_t iHash) const;
    template<typename KLike, typename... Args> SG_FORCE_INLINE std::tuple<value_type*, size_t, bool> EmplaceImpl_AssumeCapacity(KLike&& iKey, Args&&... iArgs);
    SG_FORCE_INLINE void MoveContiguousEntriesForward(size_t entryIndex);
    SG_FORCE_INLINE void MoveContiguousEntriesBackward(size_t entryIndex);

    struct EntryConfig
    {
        EntryConfig() : entryIndexMask(0), hashCropMask(0), itemIndexShift(0) {}
        size_t entryIndexMask;
        size_t hashCropMask;
        size_t itemIndexShift;
        static size_t const minBitCountForCroppedHash = 6;
        static size_t const minCroppedHashMask = (size_t(1) << 6) - 1;
    };
    SG_FORCE_INLINE static EntryConfig EntryConfigFromCapacities(size_t iEntryCapacity, size_t iItemCapacity)
    {
        size_t const bitCountForEntryIndex = FloorLog2(iEntryCapacity);
        SG_ASSERT(0 < bitCountForEntryIndex);
        size_t const bitCountForItemIndex = std::max(size_t(1), CeilLog2(iItemCapacity));
        SG_ASSERT(0 < bitCountForItemIndex);
        SG_ASSERT(bitCountForItemIndex + 1 + EntryConfig::minBitCountForCroppedHash <= sizeof(entry_type) * 8);
        EntryConfig c;
        c.entryIndexMask = (size_t(1) << bitCountForEntryIndex) - 1;
        c.itemIndexShift = sizeof(entry_type) * 8 - bitCountForItemIndex;
        c.hashCropMask = size_t(all_ones) >> (bitCountForItemIndex + 1);
        return c;
    }
    SG_FORCE_INLINE static void ClearImpl(buffers_type& ioBuffers, EntryConfig const& iEntryConfig);
    SG_FORCE_INLINE static size_t CroppedHashFromEntry(EntryConfig iConfig, entry_type iEntry) { return size_t(iEntry) & iConfig.hashCropMask; }
    SG_FORCE_INLINE static size_t ItemIndexFromEntry(EntryConfig iConfig, entry_type iEntry) { return size_t(iEntry) >> iConfig.itemIndexShift; }
    SG_FORCE_INLINE static bool IsEntryUsed(EntryConfig iConfig, entry_type iEntry) { SG_ASSERT_AND_UNUSED(0 != (1 & (size_t(iEntry) >> (iConfig.itemIndexShift - 1))) || 0 == iEntry); return 0 != iEntry; }
    SG_FORCE_INLINE static entry_type EntryFromHashAndItemIndex(EntryConfig iConfig, size_t iHash, size_t iItemIndex) { return (iHash & iConfig.hashCropMask) | (iItemIndex << iConfig.itemIndexShift) | size_t(1) << (iConfig.itemIndexShift - 1); }
    SG_FORCE_INLINE static size_t BaseEntryIndexFromHash(EntryConfig iConfig, size_t iHash) { return iHash & iConfig.entryIndexMask; }
    SG_FORCE_INLINE static size_t BaseEntryIndexFromEntry(EntryConfig iConfig, entry_type iEntry, size_t iEntryIndex) { return (size_t(iEntry) & iConfig.hashCropMask & iConfig.entryIndexMask) | (iEntryIndex & ~iConfig.hashCropMask); }

    SG_FORCE_INLINE static bool CompareLessStrictBaseEntryIndex(EntryConfig iConfig, size_t iBaseEntryIndexA, size_t iBaseEntryIndexB)
    {
        bool const isSame = iBaseEntryIndexA == iBaseEntryIndexB;
        bool const isLessLarge = ((iBaseEntryIndexB - iBaseEntryIndexA) & iConfig.entryIndexMask) <= (iConfig.entryIndexMask >> 1);
        bool const isLess = !isSame && isLessLarge;
        return isLess;
    }

    SG_FORCE_INLINE static bool CompareLessStrictWithMask(size_t iMask, size_t a, size_t b)
    {
        size_t diff = (b - a) & iMask;
        bool const isSame = 0 == diff;
        bool const isLessLarge = diff <= (iMask >> 1);
        bool const isLess = !isSame && isLessLarge;
        return isLess;
    }

private:
    buffers_type m_buffers;
    allocator_type m_allocator;
    size_t m_size;
    EntryConfig m_entryConfig;
    SG_CODE_FOR_ASSERT(size_t m_iteratorValidityStamp;)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
void HashMapImpl<K, T, H, C, P, A>::Reserve(size_t iItemCapacity)
{
    size_t const entryCapacity = (iItemCapacity * policy_type::max_load_factor_capacity + policy_type::max_load_factor_size - 1) / policy_type::max_load_factor_size;
    size_t const prevItemCapacity = m_buffers.ItemCapacity();
    size_t const prevEntryCapacity = m_buffers.EntryCapacity();
    size_t const requestedEntryCapacity = GetSmallestPowerOf2GreaterThan(entryCapacity);
    size_t const requestedItemCapacity = (requestedEntryCapacity * policy_type::max_load_factor_size + policy_type::max_load_factor_capacity - 1) / policy_type::max_load_factor_capacity;
    SG_ASSERT(requestedItemCapacity >= iItemCapacity);
    if(prevEntryCapacity < requestedEntryCapacity)
         ChangeCapacity(
             std::max(size_t(policy_type::max_load_factor_capacity), requestedEntryCapacity),
             std::max(size_t(policy_type::max_load_factor_size), requestedItemCapacity));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
void HashMapImpl<K, T, H, C, P, A>::Rehash(size_t iEntryCapacity)
{
    size_t const prevEntryCapacity = m_buffers.EntryCapacity();
    size_t const requestedEntryCapacity = GetSmallestPowerOf2GreaterThan(iEntryCapacity);
    size_t const requestedItemCapacity = (requestedEntryCapacity * policy_type::max_load_factor_size + policy_type::max_load_factor_capacity - 1) / policy_type::max_load_factor_capacity;
    SG_ASSERT(requestedEntryCapacity >= iEntryCapacity);
    if(prevEntryCapacity < requestedEntryCapacity)
         ChangeCapacity(
             std::max(size_t(policy_type::max_load_factor_capacity), requestedEntryCapacity),
             std::max(size_t(policy_type::max_load_factor_size), requestedItemCapacity));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
void HashMapImpl<K, T, H, C, P, A>::GrowIfLoadFactorTooBig()
{
    size_t const entryCapacity = m_buffers.EntryCapacity();
    size_t const itemCapacity = m_buffers.KeyCapacity();
    SG_ASSERT(itemCapacity <= entryCapacity);
    SG_ASSERT(m_size <= itemCapacity);
    if((m_size + 1) * policy_type::max_load_factor_capacity >= entryCapacity * policy_type::max_load_factor_size)
         ChangeCapacity(
             std::max(size_t(policy_type::max_load_factor_capacity), entryCapacity * 2),
             std::max(size_t(policy_type::max_load_factor_size), itemCapacity * 2));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
void HashMapImpl<K, T, H, C, P, A>::ChangeCapacity(size_t iEntryCapacity, size_t iItemCapacity)
{
    SG_ASSERT(0 != iEntryCapacity);
    SG_ASSERT(0 == (iEntryCapacity & (iEntryCapacity - 1)));
    SG_ASSERT_MSG(iEntryCapacity < (size_t(1) << (sizeof(entry_type) * 8 - 6)), "Can't hold so many items");
    SG_CODE_FOR_ASSERT(++m_iteratorValidityStamp;)
    size_t const oldSize = m_size;
    EntryConfig oldEntryConfig = m_entryConfig;
    buffers_type oldBuffers = m_buffers;
    size_t const oldItemCapacity = oldBuffers.KeyCapacity();
    SG_ASSERT(oldItemCapacity == oldBuffers.ValueCapacity());

    buffers_type newBuffers = m_allocator.AllocateAtLeast(iEntryCapacity, iItemCapacity);
    auto newEntries = newBuffers.Entries();
    size_t const newEntryCapacity = newBuffers.EntryCapacity();
    size_t const newItemCapacity = newBuffers.ItemCapacity();
    EntryConfig const newEntryConfig = EntryConfigFromCapacities(newEntryCapacity, newItemCapacity);
    for(entry_type& it : newEntries)
        it = 0;
    m_buffers = newBuffers;
    m_size = 0;
    m_entryConfig = newEntryConfig;
    if(0 != oldSize)
    {
        auto oldEntries = oldBuffers.Entries();
        auto oldKeys = oldBuffers.Keys();
        auto oldValues = oldBuffers.Values();
        for_range(size_t, i, 0, oldEntryConfig.entryIndexMask + 1)
        {
            if(IsEntryUsed(oldEntryConfig, oldEntries[i]))
            {
                // TODO: no need to rehash as it is already in old entry (when grow).
                size_t const index = ItemIndexFromEntry(oldEntryConfig, oldEntries[i]);
                Emplace_AssumeCapacity(std::move(oldKeys[allocator_type::KeyIndex(i, index)]), std::move(oldValues[allocator_type::ValueIndex(i, index)]));
            }
        }
        ClearImpl(oldBuffers, oldEntryConfig);
    }
    m_allocator.Deallocate(oldBuffers);
    SG_ASSUME(oldSize == m_size);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
void HashMapImpl<K, T, H, C, P, A>::ClearImpl(buffers_type& ioBuffers, EntryConfig const& iEntryConfig)
{
    SG_ASSERT(0 != ioBuffers.ItemCapacity());
    SG_ASSERT(0 != ioBuffers.EntryCapacity());
    auto entries = ioBuffers.Entries();
    bool const needDestructValues = !std::is_trivially_destructible<value_type>::value;
    bool const needDestructKeys = !std::is_trivially_destructible<key_type>::value;
    static_assert(std::is_trivially_destructible<entry_type>::value, "");

    auto keys = ioBuffers.Keys();
    auto values = ioBuffers.Values();
    for_range(size_t, i, 0, iEntryConfig.entryIndexMask + 1)
    {
        entry_type& entry = entries[i];
        if(IsEntryUsed(iEntryConfig, entry))
        {
            size_t const index = ItemIndexFromEntry(iEntryConfig, entry);
            if(SG_CONSTANT_CONDITION(needDestructValues))
                values[allocator_type::ValueIndex(i, index)].~value_type();
            if(SG_CONSTANT_CONDITION(needDestructKeys))
                keys[allocator_type::KeyIndex(i, index)].~key_type();
            entry = 0;
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
void HashMapImpl<K, T, H, C, P, A>::Clear()
{
    ClearImpl(m_buffers, m_entryConfig);
    m_size = 0;
    SG_CODE_FOR_ASSERT(++m_iteratorValidityStamp;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike>
SG_FORCE_INLINE typename HashMapImpl<K, T, H, C, P, A>::FindResult HashMapImpl<K, T, H, C, P, A>::FindImpl(KLike&& iKey, size_t iHash) const
{
    SG_ASSERT(0 != m_buffers.ItemCapacity());
    SG_ASSERT(0 != m_buffers.EntryCapacity());
    EntryConfig const entryConfig = m_entryConfig;
    auto entries = m_buffers.Entries();
    auto keys = m_buffers.Keys();
    comparer_type const comparer;
    SG_ASSERT(hasher_type()(iKey) == iHash);
    size_t const newHash = iHash;
    size_t const newCroppedHash = newHash & entryConfig.hashCropMask;
    size_t const newBaseEntryIndex = newHash & entryConfig.entryIndexMask;
    entry_type entry = entries[newBaseEntryIndex];

    FindResult r;
    r.entryIndex = newBaseEntryIndex;
    r.itemIndex = all_ones;
    r.isFound = false;
    r.isEntryUsed = true;

    for(;;)
    {
        if(!IsEntryUsed(m_entryConfig, entry))
        {
            r.isEntryUsed = false;
            break;
        }
        size_t const croppedHash = CroppedHashFromEntry(entryConfig, entry);
        if(newCroppedHash == croppedHash)
        {
            size_t const itemIndex = ItemIndexFromEntry(entryConfig, entry);
            key_type const& key = keys[allocator_type::KeyIndex(r.entryIndex, itemIndex)];
            if(comparer(key, iKey))
            {
                r.itemIndex = itemIndex;
                r.isFound = true;
                SG_ASSERT(r.isEntryUsed);
                break;
            }

            // hash collision (in fact, only on the tested portion)
            // try next slot
            SG_BREAKABLE_POS;
        }
        else
        {
            //size_t const baseEntryIndex = BaseEntryIndexFromEntry(entryConfig, entry, r.entryIndex);
            //bool isNewBefore = CompareLessStrictBaseEntryIndex(entryConfig, newBaseEntryIndex, baseEntryIndex);
            SG_ASSERT_MSG(!CompareLessStrictWithMask(EntryConfig::minCroppedHashMask & entryConfig.entryIndexMask, r.entryIndex, entry), "An entry should always be after its base index (large). Maybe the hashmap is too dense.");
            bool isNewBefore = CompareLessStrictWithMask(EntryConfig::minCroppedHashMask & entryConfig.entryIndexMask, newBaseEntryIndex, entry);

            if(isNewBefore)
            {
                r.isFound = false;
                SG_ASSERT(all_ones == r.itemIndex);
                SG_ASSERT(r.isEntryUsed);
                break;
            }

            // check following entries
            SG_BREAKABLE_POS;
        }

        r.entryIndex = (r.entryIndex+ 1) & entryConfig.entryIndexMask;
        entry = entries[r.entryIndex];
    }

    SG_ASSUME(m_entryConfig.entryIndexMask == entryConfig.entryIndexMask);
    SG_ASSUME(m_entryConfig.hashCropMask == entryConfig.hashCropMask);
    SG_ASSUME(m_entryConfig.itemIndexShift == entryConfig.itemIndexShift);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
SG_FORCE_INLINE void HashMapImpl<K, T, H, C, P, A>::MoveContiguousEntriesForward(size_t iEntryIndex)
{
    EntryConfig const entryConfig = m_entryConfig;
    auto entries = m_buffers.Entries();

    size_t const entryIndex = iEntryIndex;
    size_t endIndex = entryIndex;
    do
    {
        endIndex = (endIndex + 1) & entryConfig.entryIndexMask;
    } while(IsEntryUsed(entryConfig, entries[endIndex]));

    auto contiguousEntries = m_buffers.ContiguousEntries();
    auto ptrEntries = contiguousEntries.Data();
    if(endIndex < entryIndex)
    {
        if(0 != endIndex)
            Relocate_Backward(ptrEntries + 1, ptrEntries, endIndex);
        Relocate_AssumeDisjoint(ptrEntries, ptrEntries + entryConfig.entryIndexMask, 1);
        SG_ASSERT(entryIndex <= entryConfig.entryIndexMask);
        if(entryIndex < entryConfig.entryIndexMask)
            Relocate_Backward(ptrEntries + entryIndex + 1, ptrEntries + entryIndex, entryConfig.entryIndexMask - entryIndex);
    }
    else
    {
        Relocate_Backward(ptrEntries + entryIndex + 1, ptrEntries + entryIndex, endIndex - entryIndex);
    }

    SG_ASSUME(m_entryConfig.entryIndexMask == entryConfig.entryIndexMask);
    SG_ASSUME(m_entryConfig.hashCropMask == entryConfig.hashCropMask);
    SG_ASSUME(m_entryConfig.itemIndexShift == entryConfig.itemIndexShift);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
SG_FORCE_INLINE void HashMapImpl<K, T, H, C, P, A>::MoveContiguousEntriesBackward(size_t iEntryIndex)
{
    EntryConfig const entryConfig = m_entryConfig;
    auto entries = m_buffers.Entries();
    size_t const entryIndex = iEntryIndex;
    size_t nextIndex = (entryIndex + 1) & entryConfig.entryIndexMask;
    size_t lastIndex = entryIndex;
    size_t endIndex = nextIndex;
    size_t endBaseEntryIndex = BaseEntryIndexFromEntry(entryConfig, entries[endIndex], endIndex);
    while(IsEntryUsed(entryConfig, entries[endIndex]) && endBaseEntryIndex != endIndex)
    {
        SG_ASSERT(CompareLessStrictBaseEntryIndex(m_entryConfig, endBaseEntryIndex, endIndex));
        lastIndex = endIndex;
        endIndex = (endIndex + 1) & entryConfig.entryIndexMask;
        endBaseEntryIndex = BaseEntryIndexFromEntry(entryConfig, entries[endIndex], endIndex);
    }

    auto contiguousEntries = m_buffers.ContiguousEntries();
    auto ptrEntries = contiguousEntries.Data();
    if(lastIndex < entryIndex)
    {
        SG_ASSERT(entryIndex <= entryConfig.entryIndexMask);
        if(entryIndex + 1 < entryConfig.entryIndexMask)
            Relocate_Forward(ptrEntries + entryIndex, ptrEntries + entryIndex + 1, entryConfig.entryIndexMask - entryIndex - 1);
        Relocate_AssumeDisjoint(ptrEntries + entryConfig.entryIndexMask, ptrEntries, 1);
        if(0 != lastIndex)
            Relocate_Forward(ptrEntries, ptrEntries + 1, lastIndex);
    }
    else
    {
        Relocate_Forward(ptrEntries + entryIndex, ptrEntries + entryIndex + 1, lastIndex - entryIndex);
    }
    entries[lastIndex] = 0;
    SG_ASSUME(m_entryConfig.entryIndexMask == entryConfig.entryIndexMask);
    SG_ASSUME(m_entryConfig.hashCropMask == entryConfig.hashCropMask);
    SG_ASSUME(m_entryConfig.itemIndexShift == entryConfig.itemIndexShift);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike, typename... Args>
SG_FORCE_INLINE std::tuple<typename HashMapImpl<K, T, H, C, P, A>::value_type*, size_t, bool> HashMapImpl<K, T, H, C, P, A>::EmplaceImpl_AssumeCapacity(KLike&& iKey, Args&&... iArgs)
{
    EntryConfig const entryConfig = m_entryConfig;
    size_t const size = m_size;

    hasher_type const hasher;
    size_t const newHash = hasher(iKey);
    FindResult const f = FindImpl(iKey, newHash);

    if(f.isFound)
    {
        auto values = m_buffers.Values();
        SG_ASSERT(all_ones != f.itemIndex);
        value_type& value = values[allocator_type::ValueIndex(f.entryIndex, f.itemIndex)];
        return std::make_tuple(&value, f.entryIndex, false);
    }

    auto entries = m_buffers.Entries();
    if(f.isEntryUsed)
        MoveContiguousEntriesForward(f.entryIndex);

    auto r = m_allocator.Emplace_GetValuePtrAndIndex(m_buffers, f.entryIndex, std::forward<KLike>(iKey), std::forward<Args>(iArgs)...);
    size_t const itemIndex = r.second;
    entries[f.entryIndex] = EntryFromHashAndItemIndex(entryConfig, newHash, itemIndex);
    m_size = size + 1;
    SG_ASSUME(m_entryConfig.entryIndexMask == entryConfig.entryIndexMask);
    SG_ASSUME(m_entryConfig.hashCropMask == entryConfig.hashCropMask);
    SG_ASSUME(m_entryConfig.itemIndexShift == entryConfig.itemIndexShift);
    SG_CODE_FOR_ASSERT(++m_iteratorValidityStamp;)
    return std::make_tuple(r.first, f.entryIndex, true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike, typename... Args>
std::pair<typename HashMapImpl<K, T, H, C, P, A>::value_type*, bool> HashMapImpl<K, T, H, C, P, A>::Emplace_AssumeCapacity(KLike&& iKey, Args&&... iArgs)
{
    auto r = EmplaceImpl_AssumeCapacity(iKey, iArgs...);
    return std::make_pair(std::get<0>(r), std::get<2>(r));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike, typename... Args>
std::pair<typename HashMapImpl<K, T, H, C, P, A>::iterator_type, bool> HashMapImpl<K, T, H, C, P, A>::emplace(KLike&& iKey, Args&&... iArgs)
{
    GrowIfLoadFactorTooBig();
    auto r = EmplaceImpl_AssumeCapacity(iKey, iArgs...);
    return std::make_pair(iterator_type(this, std::get<1>(r) SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp)), std::get<2>(r));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
std::pair<typename HashMapImpl<K, T, H, C, P, A>::iterator_type, bool> HashMapImpl<K, T, H, C, P, A>::insert(std::pair<K, T> const& iKeyValue)
{
    GrowIfLoadFactorTooBig();
    auto r = EmplaceImpl_AssumeCapacity(iKeyValue.first, iKeyValue.second);
    return std::make_pair(iterator_type(this, std::get<1>(r) SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp)), std::get<2>(r));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike>
typename HashMapImpl<K, T, H, C, P, A>::value_type* HashMapImpl<K, T, H, C, P, A>::Find(KLike&& iKey) const
{
    hasher_type const hasher;
    size_t const hash = hasher(iKey);
    FindResult const f = FindImpl(iKey, hash);

    if(f.isFound)
    {
        SG_ASSERT(all_ones != f.itemIndex);
        auto values = m_buffers.Values();
        value_type& value = values[allocator_type::ValueIndex(f.entryIndex, f.itemIndex)];
        return &value;
    }
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike>
typename HashMapImpl<K, T, H, C, P, A>::iterator_type HashMapImpl<K, T, H, C, P, A>::find(KLike&& iKey) const
{
    hasher_type const hasher;
    size_t const hash = hasher(iKey);
    FindResult const f = FindImpl(iKey, hash);

    if(f.isFound)
    {
        SG_ASSERT(all_ones != f.itemIndex);
        return iterator_type(this, f.entryIndex SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp));
    }
    else
        return end();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
template<typename KLike>
bool HashMapImpl<K, T, H, C, P, A>::Erase(KLike&& iKey)
{
    hasher_type const hasher;
    size_t const newHash = hasher(iKey);
    FindResult const f = FindImpl(iKey, newHash);

    if(!f.isFound)
        return false;

    size_t const size = m_size;
    EntryConfig const entryConfig = m_entryConfig;

    SG_ASSERT(f.isEntryUsed);
    SG_ASSERT(all_ones != f.itemIndex);
    m_allocator.EraseItem(m_buffers, f.entryIndex, f.itemIndex);

    MoveContiguousEntriesBackward(f.entryIndex);

    m_size = size - 1;
    SG_ASSUME(m_entryConfig.entryIndexMask == entryConfig.entryIndexMask);
    SG_ASSUME(m_entryConfig.hashCropMask == entryConfig.hashCropMask);
    SG_ASSUME(m_entryConfig.itemIndexShift == entryConfig.itemIndexShift);
    SG_CODE_FOR_ASSERT(++m_iteratorValidityStamp;)
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A>
typename HashMapImpl<K, T, H, C, P, A>::iterator_type HashMapImpl<K, T, H, C, P, A>::erase(iterator_type iIterator)
{
    EntryConfig const entryConfig = m_entryConfig;
    size_t const size = m_size;

    auto entries = m_buffers.Entries();

    entry_type const entry = entries[iIterator.m_entryIndex];
    SG_ASSERT(IsEntryUsed(entryConfig, entry));
    size_t const itemIndex = ItemIndexFromEntry(entryConfig, entry);
    m_allocator.EraseItem(m_buffers, iIterator.m_entryIndex, itemIndex);

    MoveContiguousEntriesBackward(f.entryIndex);

    m_size = size - 1;
    SG_ASSUME(m_entryConfig.entryIndexMask == entryConfig.entryIndexMask);
    SG_ASSUME(m_entryConfig.hashCropMask == entryConfig.hashCropMask);
    SG_ASSUME(m_entryConfig.itemIndexShift == entryConfig.itemIndexShift);
    SG_CODE_FOR_ASSERT(++m_iteratorValidityStamp;)
    return iterator_type(this, f.entryIndex SG_CODE_FOR_ASSERT(SG_COMMA m_iteratorValidityStamp));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename K, typename T, typename H, typename C, typename P, typename A, bool IsConst>
class HashMapIterator
{
    typedef HashMapIterator this_type;

    typedef HashMapImpl<K, T, H, C, P, A> hashmap_type;
    typedef typename std::conditional<IsConst, hashmap_type const*, hashmap_type*>::type hashmapptr_type;
    typedef typename hashmap_type::allocator_type allocator_type;

    typedef typename hashmap_type::key_type key_type;
    typedef typename hashmap_type::value_type value_type;
    typedef key_type const& key_type_ref;
    typedef typename std::conditional<IsConst, value_type const, value_type>::type& value_type_ref;

    struct KeyValuePair
    {
        key_type_ref first;
        value_type_ref second;
        KeyValuePair(key_type_ref iFirst, value_type_ref iSecond) : first(iFirst), second(iSecond) {}
    };
    struct KeyValuePairAsPtr : private KeyValuePair
    {
        KeyValuePair& operator *() { return *static_cast<KeyValuePair*>(this); }
        KeyValuePair* operator ->() { return static_cast<KeyValuePair*>(this); }
        KeyValuePairAsPtr(key_type_ref iFirst, value_type_ref iSecond) : KeyValuePair(iFirst, iSecond) {}
    };

    typedef std::bidirectional_iterator_tag iterator_category;
public:
    SG_FORCE_INLINE HashMapIterator() : m_hashmap(nullptr), m_entryIndex(0) {}
    SG_FORCE_INLINE KeyValuePairAsPtr operator ->() const
    {
        SG_ASSERT(nullptr != m_hashmap);
        SG_ASSERT_MSG(m_validityStamp == m_hashmap->m_iteratorValidityStamp, "Iterator has been invalidated!");
        hashmap_type::EntryConfig const entryconfig = m_hashmap->m_entryConfig;
        SG_ASSERT(m_entryIndex <= m_hashmap->m_entryConfig.entryIndexMask);
        auto entries = m_hashmap->m_buffers.Entries();
        auto keys = m_hashmap->m_buffers.Keys();
        auto values = m_hashmap->m_buffers.Values();
        size_t const itemIndex = hashmap_type::ItemIndexFromEntry(entryconfig, entries[m_entryIndex]);
        return KeyValuePairAsPtr(keys[allocator_type::KeyIndex(m_entryIndex, itemIndex)], values[allocator_type::ValueIndex(m_entryIndex, itemIndex)]);
    }
    SG_FORCE_INLINE KeyValuePair operator *() const
    {
        return *(this->operator->());
    }
    SG_FORCE_INLINE key_type& Key() const
    {
        SG_ASSERT(nullptr != m_hashmap);
        SG_ASSERT_MSG(m_validityStamp == m_hashmap->m_iteratorValidityStamp, "Iterator has been invalidated!");
        hashmap_type::EntryConfig const entryConfig = m_hashmap->m_entryConfig;
        SG_ASSERT(m_entryIndex <= m_hashmap->m_entryConfig.entryIndexMask);
        auto entries = m_hashmap->m_buffers.Entries();
        auto keys = m_hashmap->m_buffers.Keys();
        size_t const itemIndex = hashmap_type::ItemIndexFromEntry(entryConfig, entries[m_entryIndex]);
        return keys[allocator_type::KeyIndex(m_entryIndex, itemIndex)];
    }
    SG_FORCE_INLINE value_type& Value() const
    {
        SG_ASSERT(nullptr != m_hashmap);
        SG_ASSERT_MSG(m_validityStamp == m_hashmap->m_iteratorValidityStamp, "Iterator has been invalidated!");
        hashmap_type::EntryConfig const entryConfig = m_hashmap->m_entryConfig;
        SG_ASSERT(m_entryIndex <= entryConfig.entryIndexMask);
        auto entries = m_hashmap->m_buffers.Entries();
        auto values = m_hashmap->m_buffers.Values();
        size_t const itemIndex = hashmap_type::ItemIndexFromEntry(entryConfig, entries[m_entryIndex]);
        return values[allocator_type::ValueIndex(m_entryIndex, itemIndex)];
    }
    SG_FORCE_INLINE this_type& operator++()
    {
        SG_ASSERT(nullptr != m_hashmap);
        SG_ASSERT_MSG(m_validityStamp == m_hashmap->m_iteratorValidityStamp, "Iterator has been invalidated!");
        hashmap_type::EntryConfig const entryConfig = m_hashmap->m_entryConfig;
        SG_ASSERT(m_entryIndex <= entryConfig.entryIndexMask);
        auto entries = m_hashmap->m_buffers.Entries();
        do {
            ++m_entryIndex;
        } while(m_entryIndex < entryConfig.entryIndexMask && !hashmap_type::IsEntryUsed(entryConfig, entries[m_entryIndex]));
        return *this;
    }
    SG_FORCE_INLINE this_type operator++(int) { auto r = *this; ++(*this); return r; }
    SG_FORCE_INLINE this_type& operator--()
    {
        SG_ASSERT(nullptr != m_hashmap);
        SG_ASSERT_MSG(m_validityStamp == m_hashmap->m_iteratorValidityStamp, "Iterator has been invalidated!");
        hashmap_type::EntryConfig const entryConfig = m_hashmap->m_entryConfig;
        SG_ASSERT(m_entryIndex <= entryConfig.entryIndexMask);
        auto entries = m_hashmap->m_buffers.Entries();
        do {
            --m_entryIndex;
        } while(m_entryIndex != 0 && !hashmap_type::IsEntryUsed(entryConfig, entries[m_entryIndex]));
        return *this;
    }
    SG_FORCE_INLINE this_type operator--(int) { auto r = *this; --(*this); return r; }
    friend SG_FORCE_INLINE bool operator==(this_type const& a, this_type const& b) { SG_ASSERT(a.m_hashmap == b.m_hashmap); return a.m_entryIndex == b.m_entryIndex; }
    friend SG_FORCE_INLINE bool operator!=(this_type const& a, this_type const& b) { SG_ASSERT(a.m_hashmap == b.m_hashmap); return a.m_entryIndex != b.m_entryIndex; }
private:
    template<typename K, typename T, typename H, typename C, typename P, typename A>
    friend class HashMapImpl;
    SG_FORCE_INLINE HashMapIterator(
        hashmapptr_type iHashMap,
        size_t iEntryIndex
        SG_CODE_FOR_ASSERT(SG_COMMA size_t iValidityStamp))
        : m_hashmap(iHashMap)
        , m_entryIndex(iEntryIndex)
        SG_CODE_FOR_ASSERT(SG_COMMA m_validityStamp(iValidityStamp))
    {
        SG_ASSERT(nullptr != m_hashmap);
        SG_ASSERT(m_validityStamp == m_hashmap->m_iteratorValidityStamp);
        hashmap_type::EntryConfig const entryConfig = m_hashmap->m_entryConfig;
        auto entries = m_hashmap->m_buffers.Entries();
        while(m_entryIndex < entryConfig.entryIndexMask && !hashmap_type::IsEntryUsed(entryConfig, entries[m_entryIndex]))
            ++m_entryIndex;
        SG_ASSERT(m_entryIndex == entryConfig.entryIndexMask || hashmap_type::IsEntryUsed(entryConfig, entries[m_entryIndex]));
    }
private:
    hashmapptr_type m_hashmap;
    size_t m_entryIndex;
    SG_CODE_FOR_ASSERT(size_t m_validityStamp;)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
//template<typename K, typename A = StandardAllocator<void>, bool IsCopyable = std::is_copy_constructible<T>::value >
//class HashSet : public internal::HashMapImpl<T, void, A>
//{
//public:
//    HashSet() {}
//    template <typename... Args> HashSet(size_t iCount, Args&&... iArgs) : ArrayListImpl(iCount, iArgs...) {}
//    HashSet(ArrayList const& other) = default;
//    HashSet& operator=(HashSet const& other) = default;
//    HashSet(ArrayList&& other) : ArrayListImpl(std::move(other)) { }
//    HashSet& operator=(HashSet&& other) { *static_cast<ArrayListImpl*>(this) = std::move(other); return *this; }
//};
//=============================================================================
template<typename K,
         typename T,
         typename Hasher = std::hash<K>,
         typename Comparer = std::equal_to<K>,
         typename P = hashmap_internal::HashMapDefaultPolicy,
         typename A = StandardAllocator<void>,
         bool IsCopyable = std::is_copy_constructible<T>::value>
class HashMap : public hashmap_internal::HashMapImpl<K, T, Hasher, Comparer, P, A>
{
public:
    HashMap() {}
    HashMap(HashMap const& other) = default;
    HashMap& operator=(HashMap const& other) = default;
    HashMap(HashMap&& other) : HashMapImpl(std::move(other)) { }
    HashMap& operator=(HashMap&& other) { *static_cast<HashMapImpl*>(this) = std::move(other); return *this; }
};
//=============================================================================
//template<typename K,
//         typename T,
//         typename Hasher = std::hash<K>,
//         typename Comparer = std::equal_to<K>,
//         typename P = internal::HashMapDefaultPolicy,
//         typename A = StandardAllocator<void>,
//         bool IsCopyable = std::is_copy_constructible<T>::value>
//class HashMap<K, T, Hasher, Comparer, P, A, false> : public internal::HashMapImpl<K, T, Hasher, Comparer, P, A>
//{
//public:
//    HashMap() {}
//    template <typename... Args> HashMap(size_t iCount, Args&&... iArgs) : HashMapImpl(iCount, iArgs...) {}
//    HashMap(HashMap const& other) = delete;
//    HashMap& operator=(HashMap const& other) = delete;
//    HashMap(HashMap&& other) : HashMapImpl(std::move(other)) { }
//    HashMap& operator=(HashMap&& other) { *static_cast<HashMapImpl*>(this) = std::move(other); return *this; }
//};
//=============================================================================
}

#endif

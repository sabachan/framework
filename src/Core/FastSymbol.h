#ifndef Core_FastSymbol_H
#define Core_FastSymbol_H

#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/SmartPtr.h>
#include <string>
#include <unordered_map>
#include <vector>

#define FAST_SYMBOL_TYPE_HEADER(NAME) \
    struct sg_fastsymbol_domain_##NAME \
    { \
    private: \
        friend class sg::fastsymbol::TemplateFastSymbol<sg_fastsymbol_domain_##NAME>; \
        static scopedptr<sg::fastsymbol::Database> s_database; \
    }; \
    typedef sg::fastsymbol::TemplateFastSymbol<sg_fastsymbol_domain_##NAME> NAME;

#define FAST_SYMBOL_TYPE_IMPL(NAME) \
    scopedptr<sg::fastsymbol::Database> sg_fastsymbol_domain_##NAME::s_database;

#define USE_FAST_SYMBOL_REF_IMPL 0

namespace sg {
namespace fastsymbol {
//=============================================================================
class Database : public SafeCountable
{
public:
    Database();
    ~Database();
    size_t GetIndex(char const* iValue);
    std::string const& GetValue(size_t iIndex) const;

#if USE_FAST_SYMBOL_REF_IMPL
private:
    // This is not performant. To improve, one should try to store only one copy
    // of each string, in a place where they would not be copied around too much
    // (not a std::vector), and the hash map key should be an index on the string,
    // or an offset so that it is stable even if strings are moved.
    // However, such proerties seems to imply our own hashmap implementation.
    std::unordered_map<std::string, size_t> m_map;
#else
private:
    struct Key
    {
        ptrdiff_t indexOrString; // 2 * index + 1 or char const*, depending on the lower bit.
    };
    struct Hash
    {
        Hash(Database* iDatabase) : m_database(iDatabase) {}
        size_t operator() (Key k) const
        {
            Database* db = m_database.get();
            SG_ASSERT(db->m_header == 0xBADBAFFE);

            char const* s = nullptr;
            if(0 == (k.indexOrString & 1))
                s = (char const*)k.indexOrString;
            else
                s = db->m_values[k.indexOrString >> 1].c_str();
            size_t hash = 0;

            while(0 != *s)
            {
                size_t const lscount = sizeof(size_t)*8-7;
                hash = (hash << 7) | ( ( (hash >> lscount) & 0x7F ) ^ *s );
                ++s;
            }
            return hash;
        }
        safeptr<Database> m_database;
    };
    struct Pred
    {
        Pred(Database* iDatabase) : m_database(iDatabase) {}
        bool operator() (Key a, Key b) const
        {
            Database* db = m_database.get();
            SG_ASSERT(db->m_header == 0xBADBAFFE);
            char const* sa = nullptr;
            char const* sb = nullptr;
            if(0 == (a.indexOrString & 1))
                sa = (char const*)a.indexOrString;
            else
                sa = db->m_values[a.indexOrString >> 1].c_str();
            if(0 == (b.indexOrString & 1))
                sb = (char const*)b.indexOrString;
            else
                sb = db->m_values[b.indexOrString >> 1].c_str();

            while(0 != *sa)
            {
                if(*sa != *sb)
                    return false;
                ++sa;
                ++sb;
            }
            return (*sa == *sb);
        }
        safeptr<Database> m_database;
    };
private:
#if SG_ENABLE_ASSERT
    size_t m_header;
#endif
    std::unordered_map<Key, size_t, Hash, Pred> m_map;
#endif
    std::vector<std::string> m_values;
};
//=============================================================================
template <typename domain>
class TemplateFastSymbol
{
public:
    static void Init()
    {
        SG_ASSERT(nullptr == domain::s_database);
        domain::s_database.reset(new Database());
    }
    static void Shutdown()
    {
        SG_ASSERT(nullptr != domain::s_database);
        domain::s_database.reset();
    }
#if SG_ENABLE_ASSERT
    static bool IsInit_ForAssert() { return nullptr != domain::s_database; }
#endif
public:
    TemplateFastSymbol() : m_index(0) {}
    TemplateFastSymbol(std::string const& iValue) : m_index(domain::s_database->GetIndex(iValue.c_str())) {}
    TemplateFastSymbol(char const* iValue) : m_index(domain::s_database->GetIndex(iValue)) {}
    TemplateFastSymbol(TemplateFastSymbol const& iOther) : m_index(iOther.m_index) {}
    TemplateFastSymbol& operator=(TemplateFastSymbol const& iOther) { m_index = iOther.m_index; return *this; }
    TemplateFastSymbol& operator=(std::string const& iValue) { m_index = domain::s_database->GetIndex(iValue.c_str()); return *this; }
    TemplateFastSymbol& operator=(char const* iValue) { m_index = domain::s_database->GetIndex(iValue); return *this; }
    bool IsValid() const { return m_index != (size_t)0; }
    void Invalidate() { m_index = (size_t)0; }
    std::string const& Value() const { return domain::s_database->GetValue(m_index); }
    size_t Hash() const { return m_index; }
    friend bool operator== (TemplateFastSymbol const& a, TemplateFastSymbol const& b)
    {
        return a.m_index == b.m_index;
    }
    friend bool operator!= (TemplateFastSymbol const& a, TemplateFastSymbol const& b)
    {
        return a.m_index != b.m_index;
    }
private:
    size_t m_index;
};
//=============================================================================
} // namespace fastsymbol

// Generic Fast symbol, for misc. usage
FAST_SYMBOL_TYPE_HEADER(FastSymbol)
}

namespace std {
//=============================================================================
template <typename domain> struct hash<sg::fastsymbol::TemplateFastSymbol<domain> >
{
    size_t operator()(sg::fastsymbol::TemplateFastSymbol<domain> const& x) const
    {
        return x.Hash();
    }
};
//=============================================================================
}

#endif

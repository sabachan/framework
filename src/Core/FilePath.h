#ifndef Core_FilePath_H
#define Core_FilePath_H

#include <string>
#include "Config.h"

namespace sg {
//=============================================================================
class FilePath
{
public:
    FilePath();
    explicit FilePath(std::string const& iPath);
    FilePath(FilePath const& iPath);
    FilePath(FilePath&& iPath);
    FilePath& operator=(FilePath const& iPath);
    FilePath& operator=(FilePath&& iPath);
    ~FilePath();

    static FilePath CreateFromFullSystemPath(std::string const& iSystemPath);
    static FilePath CreateFromAnyPath(std::string const& iPath);

    bool Empty() const;
    std::string MountingPoint() const;
    std::string Extension() const;
    std::string Filename() const;
    bool IsRootDirectory() const;
    FilePath ParentDirectory() const;
    FilePath ReplaceMountingPoint(std::string const& iMountingPoint) const;
    FilePath Append(char const* iSubPath) const;
    FilePath Append(std::string const& iSubPath) const { return Append(iSubPath.c_str()); }

    std::string const& GetPrintableString() const;
    std::string GetSystemFilePath() const;

    size_t Hash() const { std::hash<std::string> h; return h(m_path); }
    bool Equals(FilePath const& a) const { return m_path == a.m_path; }
private:
    std::string m_path;

#if SG_ENABLE_ASSERT
    void CheckValidity() const;
public:
    static size_t GetInstanceCount() { return s_InstanceCount; }
private:
    static size_t s_InstanceCount;
#endif
};
//=============================================================================
inline bool operator==(FilePath const& a, FilePath const& b) { return a.Equals(b); }
inline bool operator!=(FilePath const& a, FilePath const& b) { return !(a==b); }
//=============================================================================
}
namespace std {
//=============================================================================
template <> struct hash<sg::FilePath>
{
    size_t operator()(const sg::FilePath & a) const { return a.Hash(); }
};
//=============================================================================
}


#endif

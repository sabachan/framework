#include "stdafx.h"

#include "FilePath.h"

#include "Assert.h"
#include "FileSystem.h"
#include <algorithm>
#include <sstream>

namespace sg {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string GetMountingPointIFP(std::string const& iPath)
{
    size_t pos = iPath.find(':');
    if(std::string::npos != pos)
        return iPath.substr(0, pos);
    else
        return std::string();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
#if SG_ENABLE_ASSERT
size_t FilePath::s_InstanceCount = 0;
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath FilePath::CreateFromFullSystemPath(std::string const& iSystemPath)
{
    std::ostringstream oss;
#if SG_PLATFORM_IS_WIN
    if(std::string::npos != iSystemPath.find('\\'))
    {
        std::string tmp = iSystemPath;
        std::replace(tmp.begin(), tmp.end(), '\\', '/');
        oss << "system:/" << tmp;
        return FilePath(oss.str());
    }
#endif
    oss << "system:/" << iSystemPath;
    return FilePath(oss.str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath FilePath::CreateFromAnyPath(std::string const& iPath)
{
    std::string const mountingpoint = GetMountingPointIFP(iPath);
    if(mountingpoint.empty())
    {
        // relative path
        std::string const wd = filesystem::GetWorkingDirSystemPath();
        std::ostringstream oss;
        oss << wd << '/' << iPath;
        return CreateFromFullSystemPath(oss.str());
    }
    else
    {
        std::string const* mppath = filesystem::GetMountingPointSystemPathIFP(mountingpoint);
        if(nullptr == mppath)
            return CreateFromFullSystemPath(iPath);
        else
            return FilePath(iPath);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath::FilePath()
    : m_path()
{
#if SG_ENABLE_ASSERT
    CheckValidity();
    s_InstanceCount++;
    SG_ASSERT(0 != s_InstanceCount);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath::FilePath(std::string const& iPath)
    : m_path(iPath)
{
#if SG_ENABLE_ASSERT
    CheckValidity();
    s_InstanceCount++;
    SG_ASSERT(0 != s_InstanceCount);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath::FilePath(FilePath const& iPath)
    : m_path(iPath.m_path)
{
#if SG_ENABLE_ASSERT
    CheckValidity();
    s_InstanceCount++;
    SG_ASSERT(0 != s_InstanceCount);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath::FilePath(FilePath&& iPath)
{
    swap(m_path, iPath.m_path);
#if SG_ENABLE_ASSERT
    CheckValidity();
    s_InstanceCount++;
    SG_ASSERT(0 != s_InstanceCount);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath& FilePath::operator=(FilePath const& iPath)
{
    m_path = iPath.m_path;
#if SG_ENABLE_ASSERT
    CheckValidity();
#endif
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath& FilePath::operator=(FilePath&& iPath)
{
    swap(m_path, iPath.m_path);
#if SG_ENABLE_ASSERT
    CheckValidity();
#endif
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath::~FilePath()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0 != s_InstanceCount);
    --s_InstanceCount;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool FilePath::Empty() const
{
    return m_path.empty();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string FilePath::MountingPoint() const
{
    SG_ASSERT(!Empty());
    return GetMountingPointIFP(m_path);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string FilePath::Extension() const
{
    SG_ASSERT(!Empty());
    size_t pos = m_path.rfind(".");
    if(std::string::npos != pos)
        return m_path.substr(pos+1, std::string::npos);
    else
        return std::string();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string FilePath::Filename() const
{
    SG_ASSERT(!Empty());
    size_t pos = m_path.rfind("/");
    if(std::string::npos != pos)
        return m_path.substr(pos+1, std::string::npos);
    else
        return std::string();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool FilePath::IsRootDirectory() const
{
    SG_ASSERT(!Empty());
    size_t const length = m_path.length();
    return ':' == m_path[length-1] || ('/' == m_path[length-1] && ':' == m_path[length-2]);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath FilePath::ParentDirectory() const
{
    SG_ASSERT(!Empty());
    SG_ASSERT(!IsRootDirectory());
    size_t pos = m_path.rfind("/");
    if(std::string::npos != pos)
    {
        SG_ASSERT(pos != 0);
        return FilePath(m_path.substr(0, pos));
    }
    else
        return FilePath();;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath FilePath::ReplaceMountingPoint(std::string const& iMountingPoint) const
{
    SG_ASSERT(!Empty());
    SG_ASSERT(!iMountingPoint.empty());
    SG_ASSERT(std::string::npos == iMountingPoint.find(":"));
    SG_ASSERT(std::string::npos == iMountingPoint.find("/"));
    SG_ASSERT(std::string::npos == iMountingPoint.find("\\"));
    size_t pos = m_path.find(":");
    SG_ASSERT(std::string::npos != pos);
    std::ostringstream oss;
    oss << iMountingPoint << ":" << m_path.substr(pos+1, std::string::npos);
    return FilePath(oss.str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FilePath FilePath::Append(char const* iSubPath) const
{
    SG_ASSERT(!Empty());
    std::ostringstream oss;
    size_t length = m_path.length();
    if('/' == m_path[length - 1])
        oss << m_path.substr(0, length - 1);
    else
        oss << m_path;
    oss << '/' << iSubPath;
    return FilePath(oss.str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string const& FilePath::GetPrintableString() const
{
    return m_path;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string FilePath::GetSystemFilePath() const
{
    SG_ASSERT(!Empty());
    size_t pos = m_path.find(":");
    SG_ASSERT(std::string::npos != pos);
    std::string const& mountingpoint = m_path.substr(0, pos);
    SG_ASSERT(mountingpoint == MountingPoint());
    std::string const& mppath = filesystem::GetMountingPointSystemPath(mountingpoint);
    if(mppath == "")
    {
        if(m_path[pos+1] == '/')
        {
            std::ostringstream oss;
            oss << mppath << m_path.substr(pos+2, std::string::npos);
            return oss.str();
        }
        else
        {
            SG_ASSERT(pos+1 == m_path.length());
            return "";
        }
    }
    else
    {
        SG_ASSERT(mppath[mppath.length() - 1] != '/');
        std::ostringstream oss;
        oss << mppath << m_path.substr(pos+1, std::string::npos);
        return oss.str();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void FilePath::CheckValidity() const
{
    if(!m_path.empty())
    {
        SG_ASSERT_MSG(std::string::npos == m_path.find('\\'), "Invalid character (\"\\\") in FilePath !");
        SG_ASSERT_MSG(std::string::npos != m_path.find(":"), "A FilePath must contain a mounting point !");
        size_t pos = m_path.find(':');
        SG_ASSERT_MSG(std::string::npos == m_path.rfind('/', pos), "Invalid character (\"/\") in mounting point !");
        SG_ASSERT_MSG(std::string::npos == m_path.find(':', pos+1) || MountingPoint() == "system", "Invalid character (\":\") in FilePath !");
        SG_ASSERT_MSG(nullptr != filesystem::GetMountingPointSystemPathIFP(MountingPoint()), "Unknown mounting point !");
    }
}
#endif
//=============================================================================
}

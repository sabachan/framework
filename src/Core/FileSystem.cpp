#include "stdafx.h"

#include "FileSystem.h"

#include "Assert.h"
#include "FilePath.h"
#include "ini.h"
#include "TestFramework.h"
#include "WinUtils.h"

#include <algorithm>
#include <deque>
#include <unordered_map>

namespace sg {
namespace filesystem {
//=============================================================================
namespace {
    class FileSystem
    {
    public:
        FileSystem()
            : m_mountingPoints()
            , m_workingDirStack()
        {
            auto r = m_mountingPoints.emplace("system", "");
            SG_ASSERT(r.second);
        }
        ~FileSystem()
        {
            SG_ASSERT(0 == FilePath::GetInstanceCount());
        }
        void SetWorkingDir(std::string const& iSystemPath)
        {
            SG_ASSERT_MSG(std::string::npos == iSystemPath.find('\\'), "Invalid character (\"\\\") in Path ! Please use \"/\" instead.");
            SG_ASSERT(!m_workingDirStack.empty());
            m_workingDirStack.back() = iSystemPath;
        }
        void PushWorkingDir(std::string const& iSystemPath)
        {
            SG_ASSERT_MSG(std::string::npos == iSystemPath.find('\\'), "Invalid character (\"\\\") in Path ! Please use \"/\" instead.");
            m_workingDirStack.push_back(iSystemPath);
        }
        void PopWorkingDir()
        {
            SG_ASSERT(!m_workingDirStack.empty());
            m_workingDirStack.pop_back();
        }
        void AddMountingPoint(std::string const& iName, std::string const& iSystemPath)
        {
            SG_ASSERT_MSG(std::string::npos == iSystemPath.find('\\'), "Invalid character (\"\\\") in Path ! Please use \"/\" instead.");
            auto r = m_mountingPoints.emplace(iName, iSystemPath);
            SG_ASSERT_MSG(r.second, "A mounting point with same name has already been defined !");
        }
        void RemoveMountingPoint(std::string const& iName)
        {
            auto r = m_mountingPoints.erase(iName);
            SG_ASSERT_MSG_AND_UNUSED(1 == r, "No mounting point with this name has been found !");
            SG_ASSERT_MSG(0 == FilePath::GetInstanceCount(), "There may still be some FilePath(s) using this mounting point !");
        }
        std::string const& GetWorkingDirSystemPath()
        {
            SG_ASSERT(!m_workingDirStack.empty());
            return m_workingDirStack.back();
        }
        std::string const* GetMountingPointSystemPathIFP(std::string const& iName)
        {
            auto f = m_mountingPoints.find(iName);
            if(f != m_mountingPoints.end())
                return &(f->second);
            else
                return nullptr;
        }
        std::string const& GetMountingPointSystemPath(std::string const& iName)
        {
            auto f = m_mountingPoints.find(iName);
            SG_ASSERT_MSG(f != m_mountingPoints.end(), "No mounting point with this name has been found !");
            return f->second;
        }
        void GetBestContainingMountingPointIfExists(std::string const& iSystemPath,
                                                    std::string const*& oMountingPointName,
                                                    std::string const*& oMountingPointSystemPath)
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            SG_UNUSED((iSystemPath, oMountingPointName, oMountingPointSystemPath));
        }
    private:
        std::unordered_map<std::string, std::string> m_mountingPoints;
        std::deque<std::string> m_workingDirStack;
    };
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
static FileSystem* g_filesystem = nullptr;
//=============================================================================
namespace {
#if SG_PLATFORM_IS_WIN
    std::string NormaliseSystemPathIFN(std::string const& iSystemPath)
    {
        std::string tmp = iSystemPath;
        std::replace(tmp.begin(), tmp.end(), '\\', '/');
        return tmp;
    }
#else
#error "TODO"
#endif
}
//=============================================================================
void Init()
{
    SG_ASSERT(nullptr == g_filesystem);
    g_filesystem = new FileSystem();

#if SG_PLATFORM_IS_WIN
    std::string wd;
    winutils::GetWinWorkingDir(wd);
    PushWorkingDir(wd);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetWorkingDir(std::string const& iSystemPath)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    g_filesystem->SetWorkingDir(NormaliseSystemPathIFN(iSystemPath));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PushWorkingDir(std::string const& iSystemPath)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    g_filesystem->PushWorkingDir(NormaliseSystemPathIFN(iSystemPath));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PopWorkingDir()
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    g_filesystem->PopWorkingDir();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void AddMountingPoint(std::string const& iName, std::string const& iSystemPath)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    g_filesystem->AddMountingPoint(iName, NormaliseSystemPathIFN(iSystemPath));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RemoveMountingPoint(std::string const& iName)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    g_filesystem->RemoveMountingPoint(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string const& GetWorkingDirSystemPath()
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    return g_filesystem->GetWorkingDirSystemPath();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string const* GetMountingPointSystemPathIFP(std::string const& iName)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    return g_filesystem->GetMountingPointSystemPathIFP(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string const& GetMountingPointSystemPath(std::string const& iName)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    return g_filesystem->GetMountingPointSystemPath(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetBestContainingMountingPointIfExists(std::string const& iSystemPath,
                                            std::string const*& oMountingPointName,
                                            std::string const*& oMountingPointSystemPath)
{
    SG_ASSERT_MSG(nullptr != g_filesystem, "filesystem library has not been initialized !");
    g_filesystem->GetBestContainingMountingPointIfExists(NormaliseSystemPathIFN(iSystemPath), oMountingPointName, oMountingPointSystemPath);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
#if SG_PLATFORM_IS_WIN
    PopWorkingDir();
#endif
    SG_ASSERT(nullptr != g_filesystem);
    delete g_filesystem;
    g_filesystem = nullptr;
}
//=============================================================================
namespace {
class INIMountingPointHandler : public ini::IEventHandler
{
public:
    virtual bool VirtualOnEntryROK(ini::StringRef iSection, ini::StringRef iName, ini::Value const& iValue) override
    {
        std::string const section(iSection.begin, iSection.end);
        std::string const name(iName.begin, iName.end);
        if(nullptr != iValue.asString)
        {
            std::string const value(iValue.asString->begin, iValue.asString->end);
            FilePath const d = FilePath::CreateFromAnyPath(value);
            AddMountingPoint(name, d.GetSystemFilePath());
        }
        else
        {
            std::string const value(iValue.srcString.begin, iValue.srcString.end);
            FilePath const d = FilePath::CreateFromAnyPath(value);
            AddMountingPoint(name, d.GetSystemFilePath());
        }
        return true;
    }
};
}
//=============================================================================
void MountDeclaredMountingPoints()
{
    FilePath d = FilePath::CreateFromFullSystemPath(GetWorkingDirSystemPath());
    FilePath inipath = d.Append("MountingPoints.ini");
    bool inifilefound = winutils::DoesFileOrDirectoryExists(inipath.GetSystemFilePath());
    while(!inifilefound && !d.IsRootDirectory())
    {
        d = d.ParentDirectory();
        inipath = d.Append("MountingPoints.ini");
        inifilefound = winutils::DoesFileOrDirectoryExists(inipath.GetSystemFilePath());
    }
    SG_ASSERT(inifilefound);
    if(!inifilefound)
        return;

    PushWorkingDir(inipath.ParentDirectory().GetSystemFilePath());

    INIMountingPointHandler eventHandler;
    bool ok = ini::ReadROK(inipath, &eventHandler);
    SG_ASSERT_AND_UNUSED(ok);

    PopWorkingDir();
}
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
SG_TEST((sg,filesystem), FileSystem, (quick))
{
    Init();

    AddMountingPoint("testMP", "C:/testMP/DummySystemPath");
    AddMountingPoint("testMP2", "Z:");
    {
        FilePath filepath("testMP:/Dir0/Dir1/File.txt");
        SG_ASSERT(filepath.Extension() == "txt");
        SG_ASSERT(filepath.Filename() == "File.txt");
        SG_ASSERT(filepath.MountingPoint() == "testMP");
        FilePath parentdir = filepath.ParentDirectory();
        SG_ASSERT(parentdir.GetPrintableString() == "testMP:/Dir0/Dir1");
        SG_ASSERT(parentdir.GetSystemFilePath() == "C:/testMP/DummySystemPath/Dir0/Dir1");
        SG_ASSERT(parentdir.Append("File2.txt").GetPrintableString() == "testMP:/Dir0/Dir1/File2.txt");
        SG_ASSERT(!parentdir.IsRootDirectory());
        FilePath rootdir = filepath.ParentDirectory().ParentDirectory().ParentDirectory();
        SG_ASSERT(rootdir.IsRootDirectory());
        SG_ASSERT(filepath.ReplaceMountingPoint("testMP2").GetPrintableString() == "testMP2:/Dir0/Dir1/File.txt");
        SG_ASSERT(filepath.ReplaceMountingPoint("testMP2").GetSystemFilePath() == "Z:/Dir0/Dir1/File.txt");
    }
    RemoveMountingPoint("testMP");
    RemoveMountingPoint("testMP2");

    MountDeclaredMountingPoints();

    SG_ASSERT(winutils::DoesFileOrDirectoryExists(FilePath("solution:/.gitignore").GetSystemFilePath()));
    SG_ASSERT(nullptr != strstr(FilePath("solution:/.gitignore").GetSystemFilePath().c_str(), "gitignore"));

    Shutdown();
}
#endif
//=============================================================================
}
}

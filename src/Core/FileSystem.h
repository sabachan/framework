#ifndef Core_FileSystem_H
#define Core_FileSystem_H

#include <string>
#include "Config.h"

namespace sg {
namespace filesystem {
//=============================================================================
void Init();
void SetWorkingDir(std::string const& iSystemPath);
void PushWorkingDir(std::string const& iSystemPath);
void PopWorkingDir();
void AddMountingPoint(std::string const& iName, std::string const& iSystemPath);
void RemoveMountingPoint(std::string const& iName);
std::string const& GetWorkingDirSystemPath();
std::string const* GetMountingPointSystemPathIFP(std::string const& iName);
std::string const& GetMountingPointSystemPath(std::string const& iName);
void GetBestContainingMountingPointIfExists(std::string const& iSystemPath, std::string const*& oMountingPointName, std::string const*& oMountingPointSystemPath);
void Shutdown();
//=============================================================================
void MountDeclaredMountingPoints();
//=============================================================================
}
}

#endif

#ifndef FileFormat_pnm_H
#define FileFormat_pnm_H

#include <Core/IntTypes.h>
#include <Core/FilePath.h>

namespace sg {
namespace pnm {
//=============================================================================
//void ReadInfoFromMemory();
//void ReadFromMemory();
//=============================================================================
bool WriteGrayROK(FilePath const& filename, u8 const* iData, size_t iDataSize, size_t iWidth, size_t iHeight, size_t iStrideInBytes);
bool WriteRGBROK(FilePath const& filename, u8 const* iData, size_t iDataSize, size_t iWidth, size_t iHeight, size_t iStrideInBytes);
//=============================================================================
}
}

#endif

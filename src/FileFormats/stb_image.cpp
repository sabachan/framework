#include "stdafx.h"

#include <Core/Config.h>
#include <Core/Platform.h>
#if !SG_ENABLE_ASSERT
#define STBI_NO_FAILURE_STRINGS
#endif
#define STBI_NO_STDIO
#if SG_COMPILER_IS_MSVC
#pragma warning( push )
#pragma warning( disable : 4100 ) // unreferenced formal parameter
#pragma warning( disable : 4312 ) // 'type cast': conversion from 'type' to 'type' of greater size
#pragma warning( disable : 4456 ) // declaration of 'name' hides previous local declaration
#pragma warning( disable : 4457 ) // declaration of 'name' hides function parameter
#endif
#include "stb_image.c"
#if SG_COMPILER_IS_MSVC
#pragma warning( pop )
#pragma warning( disable : 4505 ) // unreferenced local function has been removed
#endif

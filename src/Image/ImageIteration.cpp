#include "stdafx.h"

#include "ImageIteration.h"

#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/FilePath.h>
#include <Core/FileSystem.h>
#include <Core/TestFramework.h>
#include <Core/WinUtils.h>
#include <FileFormats/pnm.h>
#include "Brush.h"
#include "DebugImage.h"
#include "Draw.h"

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace image {
//=============================================================================
SG_TEST((sg, image), ImageIteration, (Image, quick))
{
    //TODO
}
//=============================================================================
}
}

#endif

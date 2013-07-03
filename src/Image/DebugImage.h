#ifndef Image_DebugImage_H
#define Image_DebugImage_H

#include <Core/Config.h>
#include "Brush.h"
#include "Draw.h"
#include "Image.h"

#if SG_ENABLE_ASSERT
#define SG_DEBUG_IMAGE_STEP_INTO(IMG, ...) \
    do { static bool dumpAlways = false; sg::image::WriteDebugImageIFN(IMG, #IMG, __FILE__, __LINE__, SG_FUNCTION_NAME, dumpAlways, __VA_ARGS__); } while(SG_CONSTANT_CONDITION(0))
#else
#define SG_DEBUG_IMAGE_STEP_INTO(IMG, ...) ((void)0)
#endif


#if SG_ENABLE_ASSERT
namespace sg {
namespace image {
//=============================================================================
void WriteDebugImageIFN(ImageView<u8 const> const& img,
                        char const* expr,
                        char const* file,
                        size_t line,
                        char const* fctName,
                        bool& dumpAlways,
                        bool forceDump = false);
//=============================================================================
void WriteDebugImageIFN(ImageView<ubyte3 const> const& img,
                        char const* expr,
                        char const* file,
                        size_t line,
                        char const* fctName,
                        bool& dumpAlways,
                        bool forceDump = false);
//=============================================================================
void WriteDebugImageIFN(ImageView<ubyte4 const> const& img,
                        char const* expr,
                        char const* file,
                        size_t line,
                        char const* fctName,
                        bool& dumpAlways,
                        bool forceDump = false);
//=============================================================================
}
}
#endif

#endif

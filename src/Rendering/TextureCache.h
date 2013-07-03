#ifndef Rendering_TextureCache_H
#define Rendering_TextureCache_H

#include <Core/ComPtr.h>
#include <Core/FilePath.h>
#include <Core/SmartPtr.h>
#include "SurfaceFormat.h"

namespace sg {
namespace rendering {
//=============================================================================
class IShaderResource;
class RenderDevice;
class TextureFromFile;
//=============================================================================
struct TextureParameters
{
    int logDownscale { 0 };
    int maxExtraMipCount { 0 };
    SurfaceFormat surfaceFormat { SurfaceFormat::R8G8B8A8_UNORM_SRGB };

    friend bool operator== (TextureParameters const& a, TextureParameters const& b)
    {
        return a.logDownscale     == b.logDownscale
            && a.maxExtraMipCount == b.maxExtraMipCount;
    }
};
//=============================================================================
namespace texturecache {
//=============================================================================
void Init();
void SetRenderDevice(RenderDevice* iRenderDevice);
void GetTexture(refptr<IShaderResource>& oTexture, FilePath const& iFilePath, TextureParameters const* iOptionnalParameters = nullptr);
void Shutdown();
//=============================================================================
}
//=============================================================================
}
}

#endif

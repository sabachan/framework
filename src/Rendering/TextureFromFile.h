#ifndef Rendering_TextureFromFile_H
#define Rendering_TextureFromFile_H

#include "IShaderResource.h"
#include "ResolutionServer.h"
#include <Core/ComPtr.h>
#include <Core/SmartPtr.h>

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace sg {
class FilePath;
namespace rendering {
//=============================================================================
class RenderDevice;
//=============================================================================
class TextureFromFile : public RefWeakAndSafeCountable
                      , public IShaderResource
{
    PARENT_REF_COUNTABLE(RefWeakAndSafeCountable)
    PARENT_SAFE_COUNTABLE(RefWeakAndSafeCountable)
public:
    TextureFromFile(RenderDevice const* iRenderDevice, FilePath const& iFilename);
    virtual ~TextureFromFile() override;

    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return m_shaderResourceView.get(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return &m_resolution; }
private:
    comptr<ID3D11Texture2D> m_texture;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    ResolutionServer m_resolution;
};
//=============================================================================
}
}

#endif

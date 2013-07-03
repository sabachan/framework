#include "stdafx.h"

#include "TextureCache.h"

#include "RenderDevice.h"
#include "TextureFromFile.h"
#include <Core/ArrayList.h>
#include <Core/HashMap.h>
#include <Core/Singleton.h>
#include <Core/TemplateUtils.h>

namespace sg {
namespace rendering {
namespace texturecache {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TextureCache
{
public:
    void SetRenderDevice(RenderDevice* iRenderDevice)
    {
        SG_ASSERT((nullptr == m_renderDevice) != (nullptr == iRenderDevice));
        m_renderDevice = iRenderDevice;
    }
    void GetTexture(refptr<IShaderResource>& oTexture, FilePath const& iFilePath, TextureParameters const* iOptionnalParameters)
    {
        SoftTextureDescriptor softDesc;
        softDesc.filepath = &iFilePath;
        softDesc.parameters = iOptionnalParameters;

        size_t index = all_ones;
        auto r = m_map.emplace(softDesc, all_ones);
        if(r.second)
        {
            index = m_textures.Size();
            m_textures.EmplaceBack(nullptr);
            r.first->second = index;
        }
        else
        {
            index = r.first->second;
            SG_ASSERT(index < m_textures.Size());
        }

        m_textures[index].Lock(oTexture);
        if(nullptr == oTexture)
        {
            // TODO: handle parameters
            TextureFromFile* texture = new rendering::TextureFromFile(m_renderDevice.get(), iFilePath);
            oTexture = texture;
            m_textures[index] = texture;
        }
    }
private:
    static size_t Hash(TextureParameters const& parameters)
    {
        // TODO: use generic hash functions when available
        size_t h = size_t(parameters.surfaceFormat);
        h *= 0xac21;
        h ^= size_t(parameters.logDownscale);
        h *= 0xac21;
        h ^= size_t(parameters.maxExtraMipCount);
        return h;
    }
    static size_t Hash(FilePath const& filepath, TextureParameters const* parameters)
    {
        // TODO: use generic hash functions when available
        size_t h = 0x13;
        char const* s = filepath.GetPrintableString().c_str();
        for(;;)
        {
            char const c = *s;
            if('\0' == c)
                break;
            h *= 0xac21;
            h ^= size_t(c);
            ++s;
        }
        size_t const parametersHash = Hash(nullptr == parameters ? TextureParameters() : *parameters);
        h ^= parametersHash;
        h ^= h >> 16;
        return h;
    }
    struct SoftTextureDescriptor
    {
        FilePath const* filepath;
        TextureParameters const* parameters;
    };
    struct TextureDescriptor
    {
        FilePath filepath;
        TextureParameters parameters;

        TextureDescriptor() : filepath(), parameters() {}
        TextureDescriptor(SoftTextureDescriptor const& d)
            : filepath(nullptr != d.filepath ? *d.filepath : FilePath())
            , parameters(nullptr != d.parameters ? *d.parameters : TextureParameters())
        {}
    };
    struct Hasher
    {
        size_t operator()(SoftTextureDescriptor const& a) const { return Hash(*a.filepath, a.parameters); }
        size_t operator()(TextureDescriptor const& a) const { return Hash(a.filepath, &a.parameters); }
    };
    struct Comparer
    {
        size_t operator()(TextureDescriptor const& a, TextureDescriptor const& b) const { return a.parameters == b.parameters; }
        size_t operator()(TextureDescriptor const& a, SoftTextureDescriptor const& b) const
        {
            if(a.filepath != *b.filepath)
                return false;
            if(nullptr == b.parameters)
                return a.parameters == TextureParameters();
            return a.parameters == *b.parameters;
        }
        size_t operator()(SoftTextureDescriptor const& a, TextureDescriptor const& b) const { return operator()(b, a); }
    };
    safeptr<RenderDevice> m_renderDevice;
    HashMap<TextureDescriptor, size_t, Hasher, Comparer> m_map;
    ArrayList<weakptr<TextureFromFile> > m_textures;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureCache* g_textureCache = nullptr;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
void Init()
{
    SG_ASSERT(nullptr == g_textureCache);
    g_textureCache = new TextureCache();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetRenderDevice(RenderDevice* iRenderDevice)
{
    SG_ASSERT(nullptr != g_textureCache);
    g_textureCache->SetRenderDevice(iRenderDevice);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetTexture(refptr<IShaderResource>& oTexture, FilePath const& iFilePath, TextureParameters const* iOptionnalParameters)
{
    SG_ASSERT(nullptr != g_textureCache);
    g_textureCache->GetTexture(oTexture, iFilePath, iOptionnalParameters);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    SG_ASSERT(nullptr != g_textureCache);
    delete g_textureCache;
    g_textureCache = nullptr;
}
//=============================================================================
}
}
}

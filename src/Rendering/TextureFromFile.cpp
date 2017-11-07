#include "stdafx.h"

#include "TextureFromFile.h"

#include "RenderDevice.h"
#include <Image/DebugImage.h>
#include <Image/ImageView.h>
#include <Core/Cast.h>
#include <Core/FilePath.h>
#include <Core/IntTypes.h>
#include <Core/Log.h>
#include <Core/SimpleFileReader.h>
#include <Core/StringUtils.h>
#include <Core/WinUtils.h>
#include <d3d11.h>
#include <sstream>

#define SG_TEXTURE_FROM_FILE_USES_STB_IMAGE 0
#if SG_TEXTURE_FROM_FILE_USES_STB_IMAGE
#include <FileFormats/stb_image.h>
#else
#include <Wincodec.h>
#include <Core/ComPtr.h>
#endif

// TODO:
// - Allow mipmap generation ?

namespace sg {
namespace rendering {
//=============================================================================
TextureFromFile::TextureFromFile(RenderDevice const* iRenderDevice, FilePath const& iFilename)
    : m_texture()
    , m_shaderResourceView()
    , m_resolution()
{
#if SG_TEXTURE_FROM_FILE_USES_STB_IMAGE
    SimpleFileReader file(iFilename);
    int x, y, comp;
    int r = stbi_info_from_memory((stbi_uc const*)file.data(), (int)file.size(), &x, &y, &comp);
    if(0 == r)
    {
        std::ostringstream oss;
        oss << "Can't load texture from " << iFilename.GetPrintableString() << "!" << std::endl;
        oss << "    Error: " << stbi_failure_reason();
        SG_LOG_ERROR("Rendering/Texture", oss.str().c_str());
        return;
    }

    int req_comp = (3 == comp) ? 4 : comp;

    SG_CODE_FOR_ASSERT(int const prev_x = x; int const prev_y = y; SG_UNUSED((prev_x, prev_y));)

    u8* img = stbi_load_from_memory((stbi_uc const*)file.data(), (int)file.size(), &x, &y, &comp, req_comp);
    if(nullptr == img)
    {
        std::ostringstream oss;
        oss << "Can't load texture from " << iFilename.GetPrintableString() << "!" << std::endl;
        oss << "    Error: " << stbi_failure_reason();
        SG_LOG_ERROR("Rendering/Texture", oss.str().c_str());
        return;
    }
#if SG_ENABLE_ASSERT
    image::ImageView<ubyte4 const> dbgImgTextureFromFile(img, uint2(x,y), x*4);
    SG_DEBUG_IMAGE_STEP_INTO(dbgImgTextureFromFile, false);
#endif

    ID3D11Device* device = iRenderDevice->D3DDevice();

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    switch(req_comp)
    {
    case 1: format = DXGI_FORMAT_R8_UNORM; break;
    case 2: format = DXGI_FORMAT_R8G8_UNORM; break;
    case 3: SG_ASSERT_NOT_REACHED();
    case 4: format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
    default:
        SG_ASSERT_NOT_REACHED();
    }

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = x;
    desc.Height = y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE ;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = img;
    subresourceData.SysMemPitch = sizeof(u8) * req_comp * x;
    subresourceData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateTexture2D(
        &desc,
        &subresourceData,
        m_texture.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

#if SG_ENABLE_ASSERT
    {
        std::ostringstream oss;
        oss << "TextureFromFile: " << iFilename.GetPrintableString();
        rendering::SetDebugName(m_texture.get(), oss.str());
    }
#endif

    stbi_image_free(img);

    hr = device->CreateShaderResourceView(
        m_texture.get(),
        NULL,
        m_shaderResourceView.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

    m_resolution.Set(uint2(x, y));
#else
    // This code is not optimized. It has been coded so as to get a working
    // reader fast. Improvements can and should be done.

    // Initialize COM
    CoInitialize(NULL);

    // The factory pointer
    comptr<IWICImagingFactory> factory;

    // Create the COM imaging factory
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(factory.GetPointerForInitialisation())
    );
    SG_ASSERT(SUCCEEDED(hr));

    comptr<IWICBitmapDecoder> decoder;

    hr = factory->CreateDecoderFromFilename(
        ConvertUTF8ToUCS2(iFilename.GetSystemFilePath()).c_str(), // Image to be decoded
        NULL,                            // Do not prefer a particular vendor
        GENERIC_READ,                    // Desired read access to the file
        WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
        decoder.GetPointerForInitialisation() // Pointer to the decoder
        );
    SG_ASSERT(SUCCEEDED(hr));

    // Retrieve the first frame of the image from the decoder
    comptr<IWICBitmapFrameDecode> frame;

    if (SUCCEEDED(hr))
    {
        hr = decoder->GetFrame(0, frame.GetPointerForInitialisation());
    }

    UINT x;
    UINT y;
    hr = frame->GetSize(&x, &y);
    SG_ASSERT(SUCCEEDED(hr));

    WICPixelFormatGUID wicPixelFormat;
    hr = frame->GetPixelFormat(&wicPixelFormat);
    SG_ASSERT(SUCCEEDED(hr));

    size_t pixelSizeInBytes = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    std::vector<u8> buffer;
    u8* img = nullptr;
    if(wicPixelFormat == GUID_WICPixelFormat8bppGray)
    {
        format = DXGI_FORMAT_R8_UNORM;
        pixelSizeInBytes = 1;
        buffer.resize(x*y*pixelSizeInBytes);
        hr = frame->CopyPixels(NULL, UINT(x * pixelSizeInBytes), UINT(buffer.size()), buffer.data());
        SG_ASSERT(SUCCEEDED(hr));
        img = buffer.data();
    }
    else if(wicPixelFormat == GUID_WICPixelFormat24bppBGR)
    {
        format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        std::vector<u8> tmpbuffer(x*y*3);
        hr = frame->CopyPixels(NULL, UINT(x * 3), UINT(tmpbuffer.size()), tmpbuffer.data());
        SG_ASSERT(SUCCEEDED(hr));
        pixelSizeInBytes = 4;
        buffer.resize(x * y * pixelSizeInBytes);
        // WTF? Why don't we need an inversion of RGB <-> BGR here, when we need it next case?
        for_range_0(int, i, x * y)
            reinterpret_cast<ubyte4*>(buffer.data())[i] = reinterpret_cast<ubyte3*>(tmpbuffer.data())[i].Append(u8(255));
        img = buffer.data();
    }
    else if(wicPixelFormat == GUID_WICPixelFormat32bppBGRA)
    {
        format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        pixelSizeInBytes = 4;
        buffer.resize(x*y*pixelSizeInBytes);
        hr = frame->CopyPixels(NULL, UINT(x * pixelSizeInBytes), UINT(buffer.size()), buffer.data());
        SG_ASSERT(SUCCEEDED(hr));
        for_range_0(int, i, x * y)
            reinterpret_cast<ubyte4*>(buffer.data())[i] = reinterpret_cast<ubyte4*>(buffer.data())[i].zyxw();
        img = buffer.data();
    }
    else
    {
        SG_ASSERT_NOT_IMPLEMENTED();
    }

    CoUninitialize();

#if SG_ENABLE_ASSERT
    if(pixelSizeInBytes == 4)
    {
        image::ImageView<ubyte4 const> dbgImgTextureFromFile(img, uint2(x,y), x*4);
        SG_DEBUG_IMAGE_STEP_INTO(dbgImgTextureFromFile, false);
    }
#endif

    ID3D11Device* device = iRenderDevice->D3DDevice();

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = x;
    desc.Height = y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE ;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = img;
    subresourceData.SysMemPitch = UINT(sizeof(u8) * pixelSizeInBytes * x);
    subresourceData.SysMemSlicePitch = 0;

    hr = device->CreateTexture2D(
        &desc,
        &subresourceData,
        m_texture.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

#if SG_ENABLE_ASSERT
    {
        std::ostringstream oss;
        oss << "TextureFromFile: " << iFilename.GetPrintableString();
        rendering::SetDebugName(m_texture.get(), oss.str());
    }
#endif

    hr = device->CreateShaderResourceView(
        m_texture.get(),
        NULL,
        m_shaderResourceView.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

    m_resolution.Set(uint2(x, y));
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureFromFile::~TextureFromFile()
{
}
//=============================================================================
}
}


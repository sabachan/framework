#include "stdafx.h"

#include "ImageFromFile.h"

#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/FilePath.h>
#include <Core/FileSystem.h>
#include <Core/Log.h>
#include <Core/SimpleFileReader.h>
#include <Core/StringUtils.h>
#include <Core/WinUtils.h>
#include <FileFormats/pnm.h>
#include <sstream>

#define SG_TEXTURE_FROM_FILE_USES_STB_IMAGE 1
#if SG_TEXTURE_FROM_FILE_USES_STB_IMAGE
#include <FileFormats/stb_image.h>
#else
#include <Wincodec.h>
#include <Core/ComPtr.h>
#endif

namespace sg {
namespace image {
//=============================================================================
ImageFromFile::ImageFromFile()
    : m_buffer()
    , m_resolution()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ImageFromFile::~ImageFromFile()
{
    ReleaseMemoryIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ImageFromFile::Clear()
{
    ReleaseMemoryIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ImageFromFile::ReleaseMemory()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(!m_buffer.Empty());
    m_checkerForImageView.OnRelease();
#endif
    m_buffer.Clear();
    m_colorFormat = ColorFormat::Unknown;
    m_resolution = uint2(0);
    m_strideInBytes = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ImageFromFile::Open(FilePath const& iFilename)
{
    ReleaseMemoryIFN();
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
        return false;
    }

    int req_comp = comp;

    SG_CODE_FOR_ASSERT(int const prev_x = x; int const prev_y = y; SG_UNUSED((prev_x, prev_y));)

    u8* img = stbi_load_from_memory((stbi_uc const*)file.data(), (int)file.size(), &x, &y, &comp, req_comp);
    if(nullptr == img)
    {
        std::ostringstream oss;
        oss << "Can't load texture from " << iFilename.GetPrintableString() << "!" << std::endl;
        oss << "    Error: " << stbi_failure_reason();
        SG_LOG_ERROR("Rendering/Texture", oss.str().c_str());
        return false;
    }

    size_t const pixelSizeInBytes = req_comp;
    m_resolution = uint2(x, y);
    m_strideInBytes = x * pixelSizeInBytes;
    m_buffer.resize(x*y*pixelSizeInBytes);
    memcpy(m_buffer.data(), img, x * y * pixelSizeInBytes);

    switch(req_comp)
    {
    case 1:
    {
        m_colorFormat = ColorFormat::Gray;
#if SG_ENABLE_ASSERT
        image::ImageView<u8> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
        break;
    }
    case 2:
    {
        m_colorFormat = ColorFormat::RG;
#if SG_ENABLE_ASSERT
        image::ImageView<ubyte2> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
        break;
    }
    case 3:
    {
        m_colorFormat = ColorFormat::RGB;
#if SG_ENABLE_ASSERT
        image::ImageView<ubyte3> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
        break;
    }
    case 4:
    {
        m_colorFormat = ColorFormat::RGBA;
#if SG_ENABLE_ASSERT
        image::ImageView<ubyte4> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
        break;
    }
    default:
        SG_ASSERT_NOT_REACHED();
    }

    stbi_image_free(img);
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
    if(wicPixelFormat == GUID_WICPixelFormat8bppGray)
    {
        format = DXGI_FORMAT_R8_UNORM;
        pixelSizeInBytes = 1;
        m_resolution = uint2(x, y);
        m_strideInBytes = x * pixelSizeInBytes;
        m_buffer.resize(x*y*pixelSizeInBytes);
        hr = frame->CopyPixels(NULL, UINT(x * pixelSizeInBytes), UINT(m_buffer.size()), m_buffer.data());
        SG_ASSERT(SUCCEEDED(hr));
#if SG_ENABLE_ASSERT
        image::ImageView<u8 const> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
    }
    else if(wicPixelFormat == GUID_WICPixelFormat24bppBGR)
    {
        format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        pixelSizeInBytes = 3;
        m_resolution = uint2(x, y);
        m_strideInBytes = x * pixelSizeInBytes;
        m_buffer.resize(x*y*pixelSizeInBytes);
        hr = frame->CopyPixels(NULL, UINT(x * 3), UINT(m_buffer.size()), m_buffer.data());
        SG_ASSERT(SUCCEEDED(hr));
#if SG_ENABLE_ASSERT
        image::ImageView<ubyte3 const> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
    }
    else if(wicPixelFormat == GUID_WICPixelFormat32bppBGRA)
    {
        // WTF? Why do we need an inversion of RGB <-> BGR here, when we didn't need it for previous case?
        format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        pixelSizeInBytes = 4;
        m_resolution = uint2(x, y);
        m_strideInBytes = x * pixelSizeInBytes;
        m_buffer.resize(x*y*pixelSizeInBytes);
        hr = frame->CopyPixels(NULL, UINT(x * pixelSizeInBytes), UINT(m_buffer.size()), m_buffer.data());
        SG_ASSERT(SUCCEEDED(hr));
        for_range_0(int, i, x * y)
            reinterpret_cast<ubyte4*>(m_buffer.data())[i] = reinterpret_cast<ubyte4*>(m_buffer.data())[i].zyxw();
#if SG_ENABLE_ASSERT
        image::ImageView<ubyte4 const> dbgImageFromFile(m_buffer.data(), m_resolution, m_strideInBytes);
#endif
    }
    else
    {
        SG_ASSERT_NOT_IMPLEMENTED();
    }

    CoUninitialize();
#endif
#if SG_ENABLE_ASSERT
    m_checkerForImageView.OnAcquire();
#endif
    return true;
}
//=============================================================================
}
}

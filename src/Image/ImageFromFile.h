#ifndef Image_ImageFromFile_H
#define Image_ImageFromFile_H

#include "Image.h"
#include "ImageView.h"
#include <Core/ArrayList.h>
#include <Core/FilePath.h>
#include <Core/SmartPtr.h>

namespace sg {
namespace image {
//=============================================================================
class ImageFromFile : public RefAndSafeCountable
{
public:
    enum ColorFormat
    {
        Unknown,
        Gray,
        RG,
        RGB,
        RGBA,
    };
    ImageFromFile();
    ~ImageFromFile();
    bool Open(FilePath const& iFilename);
    ColorFormat GetColorFormat() const { return m_colorFormat; }
    ImageView<u8 const>     GetGreyViewIFP() const { return GetViewIFPImpl<u8 const>(); }
    ImageView<ubyte2 const> GetRGViewIFP()  const  { return GetViewIFPImpl<ubyte2 const>(); }
    ImageView<ubyte3 const> GetRGBViewIFP()  const { return GetViewIFPImpl<ubyte3 const>(); }
    ImageView<ubyte4 const> GetRGBAViewIFP() const { return GetViewIFPImpl<ubyte4 const>(); }
    u8 const* Data() const { return m_buffer.data(); }
    size_t DataSize() const { return m_buffer.size(); }
    size_t StrideInBytes() const { return m_strideInBytes; }
    uint2 Resolution() const { return m_resolution; }
    void Clear();
private:
    template <typename T> struct ColorTraits         { static ColorFormat const format = ColorFormat::Unknown; };
    template <>           struct ColorTraits<u8>     { static ColorFormat const format = ColorFormat::Gray;    };
    template <>           struct ColorTraits<ubyte2> { static ColorFormat const format = ColorFormat::RG;      };
    template <>           struct ColorTraits<ubyte3> { static ColorFormat const format = ColorFormat::RGB;     };
    template <>           struct ColorTraits<ubyte4> { static ColorFormat const format = ColorFormat::RGBA;    };
    template <typename T>
    ImageView<T> GetViewIFPImpl() const
    {
        if(m_colorFormat == ColorTraits<std::remove_cv<T>::type>::format)
            return ImageView<T>(m_buffer.Data(), m_resolution, m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_checkerForImageView.GetSafeCountable()));
        return ImageView<T>();
    }
    void ReleaseMemoryIFN() { if(!m_buffer.Empty()) { ReleaseMemory(); } }
    void ReleaseMemory();
private:
    ArrayList<u8> m_buffer;
    uint2 m_resolution;
    size_t m_strideInBytes;
    ColorFormat m_colorFormat;
#if SG_ENABLE_ASSERT
    image::CheckerForImageView m_checkerForImageView;
#endif
};
//=============================================================================
}
}

#endif

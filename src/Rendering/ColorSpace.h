#ifndef Rendering_ColorSpace_H
#define Rendering_ColorSpace_H

#include "Color.h"
#include <Core/Config.h>
#include <Core/For.h>
#include <Core/TemplateUtils.h>
#include <Core/Utils.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <type_traits>

namespace sg {
//=============================================================================
enum class ColorSpace {
    sRGB,
    LinearRGB,
    CIEXYZ,
    CIExyY,
    HueChromaIntensity,
    HueChromaLightness,
    HueChromaValue,
    HueChromaLuma,
    HueSaturationIntensity,
    HueSaturationLightness,
    HueSaturationValue,
};
template <ColorSpace CS, typename T = float>
class ColorSpacePoint
{
    typedef ColorSpacePoint this_type;
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
    typedef math::Vector<T, 3> vector_type;
public:
    static ColorSpace const color_space = CS;
public:
    this_type() : m_data() {}
    explicit this_type(uninitialized_t) : m_data(uninitialized) {}
    explicit this_type(vector_type const& iData) : m_data(iData) {}
    this_type(value_type_for_const_passing c0, value_type_for_const_passing c1, value_type_for_const_passing c2) : m_data(c0, c1, c2) {}
    this_type(this_type const& iOther) { m_data = iOther.m_data; }
    this_type & operator=(this_type const& iOther) { m_data = iOther.m_data; return *this; }
    template<typename T2>
    explicit this_type(ColorSpacePoint<color_space, T2> const& iOther) { m_data = vector_type(iOther.m_data); }

    explicit operator vector_type const&() const { return m_data; }
public:
    vector_type m_data;
};
//=============================================================================
namespace fast {
inline float SRGBToLinear(float c) { return std::pow(c, 2.2f); }
inline float LinearToSRGB(float c) { return std::pow(c, 1/2.2f); }
template <size_t N> math::Vector<float, N> LinearToSRGB(math::Vector<float, N> const& c) { math::Vector<float, N> r(uninitialized); for_range(size_t, i, 0, N) { r._[i] = LinearToSRGB(c._[i]); } return r; }
template <size_t N> math::Vector<float, N> SRGBToLinear(math::Vector<float, N> const& c) { math::Vector<float, N> r(uninitialized); for_range(size_t, i, 0, N) { r._[i] = SRGBToLinear(c._[i]); } return r; }
inline Color3f LinearToSRGB(Color3f const& c) { Color3f r(uninitialized); for_range(size_t, i, 0, 3) { r.Data()._[i] = LinearToSRGB(c.Data()._[i]); } return r; }
inline Color3f SRGBToLinear(Color3f const& c) { Color3f r(uninitialized); for_range(size_t, i, 0, 3) { r.Data()._[i] = SRGBToLinear(c.Data()._[i]); } return r; }
}
namespace exact {
inline float SRGBToLinear(float c) { return c < 0.04045f ? c * (1/12.92f) : std::pow((c + 0.055f)*(1/(1+0.055f)), 2.4f); }
inline float LinearToSRGB(float c) { return c <= 0.0031308f ? 12.92f * c : (1 + 0.055f) * std::pow(c, 1/2.4f) - 0.055f; }
template <size_t N> math::Vector<float, N> LinearToSRGB(math::Vector<float, N> const& c) { math::Vector<float, N> r(uninitialized); for_range(size_t, i, 0, N) { r._[i] = LinearToSRGB(c._[i]); } return r; }
template <size_t N> math::Vector<float, N> SRGBToLinear(math::Vector<float, N> const& c) { math::Vector<float, N> r(uninitialized); for_range(size_t, i, 0, N) { r._[i] = SRGBToLinear(c._[i]); } return r; }
inline Color3f LinearToSRGB(Color3f const& c) { Color3f r(uninitialized); for_range(size_t, i, 0, 3) { r.Data()._[i] = LinearToSRGB(c.Data()._[i]); } return r; }
inline Color3f SRGBToLinear(Color3f const& c) { Color3f r(uninitialized); for_range(size_t, i, 0, 3) { r.Data()._[i] = SRGBToLinear(c.Data()._[i]); } return r; }
}
//=============================================================================
using exact::LinearToSRGB;
using exact::SRGBToLinear;
//=============================================================================
// https://en.wikipedia.org/wiki/CIE_1931_color_space (cf. discussion)
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
namespace {
#if 1
float const CIEXYZToLinearRGBMatrix[] = {
     float( 12831./3959.),  -float(    329./214.),   -float( 1974./3959.),
    -float(851781./878810.), float(1648619./878810.), float(36519./878810.),
     float(   705./12673.), -float(   2585./12673.),  float(  705./667.),
};
float const LinearRGBToCIEXYZMatrix[] = {
     float(506752./1228815.), float( 87881./245763.), float(  12673./70218.),
     float( 87098./409605.),  float(175762./245763.), float(  12673./175545.),
     float(  7918./409605.),  float( 87881./737289.), float(1001167./1053270.),
};
#else
float const CIEXYZToLinearRGBMatrix[] = {
     3.2406f, -1.5372f, -0.4986f,
    -0.9689f,  1.8758f,  0.0415f,
     0.0557f, -0.2040f,  1.0570f,
};
float const LinearRGBToCIEXYZMatrix[] = {
     0.4124f,  0.3576f,  0.1805f,
     0.2126f,  0.7152f,  0.0722f,
     0.0193f,  0.1192f,  0.9505f,
};
#endif
}
inline float3 LinearRGBToCIEXYZ(float3 const& c)
{
    float3x3 const M(LinearRGBToCIEXYZMatrix);
    return M * c;
}
inline float3 SRGBToCIEXYZ(float3 const& c)
{
    float3 const l = exact::SRGBToLinear(c);
    return LinearRGBToCIEXYZ(l);
}
inline float3 CIEXYZToLinearRGB(float3 const& c)
{
    float3x3 const M(CIEXYZToLinearRGBMatrix);
    return M * c;
}
inline float3 CIEXYZToSRGB(float3 const& c)
{
    float3 l = CIEXYZToLinearRGB(c);
    return exact::LinearToSRGB(c);
}
inline float3 CIEXYZToCIExyY(float3 const& c)
{
    float const s = c._[0] + c._[1] + c._[2];
    if(0.f == s)
        return float3(0.f);
    float const oos = 1.f / s;
    float3 const xyY = float3(c._[0] * oos, c._[1] * oos, c._[1]);
    return xyY;
}
inline float3 CIExyYToCIEXYZ(float3 const& c)
{
    float const y = c._[1];
    if(0.f == y)
        return float3(0.f);
    float const x = c._[0];
    float const z = 1.f - x - y;
    float const Y = c._[2];
    float const s = Y / y;
    float3 const XYZ = float3(s * x, Y, s * z);
    return XYZ;
}
//=============================================================================
// The following methods assume that input color is in sRGB color space if not
// mentionned.
inline float LumaFromSRGB(float3 const& c) { float const (&M)[9] = LinearRGBToCIEXYZMatrix; return M[3] * c._[0] + M[4] * c._[1] + M[5] * c._[2]; }
inline float Luma(Color3f const& c) { float const (&M)[9] = LinearRGBToCIEXYZMatrix; return M[3] * c.r() + M[4] * c.g() + M[5] * c.b(); }
namespace fast {  inline float Luminance(Color3f const& c) { float const (&M)[9] = LinearRGBToCIEXYZMatrix; float3 const l = SRGBToLinear(float3(c)); return M[3] * l._[0] + M[4] * l._[1] + M[5] * l._[2]; } }
namespace exact { inline float Luminance(Color3f const& c) { float const (&M)[9] = LinearRGBToCIEXYZMatrix; float3 const l = SRGBToLinear(float3(c)); return M[3] * l._[0] + M[4] * l._[1] + M[5] * l._[2]; } }
inline float LuminanceFromLinearRGB(Color3f const& c) { float const (&M)[9] = LinearRGBToCIEXYZMatrix; return M[3] * c.r() + M[4] * c.g() + M[5] * c.b(); }
//=============================================================================
template <ColorSpace color_space>
inline float3 SRGBToHSVLikeColorSpace(float3 const& c)
{
    static_assert(color_space == ColorSpace::HueChromaIntensity
        || color_space == ColorSpace::HueChromaLightness
        || color_space == ColorSpace::HueChromaValue
        || color_space == ColorSpace::HueChromaLuma
        || color_space == ColorSpace::HueSaturationIntensity
        || color_space == ColorSpace::HueSaturationLightness
        || color_space == ColorSpace::HueSaturationValue, "");
    float const r = c._[0];
    float const g = c._[1];
    float const b = c._[2];
    float M;
    float m;
    if(r > g)
        if(r > b)
            if(g > b)
                M = r, m = b;
            else
                M = r, m = g;
        else
            M = b, m = g;
    else
        if(g > b)
            if(r > b)
                M = g, m = b;
            else
                M = g, m = r;
        else
            M = b, m = r;
    SG_ASSERT(m <= r && r <= M);
    SG_ASSERT(m <= g && g <= M);
    SG_ASSERT(m <= b && b <= M);
    float const chroma = M - m;
    float hue = 0.f;
    if(0.f == chroma) {
        hue = 0.f;
    } else if(r == M) {
        hue = (g - b) / chroma; if(hue < 0.f) hue += 6.f;
    } else if(g == M) {
        hue = (b - r) / chroma + 2.f;
    } else if(b == M) {
        hue = (r - g) / chroma + 4.f;
    } else {
        SG_ASSUME_NOT_REACHED();
    }
    hue *= 1.f / 6.f;
    float const intensity = 1/3.f * (r + g + b);
    float const value = M;
    float const lightness = 0.5f * (M + m);
    float const luma = LumaFromSRGB(c);
    float const saturationForHSV = chroma == 0 ? 0 : chroma / value;
    float const saturationForHSL = chroma == 0 ? 0 : chroma / (1 - std::abs(2 * lightness - 1));
    float const saturationForHSI = chroma == 0 ? 0 : 1 - m / intensity;
    switch(color_space)
    {
    case ColorSpace::HueChromaIntensity:
        return float3(hue, chroma, intensity);
    case ColorSpace::HueChromaLightness:
        return float3(hue, chroma, lightness);
    case ColorSpace::HueChromaValue:
        return float3(hue, chroma, value);
    case ColorSpace::HueChromaLuma:
        return float3(hue, chroma, luma);
    case ColorSpace::HueSaturationIntensity:
        return float3(hue, saturationForHSI, intensity);
    case ColorSpace::HueSaturationLightness:
        return float3(hue, saturationForHSL, lightness);
    case ColorSpace::HueSaturationValue:
        return float3(hue, saturationForHSV, value);
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return float3(0);
}
inline float3 HSIToSRGB(float3 const& c)
{
    // saturation = 1 - m / intensity
    // intensity = 1/3 * (r + g + b);
    // chroma = M - m;
    // hue computed from x - m, let's call h = (x - m) / chroma
    //
    // m = saturation - 1 * intensity
    //
    // intensity = 1/3 * (M + m + m + h * (M - m));
    // M = (intensity * 3 - (2-h) * m) / (1+h)
    // x = m + h * (M - m)
    float const hue = c._[0];
    float const saturation = c._[1];
    float const intensity = c._[2];

    float const m = (1 - saturation) * intensity;

    float const H = hue * 6;
    float const floorH = floor(H);
    SG_ASSERT(0.f <= floorH && floorH <= 5.f);
    int const hueSector = int(floorH);
    float h = 0.f;
    switch(hueSector)
    {
    case 0: h = (H-0); break;
    case 1: h = (2-H); break;
    case 2: h = (H-2); break;
    case 3: h = (4-H); break;
    case 4: h = (H-4); break;
    case 5: h = (6-H); break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    SG_ASSERT(0 <= h);

    float const M = (intensity * 3 - (2-h) * m) / (1+h);

    float const x = m + h * (M - m);

    float3 rgb(uninitialized);
    switch(hueSector)
    {
    case 0: rgb = float3(M, x, m); break;
    case 1: rgb = float3(x, M, m); break;
    case 2: rgb = float3(m, M, x); break;
    case 3: rgb = float3(m, x, M); break;
    case 4: rgb = float3(x, m, M); break;
    case 5: rgb = float3(M, m, x); break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    return rgb;
}
template <ColorSpace color_space>
inline float3 HSVLikeColorSpaceToSRGB(float3 const& c)
{
    static_assert(color_space == ColorSpace::HueChromaIntensity
        || color_space == ColorSpace::HueChromaLightness
        || color_space == ColorSpace::HueChromaValue
        || color_space == ColorSpace::HueChromaLuma
        || color_space == ColorSpace::HueSaturationIntensity
        || color_space == ColorSpace::HueSaturationLightness
        || color_space == ColorSpace::HueSaturationValue, "");

    if(SG_CONSTANT_CONDITION(color_space == ColorSpace::HueSaturationIntensity))
        return HSIToSRGB(c);

    float chroma = 0.f;
    switch(color_space)
    {
    case ColorSpace::HueChromaIntensity:
    case ColorSpace::HueChromaLightness:
    case ColorSpace::HueChromaValue:
    case ColorSpace::HueChromaLuma:
        chroma = c._[1];
        break;
    case ColorSpace::HueSaturationLightness:
        chroma = c._[1] * (1 - std::abs(2 * c._[2] - 1));
        break;
    case ColorSpace::HueSaturationValue:
        chroma = c._[1] * c._[2];
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    float const H = c._[0] * 6;
    float const floorH = floor(H);
    SG_ASSERT(0.f <= floorH && floorH <= 5.f);
    float X = 0.f;
    int const hueSector = int(floorH);
    float3 preRGB;
    switch(hueSector)
    {
    case 0: X = chroma*(H-0); preRGB = float3(chroma,X,0); break;
    case 1: X = chroma*(2-H); preRGB = float3(X,chroma,0); break;
    case 2: X = chroma*(H-2); preRGB = float3(0,chroma,X); break;
    case 3: X = chroma*(4-H); preRGB = float3(0,X,chroma); break;
    case 4: X = chroma*(H-4); preRGB = float3(X,0,chroma); break;
    case 5: X = chroma*(6-H); preRGB = float3(chroma,0,X); break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    float m = 0.f;
    switch(color_space)
    {
    case ColorSpace::HueChromaIntensity:
        // intensity = 1/3.f * (M + x + m);
        // intensity = 1/3.f * (chroma + m + X + m + m);
        m = c._[2] - 1/3.f * (chroma + X);
        break;
    case ColorSpace::HueChromaLightness:
    case ColorSpace::HueSaturationLightness:
        // lightness = 0.5f * (M + m);
        m = c._[2] - 0.5f * chroma;
        break;
    case ColorSpace::HueChromaValue:
    case ColorSpace::HueSaturationValue:
        m = c._[2] - chroma;
        break;
    case ColorSpace::HueChromaLuma:
        SG_ASSERT_NOT_IMPLEMENTED();
        m = 0.f;
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    //float3 const rgb = chroma * preRGB + m;
    float3 const rgb = preRGB + m;
    return rgb;
}
//=============================================================================
// Hue Saturation Value
inline float3 SRGBToHSV(Color3f const& c)
{
    return SRGBToHSVLikeColorSpace<ColorSpace::HueSaturationValue>(float3(c));
}
inline Color3f HSVToSRGB(float3 const& c)
{
    return Color3f(HSVLikeColorSpaceToSRGB<ColorSpace::HueSaturationValue>(float3(c)));
}
//=============================================================================
// Hue Saturation Lightness
inline float3 SRGBToHSL(Color3f const& c)
{
    return SRGBToHSVLikeColorSpace<ColorSpace::HueSaturationLightness>(float3(c));
}
inline Color3f HSLToSRGB(float3 const& c)
{
    return Color3f(HSVLikeColorSpaceToSRGB<ColorSpace::HueSaturationLightness>(float3(c)));
}
//=============================================================================
namespace colorspaceinternal {
template <ColorSpace To, ColorSpace From> struct ConvertImpl {
    ColorSpacePoint<To> operator()(ColorSpacePoint<From> const& c) { SG_ASSERT_NOT_REACHED(); return ColorSpacePoint<To>(); }
};
template <ColorSpace CS> struct ConvertImpl<CS, CS> {
    ColorSpacePoint<CS> operator()(ColorSpacePoint<CS> const& c) { return c; }
};
template <> struct ConvertImpl<ColorSpace::sRGB, ColorSpace::sRGB> {
    ColorSpacePoint<ColorSpace::sRGB> operator()(ColorSpacePoint<ColorSpace::sRGB> const& c) { return c; }
};
template <> struct ConvertImpl<ColorSpace::LinearRGB, ColorSpace::LinearRGB> {
    ColorSpacePoint<ColorSpace::LinearRGB> operator()(ColorSpacePoint<ColorSpace::LinearRGB> const& c) { return c; }
};
template <ColorSpace From> struct ConvertImpl<ColorSpace::sRGB, From> {
    ColorSpacePoint<ColorSpace::sRGB> operator()(ColorSpacePoint<From> const& c) {
        return ColorSpacePoint<ColorSpace::sRGB>(HSVLikeColorSpaceToSRGB<From>(float3(c)));
    }
};
template <ColorSpace From> struct ConvertImpl<ColorSpace::LinearRGB, From> {
    ColorSpacePoint<ColorSpace::LinearRGB> operator()(ColorSpacePoint<From> const& c) {
        return ConvertImpl<ColorSpace::LinearRGB, ColorSpace::sRGB>()(ConvertImpl<ColorSpace::sRGB, From>()(c));
    }
};
template <ColorSpace To> struct ConvertImpl<To, ColorSpace::sRGB> {
    ColorSpacePoint<To> operator()(ColorSpacePoint<ColorSpace::sRGB> const& c) {
        return ColorSpacePoint<To>(SRGBToHSVLikeColorSpace<To>(float3(c)));
    }
};
template <ColorSpace To> struct ConvertImpl<To, ColorSpace::LinearRGB> {
    ColorSpacePoint<To> operator()(ColorSpacePoint<ColorSpace::LinearRGB> const& c) {
        return ConvertImpl<To, ColorSpace::sRGB>()(ConvertImpl<ColorSpace::sRGB, ColorSpace::LinearRGB>()(c));
    }
};
template <> struct ConvertImpl<ColorSpace::LinearRGB, ColorSpace::sRGB> {
    ColorSpacePoint<ColorSpace::LinearRGB> operator()(ColorSpacePoint<ColorSpace::sRGB> const& c) {
        return ColorSpacePoint<ColorSpace::LinearRGB>(exact::SRGBToLinear(float3(c)));
    }
};
template <> struct ConvertImpl<ColorSpace::sRGB, ColorSpace::LinearRGB> {
    ColorSpacePoint<ColorSpace::sRGB> operator()(ColorSpacePoint<ColorSpace::LinearRGB> const& c) {
        return ColorSpacePoint<ColorSpace::sRGB>(exact::LinearToSRGB(float3(c)));
    }
};
template <> struct ConvertImpl<ColorSpace::CIEXYZ, ColorSpace::CIExyY> {
    ColorSpacePoint<ColorSpace::CIEXYZ> operator()(ColorSpacePoint<ColorSpace::CIExyY> const& c) {
        return ColorSpacePoint<ColorSpace::CIEXYZ>(CIExyYToCIEXYZ(float3(c)));
    }
};
template <> struct ConvertImpl<ColorSpace::CIExyY, ColorSpace::CIEXYZ> {
    ColorSpacePoint<ColorSpace::CIExyY> operator()(ColorSpacePoint<ColorSpace::CIEXYZ> const& c) {
        return ColorSpacePoint<ColorSpace::CIExyY>(CIEXYZToCIExyY(float3(c)));
    }
};
template <> struct ConvertImpl<ColorSpace::LinearRGB, ColorSpace::CIEXYZ> {
    ColorSpacePoint<ColorSpace::LinearRGB> operator()(ColorSpacePoint<ColorSpace::CIEXYZ> const& c) {
        return ColorSpacePoint<ColorSpace::LinearRGB>(CIEXYZToLinearRGB(float3(c)));
    }
};
template <> struct ConvertImpl<ColorSpace::LinearRGB, ColorSpace::CIExyY> {
    ColorSpacePoint<ColorSpace::LinearRGB> operator()(ColorSpacePoint<ColorSpace::CIExyY> const& c) {
        return ConvertImpl<ColorSpace::LinearRGB, ColorSpace::CIEXYZ>()(ConvertImpl<ColorSpace::CIEXYZ, ColorSpace::CIExyY>()(c));
    }
};
template <> struct ConvertImpl<ColorSpace::CIEXYZ, ColorSpace::LinearRGB> {
    ColorSpacePoint<ColorSpace::CIEXYZ> operator()(ColorSpacePoint<ColorSpace::LinearRGB> const& c) {
        return ColorSpacePoint<ColorSpace::CIEXYZ>(LinearRGBToCIEXYZ(float3(c)));
    }
};
template <> struct ConvertImpl<ColorSpace::CIExyY, ColorSpace::LinearRGB> {
    ColorSpacePoint<ColorSpace::CIExyY> operator()(ColorSpacePoint<ColorSpace::LinearRGB> const& c) {
        return ConvertImpl<ColorSpace::CIExyY, ColorSpace::CIEXYZ>()(ConvertImpl<ColorSpace::CIEXYZ, ColorSpace::LinearRGB>()(c));
    }
};
template <> struct ConvertImpl<ColorSpace::sRGB, ColorSpace::CIEXYZ> {
    ColorSpacePoint<ColorSpace::sRGB> operator()(ColorSpacePoint<ColorSpace::CIEXYZ> const& c) {
        return ConvertImpl<ColorSpace::sRGB, ColorSpace::LinearRGB>()(ConvertImpl<ColorSpace::LinearRGB, ColorSpace::CIEXYZ>()(c));
    }
};
template <> struct ConvertImpl<ColorSpace::sRGB, ColorSpace::CIExyY> {
    ColorSpacePoint<ColorSpace::sRGB> operator()(ColorSpacePoint<ColorSpace::CIExyY> const& c) {
        return ConvertImpl<ColorSpace::sRGB, ColorSpace::LinearRGB>()(ConvertImpl<ColorSpace::LinearRGB, ColorSpace::CIExyY>()(c));
    }
};
template <> struct ConvertImpl<ColorSpace::CIEXYZ, ColorSpace::sRGB> {
    ColorSpacePoint<ColorSpace::CIEXYZ> operator()(ColorSpacePoint<ColorSpace::sRGB> const& c) {
        return ConvertImpl<ColorSpace::CIEXYZ, ColorSpace::LinearRGB>()(ConvertImpl<ColorSpace::LinearRGB, ColorSpace::sRGB>()(c));
    }
};
template <> struct ConvertImpl<ColorSpace::CIExyY, ColorSpace::sRGB> {
    ColorSpacePoint<ColorSpace::CIExyY> operator()(ColorSpacePoint<ColorSpace::sRGB> const& c) {
        return ConvertImpl<ColorSpace::CIExyY, ColorSpace::LinearRGB>()(ConvertImpl<ColorSpace::LinearRGB, ColorSpace::sRGB>()(c));
    }
};
}
template <ColorSpace To, ColorSpace From>
ColorSpacePoint<To> ConvertTo(ColorSpacePoint<From> const& c)
{
    return colorspaceinternal::ConvertImpl<To, From>()(c);
}
//=============================================================================
}

#endif

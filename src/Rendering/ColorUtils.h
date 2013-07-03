#ifndef Rendering_ColorUtils_H
#define Rendering_ColorUtils_H

#include "Color.h"
#include "ColorSpace.h"
#include <Core/For.h>
#include <Core/Utils.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <type_traits>

namespace sg {
//=============================================================================
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, u8>::type ToU8Clamp(T v) { return u8(roundi(clamp(v, T(0), T(255)))); }
template <typename T, size_t N>
math::Vector<u8, N> ToU8Clamp(math::Vector<T, N> const& v)
{
    typedef math::Vector<u8, N> return_type;
    return_type r(uninitialized);
    for_range(size_t, i, 0, N)
        r._[i] = ToU8Clamp(v._[i]);
    return r;
}
//=============================================================================
// Convert not alpha-premultiplied sRGB colors to alpha-premultiplied linear colors
inline Color4f srgba_to_lRGBa(R8G8B8A8 c)
{
    float const oo255 = 1.f / 255;
    float4 const srgba = oo255 * float4(ubyte4(c));
    auto const lrgb = SRGBToLinear(srgba.xyz());
    float const a = srgba.w();
    float3 const lRGB = a * float3(lrgb);
    float4 const lRGBa = lRGB.Append(a);
    return Color4f(lRGBa);
}
// Convert alpha-premultiplied sRGB colors to alpha-premultiplied linear colors
inline Color4f sRGBa_to_lRGBa(R8G8B8A8 c)
{
    float const oo255 = 1.f / 255;
    float4 const sRGBa = oo255 * float4(ubyte4(c));
    auto const lRGB = SRGBToLinear(sRGBa.xyz());
    float const a = sRGBa.w();
    float4 const lRGBa = lRGB.Append(a);
    return Color4f(lRGBa);
}
// Convert alpha-premultiplied linear colors to not alpha-premultiplied sRGB colors
inline R8G8B8A8 lRGBa_to_srgba(Color4f const& c)
{
    float const a = c.a();
    float3 const lrgb = float4(c).xyz() * (1.f / a);
    float3 const srgb = LinearToSRGB(lrgb);
    float4 const srgba = srgb.Append(a);
    return R8G8B8A8(ToU8Clamp(roundi(float4(c) * 255)));
}
// Convert alpha-premultiplied linear colors to alpha-premultiplied sRGB colors
inline R8G8B8A8 lRGBa_to_sRGBa(Color4f const& c)
{
    float const a = c.a();
    float3 const lRGB = float4(c).xyz();
    float3 const sRGB = LinearToSRGB(lRGB);
    float4 const sRGBa = sRGB.Append(a);
    return R8G8B8A8(ToU8Clamp(roundi(float4(c) * 255)));
}
//=============================================================================
// Porter-Duff unary operators
// (cf. http://keithp.com/~keithp/porterduff/p253-porter.pdf)
inline Color4f Darken  (Color4f const& c, float t) { SG_ASSERT(0 <= t && t <= 1.f); float const (&_)[4] = c.Data()._; return Color4f(t * _[0], t * _[1], t * _[2],     _[3]); }
inline Color4f Dissolve(Color4f const& c, float t) { SG_ASSERT(0 <= t && t <= 1.f); float const (&_)[4] = c.Data()._; return Color4f(t * _[0], t * _[1], t * _[2], t * _[3]); }
inline Color4f Opaque  (Color4f const& c, float t) { SG_ASSERT(0 <= t && t <= 1.f); float const (&_)[4] = c.Data()._; return Color4f(    _[0],     _[1],     _[2], t * _[3]); }
//=============================================================================
}

#endif

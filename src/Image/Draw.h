#ifndef Image_Draw_H
#define Image_Draw_H

#include <algorithm>
#include <Core/Assert.h>
#include <Core/For.h>
#include <Math/Box.h>
#include <Math/NumericalUtils.h>
#include <Math/Vector.h>
#include "Brush.h"
#include "BitMapFont.h"

namespace sg {
namespace image {
//=============================================================================
template <typename T> class AbstractImage;
//=============================================================================
namespace internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename D, typename S, typename B> void BlendIFP(D& d, S const& s, B& blend)
{
    blend(d, s);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename D> void BlendIFP(D&, nullptr_t, nullptr_t) { SG_ASSERT_NOT_REACHED(); }
template <typename D, typename B> void BlendIFP(D&, nullptr_t, B&) { SG_ASSERT_NOT_REACHED(); }
template <typename D, typename S> void BlendIFP(D&, S const&, nullptr_t) { SG_ASSERT_NOT_REACHED(); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T> void BlendIFP(T& d, T const& s, nullptr_t) { d = s; }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T> void BlendAA(T& dst, T src, float t)
{
    typedef T return_type;
    typedef float compute_type;
    compute_type tmp = fast::lerp(compute_type(dst), compute_type(src), t);
    dst = return_type(tmp);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, size_t dim> void BlendAA(math::Vector<T, dim>& dst, math::Vector<T, dim> src, float t)
{
    typedef math::Vector<T, dim> return_type;
    typedef math::Vector<float, dim> compute_type;
    compute_type tmp = fast::lerp(compute_type(dst), compute_type(src), t);
    dst = return_type(tmp);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool applyFill, bool applyStroke, typename D, typename B>
void BlendAA(D& dst, B const& brush, float fillIntensity, float strokeIntensity)
{
    SG_ASSERT(B::antialiasing);
    B::blend_type const blend = B::blend_type();
    if(SG_CONSTANT_CONDITION((applyFill && !std::is_same<typename B::fill_color_type, nullptr_t>::value
       && applyStroke && !std::is_same<typename B::stroke_color_type, nullptr_t>::value)))
    {
        float const totalIntensity = fillIntensity+strokeIntensity;
        D f = dst;
        D s = dst;
        internal::BlendIFP(f, brush.fill_color, blend);
        internal::BlendIFP(s, brush.stroke_color, blend);
        internal::BlendAA(f,s,strokeIntensity/totalIntensity);
        internal::BlendAA(dst, f, totalIntensity);
    }
    else if(SG_CONSTANT_CONDITION((applyFill && !std::is_same<typename B::fill_color_type, nullptr_t>::value)))
    {
        D r = dst;
        internal::BlendIFP(r, brush.fill_color, blend);
        internal::BlendAA(dst, r, fillIntensity);
    }
    else if(SG_CONSTANT_CONDITION((applyStroke && !std::is_same<typename B::stroke_color_type, nullptr_t>::value)))
    {
        D r = dst;
        internal::BlendIFP(r, brush.stroke_color, blend);
        internal::BlendAA(dst, r, strokeIntensity);
    }
}

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
math::Box<T, dim> ClampBox(math::Box<T, dim> const& toClamp, math::Box<T, dim> const& clamping)
{
    math::Box<T, dim> r;
    r.min = componentwise::min(clamping.max, componentwise::max(clamping.min, toClamp.min));
    r.max = componentwise::min(clamping.max, componentwise::max(clamping.min, toClamp.max));
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
struct WindowToViewport
{
public:
    WindowToViewport(uninitialized_t) : O(uninitialized), scale(uninitialized) {}
    WindowToViewport(box2f const& iWindow, box2f const& iViewport)
    {
        float2 lo = iWindow.min;
        float2 hi = iWindow.max;
        float2 const inC = 0.5f * (lo + hi);
        float2 const delta = hi - lo;
        float const d = std::max(delta.x(), delta.y());
        scale = iViewport.Delta() / d * float2(1, -1);
        float2 const outC = float2(iViewport.Center());
        O = inC - outC * (1.f/scale);
    }
    float2 operator() (float2 const& xy) { return (xy - O) * scale; };
    float2 Rescale(float2 const& v) { return v*scale; }
    WindowToViewport Inverse()
    {
        WindowToViewport r(uninitialized);
        r.scale = 1.f / scale;
        r.O = -O * scale;
        return r;
    }
private:
    float2 O;
    float2 scale;
};
//=============================================================================
template <typename F>
void Bresenham(int2 const& P0, int2 const& P1, F f)
{
    int2 const delta = P1 - P0;
    int2 const absDelta = componentwise::abs(delta);
    size_t const maindir = absDelta.x() < absDelta.y() ? 1 : 0;
    size_t const secdir = 1 - maindir;
    int2 const inc = int2(delta.x() > 0 ? 1 : -1, delta.y() > 0 ? 1 : -1);

    int acc = delta._[maindir] / 2;
    int2 P = P0;
    while(P._[maindir] != P1._[maindir]) {
        f(P);
        P._[maindir] += inc._[maindir];
        acc += absDelta._[secdir];
        if(acc >= absDelta._[maindir])
        {
            P._[secdir] += inc._[secdir];
            acc -= absDelta._[maindir];
        }
    }
    f(P);
}
//=============================================================================
inline bool ClipLineReturnInside(float2& P0, float2& P1, box2f const& rect)
{
    float2 const lo = rect.min;
    float2 const hi = rect.max;

    enum { LEFT = 0x1, RIGHT = 0x2, TOP = 0x4, BOTTOM = 0x8 };
    auto ComputeClipFlags = [&](float2 const& P)
    {
        u8 clipFlags = 0;
        if( P.x() < lo.x()) clipFlags |= LEFT;
        if(hi.x() <  P.x()) clipFlags |= RIGHT;
        if( P.y() < lo.y()) clipFlags |= TOP;
        if(hi.y() <  P.y()) clipFlags |= BOTTOM;
        return clipFlags;
    };

    u8 f0 = ComputeClipFlags(P0);
    u8 f1 = ComputeClipFlags(P1);

    SG_CODE_FOR_ASSERT(size_t loopCount = 0;)
    for(;;)
    {
        SG_ASSERT(loopCount++ < 4);

        if(0 != (f0 & f1))
            return false;
        u8 const f = f0 | f1;
        if(0 == f)
            return true;

        float2 P(uninitialized);
        if(f & (LEFT | RIGHT))
        {
            P.x() = f & LEFT ? lo.x() : hi.x();
            SG_ASSERT(P1.x() != P0.x());
            float const t = (P.x() - P0.x()) / (P1.x() - P0.x());
            P.y() = lerp(P0.y(), P1.y(), saturate(t));
        }
        else
        {
            SG_ASSERT(f & (TOP | BOTTOM));
            P.y() = f & TOP ? lo.y() : hi.y();
            SG_ASSERT(P1.y() != P0.y());
            float const t = (P.y() - P0.y()) / (P1.y() - P0.y());
            P.x() = lerp(P0.x(), P1.x(), saturate(t));
        }
        if(f0 & (LEFT | TOP))
            P0 = P, f0 = ComputeClipFlags(P0);
        else if(f1 & (LEFT | TOP))
            P1 = P, f1 = ComputeClipFlags(P1);
        else if(f0 & f)
            P0 = P, f0 = ComputeClipFlags(P0);
        else
            P1 = P, f1 = ComputeClipFlags(P1);
    }
}
//=============================================================================
template<typename T, typename B>
typename std::enable_if<is_Brush<B>::value, void>::type DrawPoint1pxNoAA(AbstractImage<T>& img, int2 P, B const& brush)
{
    if(!AllLessEqual(int2(0), P) || !AllLessStrict(P, int2(img.WidthHeight())))
        return;
    B::blend_type const blend = B::blend_type();
    internal::BlendIFP(img(uint2(P)), brush.stroke_color, blend);
}
//=============================================================================
template<typename T, typename B>
typename std::enable_if<is_Brush<B>::value, void>::type DrawLine1pxNoAA(AbstractImage<T>& img, int2 P0, int2 P1, B const& brush)
{
    typedef T dst_t;
    typedef typename std::conditional<B::antialiasing, float, decltype(int(0) + std::declval<B::stroke_thickness_type>())>::type value_t;
    typedef math::Vector<value_t, 2> vector_t;
    bool const enableStroke = !std::is_same<typename B::stroke_color_type, nullptr_t>::value;
    SG_ASSERT(enableStroke);

    B::blend_type const blend = B::blend_type();

    if(P0.x() > P1.x())
        std::swap(P0, P1);

    int2 const wh = int2(img.WidthHeight());
    float2 P0f(P0);
    float2 P1f(P1);
    bool const isInside = ClipLineReturnInside(P0f, P1f, box2f::FromMinMax(float2(0.f), float2(wh)-1.f));
    if(!isInside)
        return;
    P0 = roundi(P0f);
    P1 = roundi(P1f);

    Bresenham(P0, P1, [&](int2 const& P)
    {
        internal::BlendIFP(img(uint2(P)), brush.stroke_color, blend);
    });
}
//=============================================================================
template<typename T, typename B>
typename std::enable_if<is_Brush<B>::value, void>::type DrawCircle1pxNoAA(AbstractImage<T>& img, int2 C, int R, B const& brush)
{
    typedef T dst_t;
    typedef typename std::conditional<B::antialiasing, float, decltype(int(0) + std::declval<B::stroke_thickness_type>())>::type value_t;
    typedef math::Vector<value_t, 2> vector_t;
    bool const enableStroke = !std::is_same<typename B::stroke_color_type, nullptr_t>::value;
    SG_ASSERT(enableStroke);

    int2 const wh = int2(img.WidthHeight());

    if(C.x() + R < 0) return;
    if(C.x() - R >= wh.x()) return;
    if(C.y() + R < 0) return;
    if(C.y() - R >= wh.y()) return;

    // x^2 + y^2 = R^2
    // Let's begin at P = (0, R). square distance d2 is R^2.
    // When increasing x by 1, d2 increases by 2x+1.
    // When decreasing y by 1, d2 changes by 1-2y.
    int const R2 = R*R;
    int const d2Max = R2+R; // (R+1/2)^2, rounded down
    int x = 0;
    int y = R;
    int d2 = R2;
    while(x <= y)
    {
        DrawPoint1pxNoAA(img, C + int2( x, y), brush);
        if(0 != y) DrawPoint1pxNoAA(img, C + int2( x,-y), brush);
        if(0 != x) DrawPoint1pxNoAA(img, C + int2(-x, y), brush);
        if(0 != x && 0 != y) DrawPoint1pxNoAA(img, C + int2(-x,-y), brush);
        if(x != y)
        {
            DrawPoint1pxNoAA(img, C + int2( y, x), brush);
            if(0 != x) DrawPoint1pxNoAA(img, C + int2( y,-x), brush);
            if(0 != y) DrawPoint1pxNoAA(img, C + int2(-y, x), brush);
            if(0 != x && 0 != y) DrawPoint1pxNoAA(img, C + int2(-y,-x), brush);
        }

        d2 += 2*x + 1;
        ++x;
        if(d2 > d2Max)
        {
            d2 -= 2*y + 1;
            --y;
        }
    }
}
//=============================================================================
template<typename T, typename U, typename B>
typename std::enable_if<is_Brush<B>::value, void>::type DrawRect(AbstractImage<T>& img, math::Box<U, 2> const& box, B const& brush)
{
    typedef T dst_t;
    typedef typename std::conditional<B::antialiasing, float, decltype(U()+B::stroke_thickness_type())>::type value_t;
    typedef math::Box<value_t, 2> box_t;
    typedef math::Vector<value_t, 2> vector_t;
    bool const enableFill = !std::is_same<typename B::fill_color_type, nullptr_t>::value;
    bool const enableStroke = !std::is_same<typename B::stroke_color_type, nullptr_t>::value;

    B::blend_type const blend = B::blend_type();
    value_t const strokeThickness = value_t(brush.stroke_thickness);
    value_t const halfStrokeThickness = strokeThickness / 2;
    box_t const& rawFillBox = box_t::FromMinMax(vector_t(box.min) + vector_t(value_t(halfStrokeThickness)), vector_t(box.max) - vector_t(value_t(halfStrokeThickness)));
    box2i const rawFillBoxi =
        B::antialiasing ?
        box2i::FromMinMax(ceili(rawFillBox.min), floori(rawFillBox.max)):
        box2i::FromMinMax(roundi(rawFillBox.min), roundi(rawFillBox.max));
    box2i const rawExtStrokeBoxi =
        B::antialiasing ?
        box2i::FromMinMax(ceili(rawFillBox.min-strokeThickness), floori(rawFillBox.max+strokeThickness)):
        box2i::FromMinMax(roundi(rawFillBox.min-strokeThickness), roundi(rawFillBox.max+strokeThickness));
    box2i const rawIntStrokeBoxi =
        B::antialiasing ?
        box2i::FromMinMax(rawFillBoxi.min-1, rawFillBoxi.max+1):
        rawFillBoxi;

    box2i const imgBoxi = box2i::FromMinMax(int2(0), int2(img.WidthHeight()));
    box_t const fillBox = internal::ClampBox(rawFillBox, box_t(imgBoxi));
    box2i const fillBoxi = internal::ClampBox(rawFillBoxi, imgBoxi);
    box2i const extStrokeBoxi = internal::ClampBox(rawExtStrokeBoxi, imgBoxi);
    box2i const intStrokeBoxi = internal::ClampBox(rawIntStrokeBoxi, imgBoxi);

    if(SG_CONSTANT_CONDITION(enableFill))
    {
        for_range(size_t,y,fillBoxi.min.y(),fillBoxi.max.y())
        {
            for_range(size_t,x,fillBoxi.min.x(),fillBoxi.max.x())
            {
                internal::BlendIFP(img(x,y), brush.fill_color, blend);
            }
        }
    }
    if(SG_CONSTANT_CONDITION(enableStroke))
    {
        for_range(size_t,y,extStrokeBoxi.min.y(),intStrokeBoxi.min.y())
        {
            for_range(size_t,x,extStrokeBoxi.min.x(),extStrokeBoxi.max.x())
                internal::BlendIFP(img(x,y), brush.stroke_color, blend);
        }
        for_range(size_t,y,intStrokeBoxi.max.y(),extStrokeBoxi.max.y())
        {
            for_range(size_t,x,extStrokeBoxi.min.x(),extStrokeBoxi.max.x())
                internal::BlendIFP(img(x,y), brush.stroke_color, blend);
        }
        for_range(size_t,y,intStrokeBoxi.min.y(),intStrokeBoxi.max.y())
        {
            for_range(size_t,x,extStrokeBoxi.min.x(),intStrokeBoxi.min.x())
                internal::BlendIFP(img(x,y), brush.stroke_color, blend);
            for_range(size_t,x,intStrokeBoxi.max.x(),extStrokeBoxi.max.x())
                internal::BlendIFP(img(x,y), brush.stroke_color, blend);
        }
    }
    if(SG_CONSTANT_CONDITION(B::antialiasing))
    {
        if(SG_CONSTANT_CONDITION(enableFill || enableStroke))
        {
            value_t const aa_left = value_t(fillBoxi.min.x()) - fillBox.min.x();
            value_t const aa_top = value_t(fillBoxi.min.y()) - fillBox.min.y();
            value_t const aa_right = fillBox.max.x() - value_t(fillBoxi.max.x());
            value_t const aa_bottom = fillBox.max.y() - value_t(fillBoxi.max.y());
            value_t const aa_topleft = aa_top*aa_left;
            value_t const aa_topright = aa_top*aa_right;
            value_t const aa_bottomleft = aa_bottom*aa_left;
            value_t const aa_bottomright = aa_bottom*aa_right;
            using std::min;
            value_t const aa_stroke_left = enableStroke ? min(strokeThickness, 1-aa_left) : 0;
            value_t const aa_stroke_top = enableStroke ? min(strokeThickness, 1-aa_top) : 0;
            value_t const aa_stroke_right = enableStroke ? min(strokeThickness, 1-aa_right) : 0;
            value_t const aa_stroke_bottom = enableStroke ? min(strokeThickness, 1-aa_bottom) : 0;
            value_t const aa_stroke_topleft = enableStroke ? aa_stroke_top*aa_left+aa_stroke_left*aa_top+aa_stroke_top*aa_stroke_left : 0;
            value_t const aa_stroke_topright = enableStroke ? aa_stroke_top*aa_right+aa_stroke_right*aa_top+aa_stroke_top*aa_stroke_right : 0;
            value_t const aa_stroke_bottomleft = enableStroke ? aa_stroke_bottom*aa_left+aa_stroke_left*aa_bottom+aa_stroke_bottom*aa_stroke_left : 0;
            value_t const aa_stroke_bottomright = enableStroke ? aa_stroke_bottom*aa_right+aa_stroke_right*aa_bottom+aa_stroke_bottom*aa_stroke_right : 0;
            for_range(size_t,y,fillBoxi.min.y(),fillBoxi.max.y())
            {
                internal::BlendAA<enableFill, enableStroke>(img(fillBoxi.min.x()-1, y), brush, float(aa_left), float(aa_stroke_left));
                internal::BlendAA<enableFill, enableStroke>(img(fillBoxi.max.x(), y), brush, float(aa_right), float(aa_stroke_right));
            }
            for_range(size_t,x,fillBoxi.min.x(),fillBoxi.max.x())
            {
                internal::BlendAA<enableFill, enableStroke>(img(x,fillBoxi.min.y()-1), brush, float(aa_top), float(aa_stroke_top));
                internal::BlendAA<enableFill, enableStroke>(img(x,fillBoxi.max.y()), brush, float(aa_bottom), float(aa_stroke_bottom));
            }
            internal::BlendAA<enableFill, enableStroke>(img(uint2(fillBoxi.min-1)), brush, float(aa_topleft), float(aa_stroke_topleft));
            internal::BlendAA<enableFill, enableStroke>(img(fillBoxi.max.x(), fillBoxi.min.y()-1), brush, float(aa_topright), float(aa_stroke_topright));
            internal::BlendAA<enableFill, enableStroke>(img(fillBoxi.min.x()-1, fillBoxi.max.y()), brush, float(aa_bottomleft), float(aa_stroke_bottomleft));
            internal::BlendAA<enableFill, enableStroke>(img(uint2(fillBoxi.max)), brush, float(aa_bottomright), float(aa_stroke_bottomright));
        }
        if(SG_CONSTANT_CONDITION(enableStroke))
        {
            value_t const aa_left = value_t(extStrokeBoxi.min.x()) - fillBox.min.x() + strokeThickness;
            value_t const aa_top = value_t(extStrokeBoxi.min.y()) - fillBox.min.y() + strokeThickness;
            value_t const aa_right = fillBox.max.x() + strokeThickness - value_t(extStrokeBoxi.max.x());
            value_t const aa_bottom = fillBox.max.y() + strokeThickness - value_t(extStrokeBoxi.max.y());
            value_t const aa_topleft = aa_top*aa_left;
            value_t const aa_topright = aa_top*aa_right;
            value_t const aa_bottomleft = aa_bottom*aa_left;
            value_t const aa_bottomright = aa_bottom*aa_right;
            if(extStrokeBoxi.min.x() <= intStrokeBoxi.min.x())
                for_range(size_t,y,extStrokeBoxi.min.y(),extStrokeBoxi.max.y())
                {
                    internal::BlendAA<false, enableStroke>(img(extStrokeBoxi.min.x()-1, y), brush, 0, float(aa_left));
                }
            if(extStrokeBoxi.max.x() >= intStrokeBoxi.max.x())
                for_range(size_t,y,extStrokeBoxi.min.y(),extStrokeBoxi.max.y())
                {
                    internal::BlendAA<false, enableStroke>(img(extStrokeBoxi.max.x(), y), brush, 0, float(aa_right));
                }
            if(extStrokeBoxi.min.y() <= intStrokeBoxi.min.y())
                for_range(size_t,x,extStrokeBoxi.min.x(),extStrokeBoxi.max.x())
                {
                    internal::BlendAA<false, enableStroke>(img(x,extStrokeBoxi.min.y()-1), brush, 0, float(aa_top));
                }
            if(extStrokeBoxi.max.y() >= intStrokeBoxi.max.y())
                for_range(size_t,x,extStrokeBoxi.min.x(),extStrokeBoxi.max.x())
                {
                    internal::BlendAA<false, enableStroke>(img(x,extStrokeBoxi.max.y()), brush, 0, float(aa_bottom));
                }
            if(extStrokeBoxi.min.x() <= intStrokeBoxi.min.x() && extStrokeBoxi.min.y() <= intStrokeBoxi.min.y())
                internal::BlendAA<false, enableStroke>(img(uint2(extStrokeBoxi.min-1)), brush, 0, float(aa_topleft));
            if(extStrokeBoxi.max.x() >= intStrokeBoxi.max.x() && extStrokeBoxi.min.y() <= intStrokeBoxi.min.y())
                internal::BlendAA<false, enableStroke>(img(extStrokeBoxi.max.x(), extStrokeBoxi.min.y()-1), brush, 0, float(aa_topright));
            if(extStrokeBoxi.min.x() <= intStrokeBoxi.min.x() && extStrokeBoxi.max.y() >= intStrokeBoxi.max.y())
                internal::BlendAA<false, enableStroke>(img(extStrokeBoxi.min.x()-1, extStrokeBoxi.max.y()), brush, 0, float(aa_bottomleft));
            if(extStrokeBoxi.max.x() >= intStrokeBoxi.max.x() && extStrokeBoxi.max.y() >= intStrokeBoxi.max.y())
                internal::BlendAA<false, enableStroke>(img(uint2(extStrokeBoxi.max)), brush, 0, float(aa_bottomright));
        }
    }
}
//=============================================================================
template<typename T, typename U, typename B>
typename std::enable_if<is_Brush<B>::value, void>::type DrawText(AbstractImage<T>& img, math::Vector<U, 2> const& pos, std::string const& str, B const& brush, BitMapFont const& font = GetAlwaysAvailableBitmapFonts()[0])
{
    typedef int2 pos_t;
    B::blend_type const blend = B::blend_type();
    ubyte2 const wh = font.glyphSize;
    int const leading = wh.y() * 4 / 3;
    int const baseline = font.baseline;
    u64 const* glyphsData = font.glyphsData;
    pos_t pen = pos_t(pos) - pos_t(0, baseline);
    bool const forceMonospace = brush.monospace;
    u8 prev_c = 0;
    for_range(size_t, i, 0, str.size())
    {
        u8 const c = str[i];
        if('\n' == c)
        {
            pen = pos_t(int(pos.x()), pen.y() + leading);
            continue;
        }
        BitMapFont::GlyphInfo glyphInfo;
        font.GetGlyphInfo(glyphInfo, c);

        int const kerning = font.GetKerning(prev_c, c);
        if(SG_CONSTANT_CONDITION(!forceMonospace)) pen.x() += kerning;

        // NB: in case when forced to monospace, glyph positions are rounded down (to the left).
        SG_ASSERT(font.advanceMonospace >= glyphInfo.advance);
        int const monospaceOffset = int(font.advanceMonospace - glyphInfo.advance) >> 1;
        pos_t const penCopy = pen + pos_t(forceMonospace ? monospaceOffset : 0, 0);

        size_t const glyphDataAdressInBits = glyphInfo.glyphDataAdressInBits;
        for_range(size_t, y, 0, wh.y())
        {
            for_range(size_t, x, 0, wh.x())
            {
                pos_t const pixelPos = (penCopy + pos_t(int(x),int(y)));
                if(!AllGreaterEqual(pixelPos, pos_t(0,0)))
                    continue;
                if(!AllLessStrict(pixelPos, pos_t(img.Size())))
                    break;
                size_t const adress = glyphDataAdressInBits + y * wh.x() + x;
                size_t const adressShift = 6;
                size_t const adressMask = (1 << adressShift) - 1;
                if(1 == ((glyphsData[adress >> adressShift] >> (adress & adressMask)) & 1))
                {
                    internal::BlendIFP(img(uint2(pixelPos)), brush.fill_color, blend);
                }
            }
        }
        if(SG_CONSTANT_CONDITION(forceMonospace))
            pen.x() += font.advanceMonospace;
        else
            pen.x() += int(glyphInfo.advance);
        prev_c = c;
    }
}
//=============================================================================
}
}

#endif

#ifndef Image_Brush_H
#define Image_Brush_H

#include <Math/Vector.h>
#include <Math/NumericalUtils.h>

namespace sg {
namespace image {
//=============================================================================
namespace blend {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t dim>
math::Vector<u8, dim> u8lerp(math::Vector<u8, dim> const& a, math::Vector<u8, dim> const& b, u8 t)
{
    typedef math::Vector<u8, dim> return_type;
    typedef math::Vector<int, dim> compute_type;
    compute_type const delta = compute_type(b)-compute_type(a);
    compute_type const fracDelta = (delta * t + 127) / 255;
    compute_type const r = compute_type(a) + fracDelta;
    SG_ASSERT(AllGreaterEqual(r, compute_type(0)));
    SG_ASSERT(AllLessEqual(r, compute_type(255)));
    return return_type(r);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t dim>
math::Vector<u8, dim> u8maddclamp(math::Vector<u8, dim> const& a, u8 b, math::Vector<u8, dim> const& c)
{
    typedef math::Vector<u8, dim> return_type;
    typedef math::Vector<int, dim> compute_type;
    compute_type const mul = (compute_type(a) * b + 127) / 255;
    compute_type const r = componentwise::min(mul + compute_type(c), compute_type(255));
    SG_ASSERT(AllGreaterEqual(r, compute_type(0)));
    SG_ASSERT(AllLessEqual(r, compute_type(255)));
    return return_type(r);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t dim>
math::Vector<float, dim> floatmaddclamp(math::Vector<float, dim> const& a, float b, math::Vector<float, dim> const& c)
{
    typedef math::Vector<float, dim> vector_type;
    vector_type const madd = a * b + c;
    vector_type const r = componentwise::min(madd, vector_type(1));
    SG_ASSERT(AllGreaterEqual(r, vector_type(0)));
    SG_ASSERT(AllLessEqual(r, vector_type(1)));
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// TODO: template <size_t alphaIndex = 3>
struct PremultipliedRGBA
{
    void operator() (float4& o, float4 const& s) const
    {
        float const t = (1.f-s.w());
        o = floatmaddclamp(o, t, s);
    }
    void operator() (float3& o, float4 const& s) const
    {
        float const t = (1.f-s.w());
        o = floatmaddclamp(o, t, s.xyz());
    }
    void operator() (float3& o, float3 const& s) const
    {
        o = s;
    }
    void operator() (ubyte4& o, ubyte4 const& s) const
    {
        u8 const t = 255 - s.w();
        o = u8maddclamp(o, t, s);
    }
    void operator() (ubyte3& o, ubyte4 const& s) const
    {
        u8 const t = 255 - s.w();
        o = u8maddclamp(o, t, s.xyz());
    }
    void operator() (ubyte3& o, ubyte3 const& s) const
    {
        o = s;
    }
    void operator() (ubyte4& o, float4 const& s) const
    {
        float4 of = float4(o) * (1/255.f);
        operator()(of, s);
        SG_ASSERT(AllGreaterEqual(of, float4(0)));
        SG_ASSERT(AllLessEqual(of, float4(1)));
        o = ubyte4(roundi(of * 255.f));
    }
    void operator() (ubyte3& o, float4 const& s) const
    {
        float3 of = float3(o) * (1/255.f);
        operator()(of, s);
        SG_ASSERT(AllGreaterEqual(of, float3(0)));
        SG_ASSERT(AllLessEqual(of, float3(1)));
        o = ubyte3(roundi(of * 255.f));
    }
};
struct Classic
{
    void operator() (float4& o, float4 const& s) const
    {
        float const t = s.w();
        o = fast::lerp(o, s, t);
    }
    void operator() (float3& o, float4 const& s) const
    {
        float const t = s.w();
        o = fast::lerp(o, s.xyz(), t);
    }
    void operator() (float3& o, float3 const& s) const
    {
        o = s;
    }
    void operator() (ubyte4& o, ubyte4 const& s) const
    {
        u8 const t = s.w();
        o = u8lerp(o, s, t);
    }
    void operator() (ubyte3& o, ubyte4 const& s) const
    {
        u8 const t = s.w();
        o = u8lerp(o, s.xyz(), t);
    }
    void operator() (ubyte3& o, ubyte3 const& s) const
    {
        o = s;
    }
    void operator() (ubyte4& o, float4 const& s) const
    {
        float4 of = float4(o) * (1/255.f);
        operator()(of, s);
        o = ubyte4(roundi(of * 255.f));
    }
    void operator() (ubyte3& o, float4 const& s) const
    {
        float3 of = float3(o) * (1/255.f);
        operator()(of, s);
        o = ubyte3(roundi(of * 255.f));
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct Add { template <typename C> C operator() (C ca, C cb) { return ca + cb; } };
struct Sub {};
struct Min {};
struct Max {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define RETURN_TYPE decltype(std::declval<C>() * std::declval<A>())
struct Zero { template <typename C, typename A> RETURN_TYPE operator() (C const& c, A one, A as, A ias, A ad, A iad) { SG_UNUSED((c, one, as, ias, ad, iad)); return RETURN_TYPE(0); } };
struct One { template <typename C, typename A> RETURN_TYPE operator() (C const& c, A one, A as, A ias, A ad, A iad) { SG_UNUSED((c, one, as, ias, ad, iad)); return c * one; } };
struct SrcAlpha { template <typename C, typename A> RETURN_TYPE operator() (C const& c, A one, A as, A ias, A ad, A iad) { SG_UNUSED((c, one, as, ias, ad, iad)); return c * as; } };
struct DstAlpha { template <typename C, typename A> RETURN_TYPE operator() (C const& c, A one, A as, A ias, A ad, A iad) { SG_UNUSED((c, one, as, ias, ad, iad)); return c * ad; } };
struct InvSrcAlpha { template <typename C, typename A> RETURN_TYPE operator() (C const& c, A one, A as, A ias, A ad, A iad) { SG_UNUSED((c, one, as, ias, ad, iad)); return c * ias; } };
struct InvDstAlpha { template <typename C, typename A> RETURN_TYPE operator() (C const& c, A one, A as, A ias, A ad, A iad) { SG_UNUSED((c, one, as, ias, ad, iad)); return c * iad; } };
#undef RETURN_TYPE
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename SrcBlend, typename DstBlend, typename BlendOp>
struct BlendState
{
    void operator() (float4& o, float4 const& s) const
    {
        float const one = 1;
        float const as = s._[3];
        float const ad = o._[3];
        float const ias = 1.f - as;
        float const iad = 1.f - ad;
        float4 const cs = SrcBlend()(s, one, as, ias, ad, iad);
        float4 const cd = DstBlend()(o, one, as, ias, ad, iad);
        float4 const c = BlendOp()(cs, cd);
        o = saturate(c);
    }
    void operator() (float3& o, float4 const& s) const
    {
        float4 oex = o.Append(1.f);
        operator()(oex, s);
        o = oex.SubVector<3>(0);
    }
    void operator() (float3& o, float3 const& s) const
    {
        float4 const sex = s.Append(1.f);
        float4 oex = o.Append(1.f);
        operator()(oex, sex);
        o = oex.SubVector<3>(0);
    }
    void operator() (ubyte4& o, ubyte4 const& s) const
    {
        int const one = 255;
        int const as = s._[3];
        int const ad = o._[3];
        int const ias = 255 - as;
        int const iad = 255 - ad;
        int4 const cs = SrcBlend()(s, one, as, ias, ad, iad);
        int4 const cd = DstBlend()(o, one, as, ias, ad, iad);
        int4 const cfp = BlendOp()(cs, cd);
        int4 const c = cfp / 255;
        o = checked_numcastable(clamp(c, int4(0), int4(255)));
    }
    void operator() (ubyte3& o, ubyte4 const& s) const
    {
        ubyte4 oex = o.Append(255);
        operator()(oex, s);
        o = oex.SubVector<3>(0);
    }
    void operator() (ubyte3& o, ubyte3 const& s) const
    {
        ubyte4 const sex = s.Append(255);
        ubyte4 oex = o.Append(255);
        operator()(oex, sex);
        o = oex.SubVector<3>(0);
    }
    void operator() (ubyte4& o, float4 const& s) const
    {
        float4 of = float4(o) * (1/255.f);
        operator()(of, s);
        o = ubyte4(roundi(of * 255.f));
    }
    void operator() (ubyte3& o, float4 const& s) const
    {
        float3 of = float3(o) * (1/255.f);
        operator()(of, s);
        o = ubyte3(roundi(of * 255.f));
    }
};
}
//=============================================================================
template <
    typename sc_t = nullptr_t,
    typename st_t = nullptr_t,
    typename fc_t = nullptr_t,
    typename b_t = nullptr_t,
    bool aa = false,
    bool fm = false
>
struct Brush
{
private:
    template <typename, typename, typename, typename, bool, bool> friend struct Brush;
    friend Brush<> CreateBrush();
    Brush() : stroke_thickness(0) {}
public:
    typedef sc_t stroke_color_type;
    typedef typename std::conditional<std::is_same<st_t, nullptr_t>::value, int, st_t>::type stroke_thickness_type;
    typedef fc_t fill_color_type;
    typedef b_t blend_type;
    static bool const antialiasing = aa;
    static bool const monospace = fm;
public:
    template<typename T>
    Brush<T, st_t, fc_t, b_t, aa, fm> Stroke(T const& iColor) const
    {
        Brush<T, st_t, fc_t, b_t, aa, fm> tmp;
        tmp.stroke_color = iColor;
        tmp.stroke_thickness = stroke_thickness;
        tmp.fill_color = fill_color;
        return tmp;
    }
    template<typename T, typename U>
    Brush<T, U, fc_t, b_t, aa, fm> Stroke(T const& iColor, U const& iThickness) const
    {
        Brush<T, U, fc_t, b_t, aa, fm> tmp;
        tmp.stroke_color = iColor;
        tmp.stroke_thickness = iThickness;
        tmp.fill_color = fill_color;
        return tmp;
    }
    template<typename T>
    Brush<sc_t, T, fc_t, b_t, aa, fm> StrokeThickness(T const& iThickness) const
    {
        Brush<sc_t, T, fc_t, b_t, aa, fm> tmp;
        tmp.stroke_color = stroke_color;
        tmp.stroke_thickness = iThickness;
        tmp.fill_color = fill_color;
        return tmp;
    }
    template<typename T>
    Brush<sc_t, st_t, T, b_t, aa, fm> Fill(T const& iColor) const
    {
        Brush<sc_t, st_t, T, b_t, aa, fm> tmp;
        tmp.stroke_color = stroke_color;
        tmp.stroke_thickness = stroke_thickness;
        tmp.fill_color = iColor;
        return tmp;
    }
    Brush<sc_t, st_t, fc_t, b_t, true, fm> Antialiased() const
    {
        Brush<sc_t, st_t, fc_t, b_t, true, fm> tmp;
        tmp.stroke_color = stroke_color;
        tmp.stroke_thickness = stroke_thickness;
        tmp.fill_color = fill_color;
        return tmp;
    }
    Brush<sc_t, st_t, fc_t, b_t, aa, true> Monospace() const
    {
        Brush<sc_t, st_t, fc_t, b_t, aa, true> tmp;
        tmp.stroke_color = stroke_color;
        tmp.stroke_thickness = stroke_thickness;
        tmp.fill_color = fill_color;
        return tmp;
    }
    template<typename T>
    Brush<sc_t, st_t, fc_t, T, aa, fm> Blend() const
    {
        Brush<sc_t, st_t, fc_t, T, aa, fm> tmp;
        tmp.stroke_color = stroke_color;
        tmp.stroke_thickness = stroke_thickness;
        tmp.fill_color = fill_color;
        return tmp;
    }
public:
    stroke_color_type stroke_color;
    stroke_thickness_type stroke_thickness;
    fill_color_type fill_color;
    float aa_radius;
private:
};
//=============================================================================
template <typename T> struct is_Brush { static const bool value = false; };
template <typename sc_t, typename st_t, typename fc_t, typename b_t, bool aa, bool fm> struct is_Brush<Brush<sc_t, st_t, fc_t, b_t, aa, fm> > { static const bool value = true; };
//=============================================================================
inline Brush<> CreateBrush() { return Brush<>(); }
//=============================================================================
namespace brush {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T> Brush<T, nullptr_t, nullptr_t, nullptr_t, false, false> Stroke(T const& iColor) { return CreateBrush().Stroke(iColor); }
template<typename T> Brush<nullptr_t, nullptr_t, T, nullptr_t, false, false> Fill(T const& iColor) { return CreateBrush().Fill(iColor); }
inline Brush<nullptr_t, nullptr_t, nullptr_t, nullptr_t, true, false> Antialiased() { return CreateBrush().Antialiased(); }
inline Brush<nullptr_t, nullptr_t, nullptr_t, nullptr_t, false, true> Monospace() { return CreateBrush().Monospace(); }
template<typename T> Brush<nullptr_t, nullptr_t, nullptr_t, T, false, false> Blend() { return CreateBrush().Blend<T>(); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
}
}

#endif

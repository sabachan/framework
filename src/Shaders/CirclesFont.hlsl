#ifndef CirclesFont_HLSL
#define CirclesFont_HLSL

#include "AntiAliasing.hlsl"
#include "Geometry.hlsl"
#include "Unicode.hlsl"

#define _0_ UNICODE_DIGIT_ZERO
#define _1_ UNICODE_DIGIT_ONE
#define _2_ UNICODE_DIGIT_TWO
#define _3_ UNICODE_DIGIT_THREE
#define _4_ UNICODE_DIGIT_FOUR
#define _5_ UNICODE_DIGIT_FIVE
#define _6_ UNICODE_DIGIT_SIX
#define _7_ UNICODE_DIGIT_SEVEN
#define _8_ UNICODE_DIGIT_EIGHT
#define _9_ UNICODE_DIGIT_NINE

#define A_ UNICODE_LATIN_CAPITAL_LETTER_A
#define B_ UNICODE_LATIN_CAPITAL_LETTER_B
#define C_ UNICODE_LATIN_CAPITAL_LETTER_C
#define D_ UNICODE_LATIN_CAPITAL_LETTER_D
#define E_ UNICODE_LATIN_CAPITAL_LETTER_E
#define F_ UNICODE_LATIN_CAPITAL_LETTER_F
#define G_ UNICODE_LATIN_CAPITAL_LETTER_G
#define H_ UNICODE_LATIN_CAPITAL_LETTER_H
#define I_ UNICODE_LATIN_CAPITAL_LETTER_I
#define J_ UNICODE_LATIN_CAPITAL_LETTER_J
#define K_ UNICODE_LATIN_CAPITAL_LETTER_K
#define L_ UNICODE_LATIN_CAPITAL_LETTER_L
#define M_ UNICODE_LATIN_CAPITAL_LETTER_M
#define N_ UNICODE_LATIN_CAPITAL_LETTER_N
#define O_ UNICODE_LATIN_CAPITAL_LETTER_O
#define P_ UNICODE_LATIN_CAPITAL_LETTER_P
#define Q_ UNICODE_LATIN_CAPITAL_LETTER_Q
#define R_ UNICODE_LATIN_CAPITAL_LETTER_R
#define S_ UNICODE_LATIN_CAPITAL_LETTER_S
#define T_ UNICODE_LATIN_CAPITAL_LETTER_T
#define U_ UNICODE_LATIN_CAPITAL_LETTER_U
#define V_ UNICODE_LATIN_CAPITAL_LETTER_V
#define W_ UNICODE_LATIN_CAPITAL_LETTER_W
#define X_ UNICODE_LATIN_CAPITAL_LETTER_X
#define Y_ UNICODE_LATIN_CAPITAL_LETTER_Y
#define Z_ UNICODE_LATIN_CAPITAL_LETTER_Z

#define a_ UNICODE_LATIN_SMALL_LETTER_A
#define b_ UNICODE_LATIN_SMALL_LETTER_B
#define c_ UNICODE_LATIN_SMALL_LETTER_C
#define d_ UNICODE_LATIN_SMALL_LETTER_D
#define e_ UNICODE_LATIN_SMALL_LETTER_E
#define f_ UNICODE_LATIN_SMALL_LETTER_F
#define g_ UNICODE_LATIN_SMALL_LETTER_G
#define h_ UNICODE_LATIN_SMALL_LETTER_H
#define i_ UNICODE_LATIN_SMALL_LETTER_I
#define j_ UNICODE_LATIN_SMALL_LETTER_J
#define k_ UNICODE_LATIN_SMALL_LETTER_K
#define l_ UNICODE_LATIN_SMALL_LETTER_L
#define m_ UNICODE_LATIN_SMALL_LETTER_M
#define n_ UNICODE_LATIN_SMALL_LETTER_N
#define o_ UNICODE_LATIN_SMALL_LETTER_O
#define p_ UNICODE_LATIN_SMALL_LETTER_P
#define q_ UNICODE_LATIN_SMALL_LETTER_Q
#define r_ UNICODE_LATIN_SMALL_LETTER_R
#define s_ UNICODE_LATIN_SMALL_LETTER_S
#define t_ UNICODE_LATIN_SMALL_LETTER_T
#define u_ UNICODE_LATIN_SMALL_LETTER_U
#define v_ UNICODE_LATIN_SMALL_LETTER_V
#define w_ UNICODE_LATIN_SMALL_LETTER_W
#define x_ UNICODE_LATIN_SMALL_LETTER_X
#define y_ UNICODE_LATIN_SMALL_LETTER_Y
#define z_ UNICODE_LATIN_SMALL_LETTER_Z

//=============================================================================
struct circlesfont_Parameters
{
    float size;

    // editable parameters
    float ascend;
    float descend;
    float xheight;
    float thickness;
    float thinness;
    float oblique;
    bool showdot;
    float interchar;
    float interline;
    float dotpos;
    float italic_inclination;

    // computed parameters
    float2 offset;
    float width;
    float radius;
    float advance; // monospace font
    float lineheight;
    float i_pos;
    float e_hbar;
    float s_hbar;
    float t_hbar;
    float2 C0;
    float2 C1;
    float2 C2;
    float2 C3;
    float2 C4;
    float s_oblique;

    bool isValidated;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void circlesfont_Validate(inout circlesfont_Parameters p)
{
    p.isValidated = true;
    if(p.descend < 2*p.thickness) p.isValidated = false;
    if(p.ascend < 0.75f) p.isValidated = false;
    if(p.xheight < 0.4f || p.xheight > 0.6f) p.isValidated = false;
    if(p.thickness < p.thinness) p.isValidated = false;
    if(p.thinness <= 0.f) p.isValidated = false;
    if(p.oblique < 0.f) p.isValidated = false;
    if(p.thickness >= p.radius) p.isValidated = false;
    if(abs(p.italic_inclination) > 2.5f) p.isValidated = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
circlesfont_Parameters circlesfont_ComputeHintingAndParameters(in circlesfont_Parameters p)
{
    const float size = p.size;
    const float oosize = 1.f/size;

    p.size = size;

    // this ensures that lines won't be to thin to show.
    p.thickness = max(0.6f*oosize, p.thickness);
    p.thinness = max(0.4f*oosize, p.thinness);

    float offset = 0.5f * saturate(1.f - p.thickness * size) * oosize;
    p.offset.y = -offset;
    p.offset.x = offset;

    float gridAligned_xheight = floor(p.xheight * size + 0.5f) * oosize;
    p.xheight = gridAligned_xheight - 2.f * offset;
    p.ascend = floor(p.ascend * size + 0.5f) * oosize - 2.f * offset;
    p.descend = floor(p.descend * size + 0.5f) * oosize;

    p.interchar = max(oosize, floor(p.interchar * size + 0.5f) * oosize);
    p.interline = max(oosize, floor(p.interline * size + 0.5f) * oosize);

    p.oblique = p.oblique;
    p.showdot = p.showdot;
    p.advance = floor(p.advance * size + 0.5f) * oosize;
    p.lineheight = floor(p.lineheight * size + 0.5f) * oosize;

    p.dotpos = floor((p.dotpos - gridAligned_xheight) * size + 0.5f) * oosize + gridAligned_xheight - offset;

    p.width = p.xheight;
    p.radius = 0.5f*p.xheight;

    float bias = 0.25f;
    float sat_thickness = saturate(p.thickness * size) * oosize;
    //p.i_pos = (floor(p.radius * size + bias) + 0.5f) * oosize - 0.5f*sat_thickness - offset;
    p.i_pos = (floor(p.radius * size + bias) + 0.5f) * oosize - 0.5f*p.thickness - offset;
    p.e_hbar = p.radius;
    p.s_hbar = p.radius-0.5f*p.thinness;
    p.t_hbar = p.xheight;
    p.e_hbar = floor(p.e_hbar * size) * oosize;
    p.s_hbar = floor(p.s_hbar * size) * oosize;
    p.t_hbar = floor(p.t_hbar * size) * oosize;

    p.C0 = float2(0.5f * p.width, -p.radius); // OK, all has been computed for this
    p.C1 = float2(0.5f * p.width, -p.ascend+p.radius); // idem
    p.C2 = float2(0.5f * p.width, p.descend-p.radius); // idem
    // For C3 and C4, they must create circles that go through the pixels of of t_hbar.
    //p.C3 = p.C0+float2(0,-p.thinness);
    //p.C4 = p.C0+float2(0,-2.f*p.radius);
    p.C3 = float2(p.C0.x, -p.t_hbar - p.thinness + p.radius);
    p.C4 = float2(p.C0.x, -p.t_hbar - p.radius);

    p.isValidated = false;

    return p;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
circlesfont_Parameters circlesfont_ComputeParameters(in circlesfont_Parameters p)
{
    p.offset = float2(0,0);
    p.width = p.xheight;
    p.advance = p.width + p.interchar;
    p.lineheight = p.ascend + p.descend + p.interline;
    p.radius = 0.5f * p.xheight;
    p.i_pos = p.radius-0.5f*p.thickness;
    p.e_hbar = p.radius;
    p.s_hbar = p.radius-0.5f*p.thinness;
    p.t_hbar = p.xheight;

    p.C0 = float2(0.5f * p.width, -p.radius);
    p.C1 = float2(0.5f * p.width, -p.ascend+p.radius);
    p.C2 = float2(0.5f * p.width, p.descend-p.radius);
    p.C3 = p.C0+float2(0,-p.thinness);
    p.C4 = p.C0+float2(0,-2.f*p.radius);

    p.s_oblique = max(p.oblique, 1.f);

    p.isValidated = false;

    return p;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
circlesfont_Parameters circlesfont_CreateParameters_Ref(in float size)
{
    circlesfont_Parameters p;
    p.size = size;
    p.ascend = 1.f;
    p.descend = 0.5f;
    p.xheight = 0.5f;
    p.interchar = 0.5f;
    p.interline = 0.5f;
    p.thickness = 0.05f;
    p.thinness = 0.05f;
    p.oblique = 1.f;
    p.showdot = true;
    p.italic_inclination = 0.f;
    p.dotpos = 1.5f * p.xheight;
    p = circlesfont_ComputeParameters(p);
    return p;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
circlesfont_Parameters circlesfont_CreateParameters(in float size)
{
    circlesfont_Parameters p;
    p.size = size;
    p.ascend = 1.f;
    p.descend = 0.5f;
    p.xheight = 0.5f; // should be ~ 0.5
    p.interchar = 1.f - p.xheight;
    p.interline = 2.5f - p.descend - p.ascend;
    p.thickness = 0.1f;
    p.thinness = 0.02f;
    p.oblique = 1.f;
    p.showdot = true;
    p.italic_inclination = 0.f;
    p.dotpos = 1.5f * p.xheight;
    p = circlesfont_ComputeParameters(p);
    p = circlesfont_ComputeHintingAndParameters(p);
    circlesfont_Validate(p);
    return p;
}
//=============================================================================
// A and B are the corners of a bounding rectangle, the segment being cut by the horizontal edges.
void circlesfont_ObliqueSegmentGetEdges(in float2 A, in float2 B, in float thickness, inout float2 Ex, inout float2 Ey)
{
    // We assume that A.x < B.x
    // We seek a distance w so that the parallelogram ACBD has the expected thickness, l,
    // where C = A + (w,0)
    //       D = B - (w,0)
    // Let C' be the orthogonal projection of C on AD. Then, |CC'| = w.
    // AC' = dot(AC,AD)/|AD|^2 . AD
    // AD = AB - (w, 0),
    // |AD|^2 = (ABx - w)^2 + ABy^2
    // then AC' = dot((w,0), (ABx-w, ABy)) / |(ABx-w, ABy)|^2 * (ABx-w, ABy)
    //          = w*(ABx-w) / |(ABx-w, ABy)|^2 * (ABx-w, ABy)
    //          = (w*ABx - w^2) / |(ABx-w, ABy)|^2 * (ABx-w, ABy)
    // using pythagore in ACC':
    // |AC'|^2 = |AC|^2-|CC'|^2
    //         = w^2 - l^2
    //         = (w*ABx - w^2)^2 / |(ABx-w, ABy)|^4 * |(ABx-w, ABy)|^2
    //         = (w*ABx - w^2)^2 / |(ABx-w, ABy)|^2
    // <=> (w^2 - l^2) * (ABx^2 - 2 ABx w + w^2 + ABy^2) = (w*ABx - w^2)^2
    //     (w^2 - l^2) * (ABx^2 - 2 ABx w + w^2 + ABy^2) = w^2 ABx^2 - 2 w^3 ABx + w^4
    // <=>  0 =   w^4 * [ 1 - 1]                                ([...] = 0)
    //          + w^3 * [ - 2 ABx + 2 ABx ]                     ([...] = 0) => There should be a simpler computation ?
    //          + w^2 * [ ABx^2 + ABy^2 - l^2 - ABx^2]          ([...] = ABy^2 - l^2)
    //          + w   * [ 2 ABx l^2]
    //          +       [ - l^2 * ( ABx^2 + ABy^2) ]
    // Let's divide by l^2
    // a = ABy^2 / l^2 - 1
    // b = 2 ABx
    // c = -(ABx2 + ABy2)
    float2 AB = B-A;
    float ABx = AB.x;
    float ABx2 = ABx*ABx;
    float ABy2 = AB.y*AB.y;
    float l2 = thickness*thickness;
    float a = ABy2 / l2 - 1;
    float b = 2 * ABx;
    float c = -(ABx2 + ABy2);
    float delta = b*b-4*a*c;
    if(delta <= 0)
        return;

    float sqrtdelta = sqrt(delta);
    float w = (-b + sqrtdelta) / (2*a);
    Ex = float2(w, 0);
    Ey = AB - float2(w, 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity circlesfont_ObliqueSegment(in aa_Fragment frg, in float2 A, in float2 B, in float thickness)
{
    float2 Ex = float2(0, 0);
    float2 Ey = float2(0, 0);
    circlesfont_ObliqueSegmentGetEdges(A, B, thickness, Ex, Ey);
    return geom_Parallelogram_OriginEdges(frg, A, Ex, Ey );
}
//=============================================================================
// N is a unit vector corresponding to the normal of the edges that are of size thickness and thinness. 
aa_Intensity circlesfont_PinchedParallelogram(in aa_Fragment frg, in float2 A, in float2 B, in float2 N, in float thickness, in float thinness)
{
    float2 CP = frg.pos - A;
    float valueA = dot(CP, N);
    float valueB = valueA - dot(B-A, N);
    aa_Intensity i = aa_CreateIntensity(0.f);
    if(0 != frg.aaradius)
    {
        float delta = abs(dot(frg.dposdu, N)) + abs(dot(frg.dposdv, N));
        delta *= frg.aaradius;
        i = aa_xor(aa_Heaviside(valueA, delta), aa_Heaviside(valueB, delta));
    }
    else
    {
        float i1 = valueA > 0 ? 1 : 0;
        float i2 = valueB > 0 ? 1 : 0;
        i = aa_xor(aa_CreateIntensity(i1), aa_CreateIntensity(i2));
    }

    float2 Ny = float2(N.y, -N.x);
    float2 A0 = A-0.5f*thickness*Ny;
    float2 B0 = B-0.5f*thinness*Ny;
    float2 AB0 = B0-A0;
    float2 N0 = float2(AB0.y, -AB0.x);
    float2 A1 = A+0.5f*thickness*Ny;
    float2 B1 = B+0.5f*thinness*Ny;
    float2 AB1 = B1-A1;
    float2 N1 = float2(-AB1.y, AB1.x);
    i = aa_sub(i, geom_HalfPlane_PointNormal(frg, A0, N0));
    i = aa_sub(i, geom_HalfPlane_PointNormal(frg, A1, N1));
    return i;
}
//=============================================================================
aa_Intensity circlesfont_DrawGlyphBox(in aa_Fragment frg, in circlesfont_Parameters param)
{
    float2 min = float2(0, -param.ascend);
    float2 max = float2(param.width, 0);
    //float2 e = 0.01f * param.ascend * float2(1,1);
    float2 e = param.thinness * float2(1,1);
    aa_Intensity intensity = aa_sub(geom_Box_MinMax(frg, min, max), geom_Box_MinMax(frg, min+e, max-e));
    intensity.value *= 0.1f;
    return intensity;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity circlesfont_DrawGlyph(in aa_Fragment frg, in circlesfont_Parameters param, in uint c)
{
    aa_Intensity intensity = aa_CreateIntensity(0.f);

    const float2 C0 = param.C0;
    const float2 C1 = param.C1;
    const float2 C2 = param.C2;
    const float2 C3 = param.C3;
    const float2 C4 = param.C4;
    const float ellipseRatio = (param.radius - param.thinness) / (param.radius-param.thickness);

    const float overrun = param.width; // overrun distance when subtracting a geometry from another when generating concave figure.
    const float safeoblique = 1.f; // safe y/x parameter for a line passing from a circle center so that it doesn't cut other figures.
    if(c == a_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, 0)) );
    }
    else if(c == b_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
    }
    else if(c == c_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(-1, -1)),
                geom_HalfPlane_PointNormal(frg, C0, float2(-1, 1))
            )
        );
    }
    else if(c == d_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) );
    }
    else if(c == e_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, float2(C0.x,-param.e_hbar), float2(0, -1)),
                geom_HalfPlane_PointNormal(frg, C0, float2(-param.s_oblique, 1))
            )
        );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0.5f*param.thickness, -param.e_hbar-param.thinness), float2(param.width-0.5f*param.thickness, -param.e_hbar)) );
    }
    else if(c == f_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_sub( intensity, aa_inter(geom_HalfPlane_PointNormal(frg, C1, float2(-param.oblique, -1)), geom_HalfPlane_PointNormal(frg, C1, float2(-1,0))) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend+param.radius), float2(param.thickness, 0)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.radius), float2(param.width, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar-param.thinness), float2(param.radius, -param.t_hbar)) );
    }
    else if(c == g_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C2, param.radius) );
        intensity = aa_sub( intensity, aa_inter(geom_HalfPlane_PointNormal(frg, C2, float2(param.oblique, 1)), geom_HalfPlane_PointNormal(frg, C2, float2(1,0))) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(0, -overrun), float2(param.width-param.thickness, C2.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C2, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, param.descend-param.radius)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == h_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.radius), float2(param.width, 0)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.radius), float2(param.width-param.thickness, overrun)) );
    }
    else if(c == i_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.xheight), float2(param.i_pos+param.thickness, 0)) );
        if(param.showdot)
        {
            intensity = aa_union( intensity, geom_Box_MinMax(frg,
                float2(param.i_pos, -param.dotpos-param.thickness),
                float2(param.i_pos+param.thickness, -param.dotpos)) );
        }
    }
    else if(c == j_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C2, param.radius) );
        intensity = aa_sub( intensity, aa_inter(geom_HalfPlane_PointNormal(frg, C2, float2(param.oblique, 1)), geom_HalfPlane_PointNormal(frg, C2, float2(1,0))) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(0, -overrun), float2(param.width-param.thickness, C2.y)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, param.descend-param.radius)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C2, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(param.showdot)
        {
            intensity = aa_union( intensity, geom_Box_MinMax(frg,
                float2(param.width-param.thickness, -param.dotpos-param.thickness),
                float2(param.width, -param.dotpos)) );
        }
    }
    else if(c == k_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
        float2 Ex1 = float2(0,0);
        float2 Ey1 = float2(0,0);
        circlesfont_ObliqueSegmentGetEdges(float2(0,0), float2(param.width,-param.xheight), param.thinness, Ex1, Ey1);
        intensity = aa_union( intensity, geom_Parallelogram_OriginEdges(frg, float2(0,0), Ex1, Ey1) );
        float2 Ex2 = float2(0,0);
        float2 Ey2 = float2(0,0);
        circlesfont_ObliqueSegmentGetEdges(float2(0,-param.xheight), float2(param.width,0), param.thickness, Ex2, Ey2);
        intensity = aa_union( intensity,
            aa_sub(
                geom_Parallelogram_OriginEdges(frg, float2(0,-param.xheight), Ex2, Ey2 ),
                geom_HalfPlane_PointNormal(frg, float2(0,0), float2(-Ey1.y, Ey1.x))
            )
        );
        /*
        float2 A = float2(0, 1.5f);
        float2 B = A + float2(1,-1);
        float l = 0.05f;
        aa_Intensity iii = geom_Box_MinMax(frg, A, B );
        iii = aa_union( iii, geom_Box_MinMax(frg, A, A + float2(l, 0.1f) ) );
        iii.value *= 0.3f;
        iii = aa_union( iii, circlesfont_ObliqueSegment(frg, A, B, l ));
        intensity = aa_union( intensity, iii );
        */
    }
    else if(c == l_)
    {
#if 0
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
#else
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, -param.radius)) );
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, C0, float2(-1, 0)) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, float2(param.thickness, -param.radius), float2(-1, 0)),
                geom_HalfPlane_PointNormal(frg, float2(param.thickness, -param.radius), float2(0, 1))
            )
        );
#endif
    }
    else if(c == m_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.radius), float2(param.width, 0)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.radius), float2(param.width-param.thickness, overrun)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.xheight), float2(param.i_pos+param.thickness, 0)) );
    }
    else if(c == n_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.radius), float2(param.width, 0)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.radius), float2(param.width-param.thickness, overrun)) );
    }
    else if(c == o_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == p_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0.f, -param.xheight), float2(param.thickness, param.descend)) );
    }
    else if(c == q_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, param.descend)) );
    }
    else if(c == r_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity,
            aa_union(
                geom_HalfPlane_PointNormal(frg, C0, float2(0, -1)),
                geom_HalfPlane_PointNormal(frg, C0, float2(-param.oblique, -1))
            )
        );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, 0)) );
    }
    else if(c == s_)
    {
        intensity = aa_union( intensity, aa_inv(geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio)) );
        intensity = aa_sub( intensity,
            aa_xor(
                geom_HalfPlane_PointNormal(frg, float2(C0.x, -param.s_hbar-0.5f*param.thinness), float2(0, -1)),
                geom_HalfPlane_PointNormal(frg, float2(C0.x, -param.s_hbar-0.5f*param.thinness), float2(-param.s_oblique, -1))
            )
        );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(-overrun, -param.s_hbar-1.f*param.thinness), float2(param.width+overrun, -param.s_hbar)) );
        intensity = aa_inter( intensity, geom_Disc(frg, C0, param.radius) );
    }
    else if(c == t_)
    {
        float top = min(-param.ascend+param.radius, 0.5f*(C1.y+C4.y));
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(-1, 0)),
                geom_HalfPlane_PointNormal(frg, C0, float2(-param.oblique, 1))
            )
        );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend-overrun), float2(param.width+overrun, C0.y)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, top), float2(param.thickness, -param.radius)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar-param.thinness), float2(param.radius, -param.t_hbar)) );
    }
    else if(c == u_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.xheight-overrun), float2(param.width-param.thickness, -param.radius)) );
    }
    else if(c == v_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.width, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.xheight-overrun), float2(param.width-param.thickness, -param.radius)) );
    }
    else if(c == w_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.width, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.xheight-overrun), float2(param.width-param.thickness, -param.radius)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.xheight), float2(param.i_pos+param.thickness, -0.5f*param.thinness)) );
    }
    else if(c == x_)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,0), float2(param.width,-param.xheight), param.thickness ) );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,-param.xheight), float2(param.width,0), param.thinness ) );
    }
    else if(c == y_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C2, param.radius) );
        intensity = aa_sub( intensity, aa_inter(geom_HalfPlane_PointNormal(frg, C2, float2(param.oblique, 1)), geom_HalfPlane_PointNormal(frg, C2, float2(1,0))) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(0, -overrun), float2(param.width-param.thickness, C2.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C2, float2(param.radius-param.thickness, 0), ellipseRatio) );

        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, C2.y)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.xheight-overrun), float2(param.width-param.thickness, -param.radius)) );
    }
    else if(c == z_)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,0), float2(param.width,-param.xheight), param.thickness ) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.width-param.thinness, -param.xheight+param.thinness)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.thinness, -param.thinness), float2(param.width, 0)) );
    }
    else if(c == A_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, 0)) );

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, overrun)) );

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar-param.thinness), float2(param.width, -param.t_hbar)) );
    }
    else if(c == B_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.radius, 0)) );

        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius));
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Disc(frg, C4, param.radius)); }
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.thinness), float2(param.radius, -param.t_hbar-param.thinness)) );
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }
        if(C4.y < C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y-param.radius), float2(param.radius, 0)) ); }

        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius)); }

        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y), float2(param.width, C0.y)) ); }

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.t_hbar), float2(param.radius, -param.thinness)) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
    }
    else if(c == C_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );

        intensity = aa_sub( intensity,
            aa_inter(
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(-1, 0)),
                geom_HalfPlane_PointNormal(frg, C1, float2(-param.oblique, -1))
            ),
                geom_HalfPlane_PointNormal(frg, C0, float2(-param.oblique, 1))
            )
        );
    }
    else if(c == D_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(C0.x, 0)) ); 

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.thinness), float2(C0.x, -param.thinness)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == E_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) ); // v bar left
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, -param.ascend+param.thinness)) ); // top h bar
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.thinness), float2(param.width, 0)) ); // bottom h bar
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar-param.thinness), float2(param.radius, -param.t_hbar)) ); // middle h bar
    }
    else if(c == F_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) ); // v bar left
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, -param.ascend+param.thinness)) ); // top h bar
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar-param.thinness), float2(param.radius, -param.t_hbar)) ); // middle h bar
    }
    else if(c == G_)
    {
        aa_Intensity BoxIntensity = geom_Box_MinMax(frg, float2(0, -param.ascend+param.radius), float2(param.width, -param.radius));

        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        //if(C4.y >= C1.y) { intensity = aa_union( intensity, BoxIntensity ); }
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend+param.radius), float2(param.width, -param.radius)) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C1, float2(-safeoblique, 1)),
                geom_HalfPlane_PointNormal(frg, C1, float2(-param.oblique, -1))
            )
        );
        //if(C4.y < C1.y) { intensity = aa_union( intensity, BoxIntensity ); }
        if(C4.y < C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar), float2(param.width, -param.radius)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.radius), float2(param.width-param.thickness, -param.radius)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, C1, float2(param.width+overrun, -param.t_hbar-param.thinness)) );

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.radius, -param.t_hbar-param.thinness), float2(param.width, -param.t_hbar)) );
    }
    else if(c == H_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) ); // v bar left
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.t_hbar-param.thinness), float2(param.width, -param.t_hbar)) ); // middle h bar
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) ); // v bar right
    }
    else if(c == I_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.ascend), float2(param.i_pos+param.thickness, 0)) );
    }
    else if(c == J_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, -param.radius)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, -param.ascend-overrun), float2(param.width-param.thickness, -param.radius)) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(1, 0)),
                geom_HalfPlane_PointNormal(frg, C0, float2(param.oblique, 1))
            )
        );
    }
    else if(c == K_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius));
        intensity = aa_union( intensity, geom_Disc(frg, C4, param.radius));

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y), float2(param.width, 0)) );
        if(C4.y > -param.ascend) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, C4.y)) ); }
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.radius, 0)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend-overrun), float2(param.radius, -param.t_hbar-param.thinness)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.t_hbar), float2(param.radius, overrun)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend-overrun), float2(param.width-param.thickness, C4.y)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width-param.thickness, overrun)) );

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) );

        if(C4.y < C1.y) { intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, float2(0, -param.ascend), float2(0, 1)) ); }
    }
    else if(c == L_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) ); // v bar left
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.thinness), float2(param.width, 0)) ); // bottom h bar
    }
    else if(c == M_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C1+float2(0,-param.radius), param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1+float2(0,-param.radius), float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, C1+float2(0,-param.radius), float2(0, 1)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) );
    }
    else if(c == N_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) );
        float estimatedD = param.thickness-param.thinness; // correspond to the offset where the oblique line begins begins
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(estimatedD,-param.ascend), float2(param.width-estimatedD,0), param.thinness ) );
    }
    else if(c == O_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == P_)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        aa_Intensity Ellipse1intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio);
        aa_Intensity Ellipse4intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio);

        if(C4.y > C1.y) { intensity = aa_union( intensity, aa_union(Circle1intensity, Circle4intensity)); }
        else            { intensity = aa_union( intensity, aa_inter(Circle1intensity, Circle4intensity)); }

        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.radius, -param.t_hbar)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.thinness), float2(param.radius, -param.t_hbar-param.thinness)) );

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }
        if(C4.y > C1.y) { intensity = aa_sub( intensity, Ellipse4intensity ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, Ellipse1intensity ); }
        else             { intensity = aa_sub( intensity, aa_inter(Ellipse1intensity, Ellipse4intensity) ); }
    }
    else if(c == Q_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.radius, -param.thinness), float2(param.width, 0)) ); // bottom h bar
    }
    else if(c == R_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.radius, 0)) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius));
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Disc(frg, C4, param.radius)); }
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.thinness), float2(param.radius, -param.t_hbar-param.thinness)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C4.y < C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y-param.radius), float2(param.radius, 0)) ); }

        intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius));
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y), float2(param.width, 0)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.t_hbar), float2(param.radius, param.thickness)) );
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width-param.thickness, overrun)) );

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
    }
    else if(c == S_)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle3intensity = geom_Disc(frg, C3, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        intensity = aa_union( intensity, Circle1intensity);
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );

        intensity = aa_sub( intensity, aa_inter(aa_inv(Circle4intensity), geom_Box_MinMax(frg, float2(-overrun, C4.y), float2(C0))) );
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, C1, float2(param.width+overrun, C3.y))) ); }
        else { intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, float2(C1.x, -param.ascend-overrun), float2(param.width+overrun, C3.y))) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width+param.thickness, C4.y)) ); }
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        else  { intensity = aa_sub( intensity,
            aa_inter( geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio),
                      geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio)
            ));
        }
        if(C4.y < C1.y) {
            intensity = aa_union( intensity, aa_inter(
                aa_sub(Circle1intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio)),
                geom_Box_MinMax(frg, float2(C1.x-param.thickness, -param.ascend-overrun), float2(param.width+overrun, C1.y))
            ));
        }

#if 0 // oblique cut, if decided. For now, I feel it is better without
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C1+0.5f*(param.radius-param.thinness), float2(0, 1)),
                geom_HalfPlane_PointNormal(frg, C1, float2(-param.oblique, -1))
            )
        );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0-0.5f*(param.radius-param.thinness), float2(0, -1)),
                geom_HalfPlane_PointNormal(frg, C0, float2(param.oblique, 1))
            )
        );
#endif
    }
    else if(c == T_)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.ascend), float2(param.i_pos+param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, -param.ascend+param.thinness)) );
    }
    else if(c == U_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, -param.radius)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend-overrun), float2(param.width-param.thickness, -param.radius)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == V_)
    {
        float2 estimatedMiddle = float2(0.5f*(param.width+param.thinness-param.thickness),0);
        float2 Ex1 = float2(0,0);
        float2 Ey1 = float2(0,0);
        circlesfont_ObliqueSegmentGetEdges(float2(0,-param.ascend), estimatedMiddle, 0.5f*param.thinness, Ex1, Ey1 );
        intensity = aa_union( intensity, geom_Parallelogram_OriginEdges(frg, float2(0,-param.ascend), 2.f*Ex1, Ey1 ) );
        float2 Ex2 = float2(0,0);
        float2 Ey2 = float2(0,0);
        circlesfont_ObliqueSegmentGetEdges(float2(Ey1.x,0), float2(param.width,-param.ascend), param.thickness, Ex2, Ey2 );
        intensity = aa_union( intensity, geom_Parallelogram_OriginEdges(frg, float2(Ey1.x,0), Ex2, Ey2) );

        /*
        intensity = aa_union( intensity,  );
        // TODO: compensate thickness reduction due to orientation
        const float2 Ex1 = float2(param.thinness, 0);
        const float2 Ey1 = float2(0.5f*param.width - 0.5f*param.thickness, param.ascend);
        const float2 P1 = float2(0, -param.ascend) + 0.5f * Ex1 + 0.5f * Ey1;
        intensity = aa_union( intensity, geom_Parallelogram_CenterEdges(frg, P1, Ex1, Ey1 ) );
        const float2 Ex2 = float2(param.thickness, 0);
        const float2 Ey2 = float2(0.5f*param.width - 0.5f*param.thickness, -param.ascend);
        const float2 P2 = float2(param.width, -param.ascend) - 0.5f * Ex2 - 0.5f * Ey2;
        intensity = aa_union( intensity, geom_Parallelogram_CenterEdges(frg, P2, Ex2, Ey2 ) );*/
    }
    else if(c == W_)
    {
        intensity = aa_union( intensity,
            aa_sub(
                aa_sub(
                    geom_Disc(frg, C0+float2(0,param.radius), param.radius),
                    geom_EllipticDisc_CenterHalfAxisRatio(frg, C0+float2(0,param.radius), float2(param.radius-param.thickness, 0), ellipseRatio)
                ),
                geom_HalfPlane_PointNormal(frg, C0+float2(0,param.radius), float2(0, -1))
            )
        );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) );
    }
    else if(c == X_)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,0), float2(param.width, -param.ascend), param.thickness) );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0, -param.ascend), float2(param.width,0), param.thinness) );
    }
    else if(c == Y_)
    {
        float2 Ex = float2(0,0);
        float2 Ey = float2(0,0);
        circlesfont_ObliqueSegmentGetEdges( float2(0, 0), float2(param.width, -param.ascend), param.thickness, Ex, Ey );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0, -param.ascend), float2(param.width, 0), param.thinness) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, 0.5f*Ex, float2(Ey.y, -Ey.x) ) );

        intensity = aa_union( intensity, geom_Parallelogram_OriginEdges(frg, float2(0,0), Ex, Ey) );
    }
    else if(c == Z_)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,0), float2(param.width, -param.ascend), param.thickness) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width-0.75f*param.thickness, -param.ascend+param.thinness)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0.75f*param.thickness, -param.thinness), float2(param.width, 0)) );
    }
    else if(c == _0_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );

        intensity = aa_union( intensity, geom_Box_CenterDelta(frg, float2(0.5f*param.width, -0.5f*param.ascend), float2(param.thickness, param.radius)) );
    }
    else if(c == _1_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C1+float2(0,-param.radius), param.radius) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1+float2(0,-param.radius), float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, C1+float2(0,-param.radius), float2(0, 1)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, C1, float2(1, 0)) );
    }
    else if(c == _2_)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle3intensity = geom_Disc(frg, C3, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);

        intensity = aa_union( intensity, Circle1intensity);
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, 0)) );
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, float2(-overrun,C1.y), C3)) ); }
        else { intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, float2(-overrun, -param.ascend-overrun), C3)) ); }

        intensity = aa_sub( intensity, aa_inter(aa_inv(Circle4intensity), geom_Box_MinMax(frg, C4, float2(param.width+overrun, -param.thinness))) );
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.width-param.thickness, C4.y)) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        else  { intensity = aa_sub( intensity,
            aa_inter( geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio),
                      geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio)
            ));
        }
        if(C4.y < C1.y) {
            intensity = aa_union( intensity, aa_inter(
                aa_sub(Circle1intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio)),
                geom_Box_MinMax(frg, float2(-overrun, -param.ascend-overrun), float2(C1.x+param.thickness, C1.y))
            ));
        }
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width+overrun, -param.thinness)) );
    }
    else if(c == _3_)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        aa_Intensity Ellipse1intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio);
        aa_Intensity Ellipse4intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio);

        if(C4.y > C1.y) { intensity = aa_union( intensity, aa_union(Circle1intensity, Circle4intensity)); }
        else            { intensity = aa_union( intensity, aa_inter(Circle1intensity, Circle4intensity)); }

        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius)); }
        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y), float2(param.width, C0.y)) ); }
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, Ellipse4intensity ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, Ellipse1intensity ); }
        else             { intensity = aa_sub( intensity, aa_inter(Ellipse1intensity, Ellipse4intensity) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }

        intensity = aa_sub( intensity,
            aa_inter(
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(1, 0)),
                geom_HalfPlane_PointNormal(frg, C1, float2(param.oblique, -1))
            ),
                geom_HalfPlane_PointNormal(frg, C0, float2(param.oblique, 1))
            )
        );
        if(C4.y < C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, 0.5f*(C1.y+C4.y)), float2(param.radius, C3.y)) ); }
    }
    else if(c == _4_)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,-param.radius), float2(param.width,-param.ascend), param.thickness) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0.75f*param.thickness, -param.radius-param.thinness), float2(param.width, -param.radius)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.t_hbar-param.thinness), float2(param.width, 0)) );
    }
    else if(c == _5_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, C0.y)) );

        aa_Intensity Circle3intensity = geom_Disc(frg, C3, param.radius);
        intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, 0.5f*(C1+C4), float2(param.width+overrun, C3.y))) );

        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, -param.t_hbar), C0) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.thinness), float2(param.width+overrun, -param.t_hbar-param.thinness)) );

        //intensity = aa_sub( intensity,
        //    aa_inter(
        //        geom_HalfPlane_PointNormal(frg, C3, float2(0, -1)),
        //        geom_HalfPlane_PointNormal(frg, C0, float2(param.oblique, 1))
        //    )
        //);

        //intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend-overrun), float2(param.width+overrun, C1.y)) );
    }
    else if(c == _6_)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius));
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width+param.thickness, C4.y)) ); }
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C1, float2(-safeoblique, 1)),
                geom_HalfPlane_PointNormal(frg, C1, float2(-param.oblique, -1))
            )
        );

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.thickness, C0.y)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width+overrun, C0.y)) );
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius));

        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
    }
    else if(c == _7_)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,0), float2(param.width, -param.ascend), param.thickness) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width-0.75f*param.thickness, -param.ascend+param.thinness)) );
    }
    else if(c == _8_)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        aa_Intensity Ellipse1intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio);
        aa_Intensity Ellipse4intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio);

        if(C4.y > C1.y) { intensity = aa_union( intensity, aa_union(Circle1intensity, Circle4intensity)); }
        else            { intensity = aa_union( intensity, aa_inter(Circle1intensity, Circle4intensity)); }

        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius)); }
        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y), float2(param.width, C0.y)) ); }
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, Ellipse4intensity ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, Ellipse1intensity ); }
        else             { intensity = aa_sub( intensity, aa_inter(Ellipse1intensity, Ellipse4intensity) ); }
    }
    else if(c == _9_)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        aa_Intensity Ellipse1intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio);
        aa_Intensity Ellipse4intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio);

        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(1,0)),
                geom_HalfPlane_PointNormal(frg, C0, float2(param.oblique, 1))
            )
        );
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.width-param.thickness, C0.y)) ); }

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, C1.y), float2(param.width, C0.y)) );
        if(C4.y < C1.y)
        {
            intensity = aa_union( intensity, aa_sub( Circle1intensity,
                aa_union(geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.width-param.thickness, overrun)),
                    geom_HalfPlane_PointNormal(frg, C1, float2(1,0))) ) );
        }
        if(C4.y < C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.width-param.thickness, C0.y)) ); }
        if(C4.y < C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) ); }

        if(C4.y > C1.y) { intensity = aa_union( intensity, aa_union(Circle1intensity, Circle4intensity)); }
        else            { intensity = aa_union( intensity, aa_inter(Circle1intensity, Circle4intensity)); }
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }
        if(C4.y > C1.y) { intensity = aa_sub( intensity, Ellipse4intensity ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, Ellipse1intensity ); }
        else             { intensity = aa_sub( intensity, aa_inter(Ellipse1intensity, Ellipse4intensity) ); }

    }
    else if(c == UNICODE_EXCLAMATION_MARK)
    {
#if 1
        float2 A = float2(param.i_pos+0.5f*param.thickness, -param.ascend);
        float2 B = float2(param.i_pos+0.5f*param.thickness, -param.e_hbar);
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, A, B, float2(0,1), param.thickness, param.thinness) );
#else
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.e_hbar), float2(param.i_pos+param.thickness, -param.ascend)) );
#endif
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, 0), float2(param.i_pos+param.thickness, -param.thickness)) );
    }
    else if(c == UNICODE_QUOTATION_MARK)
    {
        float m = 0.25f * param.radius;
        float ht = 0.5f*param.thickness;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(m+ht, -param.ascend), float2(m+ht, -param.ascend+param.radius), float2(0,1), param.thickness, param.thinness) );
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(param.width-m-ht, -param.ascend), float2(param.width-m-ht, -param.ascend+param.radius), float2(0,1), param.thickness, param.thinness) );
    }
    else if(c == UNICODE_NUMBER_SIGN)
    {
        float a = 0.3f * (param.radius-param.thickness);
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(a, -1.5f*param.radius-0.5f*param.thinness), float2(param.width, -1.5f*param.radius+0.5f*param.thinness)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -0.5f*param.radius-0.5f*param.thinness), float2(param.width-a, -0.5f*param.radius+0.5f*param.thinness)) );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(param.thickness,0), float2(param.radius, -param.xheight), param.thickness) );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(param.radius,0), float2(param.width-param.thickness, -param.xheight), param.thickness) );
    }
    else if(c == UNICODE_DOLLAR_SIGN)
    {
        // Copy of S
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle3intensity = geom_Disc(frg, C3, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        intensity = aa_union( intensity, Circle1intensity);
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C0.y)) );

        intensity = aa_sub( intensity, aa_inter(aa_inv(Circle4intensity), geom_Box_MinMax(frg, float2(-overrun, C4.y), float2(C0))) );
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, C1, float2(param.width+overrun, C3.y))) ); }
        else { intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, float2(C1.x, -param.ascend-overrun), float2(param.width+overrun, C3.y))) ); }

        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width+param.thickness, C4.y)) ); }
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        else  { intensity = aa_sub( intensity,
            aa_inter( geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio),
                      geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio)
            ));
        }
        if(C4.y < C1.y) {
            intensity = aa_union( intensity, aa_inter(
                aa_sub(Circle1intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio)),
                geom_Box_MinMax(frg, float2(C1.x-param.thickness, -param.ascend-overrun), float2(param.width+overrun, C1.y))
            ));
        }
        // vertical line
        float dollarLineOverrun = 0.5f * param.radius;
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.ascend-dollarLineOverrun), float2(param.i_pos+param.thickness, dollarLineOverrun)) );
    }
    else if(c == UNICODE_PERCENT_SIGN)
    {
        float r = 0.65f*param.radius;
        float2 small_C0 = float2(param.width-r, -r);
        float2 small_C1 = float2(r, -param.ascend+r);
        float small_ellipseRatio = (r-param.thinness) / (r-param.thickness);
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,-param.radius), float2(param.width, -param.ascend+param.radius), param.thinness) );
        intensity = aa_union( intensity, geom_Disc(frg, small_C0, r));
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, small_C0, float2(r-param.thickness, 0), small_ellipseRatio) );
        intensity = aa_union( intensity, geom_Disc(frg, small_C1, r));
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, small_C1, float2(r-param.thickness, 0), small_ellipseRatio) );
    }
    else if(c == UNICODE_AMPERSAND)
    {
    /*
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius));
        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Disc(frg, C3, param.radius)); }
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius));
        if(C4.y != C1.y) { intensity = aa_union( intensity, geom_Disc(frg, C4, param.radius)); }

        if(C3.y != C0.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C3.y), float2(param.width, C0.y)) ); }
        if(C4.y != C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }

        if(C4.y != C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C1.y), float2(param.width-param.thickness, C4.y)) ); }
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width-param.thickness, C0.y)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C3.y != C0.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) ); }
        if(C4.y != C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }

        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius));
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, 0)) );

        aa_Intensity Circle3intensity = geom_Disc(frg, C3, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);

        intensity = aa_sub( intensity, aa_inter(aa_inv(Circle3intensity), geom_Box_MinMax(frg, float2(-overrun,C1.y), C3)) );
        intensity = aa_sub( intensity, aa_inter(aa_inv(Circle4intensity), geom_Box_MinMax(frg, C4, float2(param.width+overrun, -param.thinness))) );
        if(C4.y != C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.width-param.thickness, C4.y)) ); }

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C3, float2(param.radius-param.thickness, 0), ellipseRatio) );
        if(C4.y != C1.y) { intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio) ); }

        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, C3.y), float2(param.width+overrun, -param.thinness)) );
*/
    }
    else if(c == UNICODE_APOSTROPHE)
    {
        float ht = 0.5f*param.thickness;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(param.i_pos+ht, -param.ascend), float2(param.i_pos+ht, -param.ascend+param.radius), float2(0,1), param.thickness, param.thinness) );
    }
    else if(c == UNICODE_LEFT_PARENTHESIS) // ()()
    {
#if 1
        float2 A = float2(param.radius, -0.5f*param.ascend);
        float2 B = float2(param.width, 0);
        float2 C = float2(param.width, -param.ascend);
        float2 O;
        float r;
        geom_Disc3Points_ComputeCenterRadius(A, B, C, O, r);
        float2 Ap = A + float2(-param.thickness, 0);
        //float2 Bp = B + param.thinness * normalize(B-O);
        //float2 Cp = C + param.thinness * normalize(C-O);
        float2 Bp = B + float2(-param.thinness, 0);
        float2 Cp = C + float2(-param.thinness, 0);
        intensity = aa_union( intensity, geom_Disc3Points(frg, Ap, Bp, Cp) );
        intensity = aa_sub( intensity, geom_Disc(frg, O, r) );
        intensity = aa_inter( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, 0)) );
#else
        float2 C = float2(param.width, -0.5f*param.ascend);
        float extRatio = 0.5f*param.ascend / param.width;
        float inRatio = (0.5f*param.ascend-param.thinness) / (param.width-param.thickness);
        intensity = aa_union( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C, float2(param.width, 0), extRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C, float2(param.width-param.thickness, 0), inRatio) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, C, float2(-1, 0)) );
#endif
    }
    else if(c == UNICODE_RIGHT_PARENTHESIS)
    {
#if 1
        float2 A = float2(param.radius, -0.5f*param.ascend);
        float2 B = float2(0, 0);
        float2 C = float2(0, -param.ascend);
        float2 O;
        float r;
        geom_Disc3Points_ComputeCenterRadius(A, B, C, O, r);
        float2 Ap = A + float2(param.thickness, 0);
        //float2 Bp = B + param.thinness * normalize(B-O);
        //float2 Cp = C + param.thinness * normalize(C-O);
        float2 Bp = B + float2(param.thinness, 0);
        float2 Cp = C + float2(param.thinness, 0);
        intensity = aa_union( intensity, geom_Disc3Points(frg, Ap, Bp, Cp) );
        intensity = aa_sub( intensity, geom_Disc(frg, O, r) );
        intensity = aa_inter( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.width, 0)) );
#else
        float2 C = float2(0, -0.5f*param.ascend);
        float extRatio = 0.5f*param.ascend / param.width;
        float inRatio = (0.5f*param.ascend-param.thinness) / (param.width-param.thickness);
        intensity = aa_union( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C, float2(param.width, 0), extRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C, float2(param.width-param.thickness, 0), inRatio) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, C, float2(1, 0)) );
#endif
    }
    else if(c == UNICODE_ASTERISK)
    {
        float ht = 0.5f*param.thickness;
        float2 C = float2(param.i_pos+ht, -param.ascend + param.radius);
        float2 E0 = normalize(float2(1.f,1));
        float2 E1 = float2(-E0.x,E0.y);
        float2 E2 = normalize(float2(1,-0.5f));
        float2 E3 = float2(-E2.x,E2.y);
        float2 E4 = float2(0,-1);
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, C, C+param.radius*E0, E0, param.thickness, param.thinness) );
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, C, C+param.radius*E1, E1, param.thickness, param.thinness) );
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, C, C+param.radius*E2, E2, param.thickness, param.thinness) );
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, C, C+param.radius*E3, E3, param.thickness, param.thinness) );
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, C, C+param.radius*E4, E4, param.thickness, param.thinness) );
        intensity = aa_union( intensity, geom_Disc(frg, C, 0.2f*param.radius) );
    }
    else if(c == UNICODE_PLUS_SIGN)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.e_hbar-param.thinness), float2(param.width, -param.e_hbar)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.xheight), float2(param.i_pos+param.thickness, 0)) );
    }
    else if(c == UNICODE_COMMA)
    {
        float ht = 0.5f*param.thickness;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(param.i_pos+ht, -param.thickness), float2(param.i_pos+ht-param.thickness, param.radius-param.thickness), float2(0,1), param.thickness, param.thinness) );
    }
    else if(c == UNICODE_HYPHEN_MINUS)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.e_hbar-param.thinness), float2(param.width, -param.e_hbar)) );
    }
    else if(c == UNICODE_FULL_STOP)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, 0), float2(param.i_pos+param.thickness, -param.thickness)) );
    }
    else if(c == UNICODE_SOLIDUS)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,0), float2(param.width, -param.ascend), param.thinness) );
    }
    else if(c == UNICODE_COLON)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, 0), float2(param.i_pos+param.thickness, -param.thickness)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.e_hbar), float2(param.i_pos+param.thickness, -param.e_hbar-param.thickness)) );
    }
    else if(c == UNICODE_SEMICOLON)
    {
        float ht = 0.5f*param.thickness;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(param.i_pos+ht, -param.thickness), float2(param.i_pos+ht-param.thickness, param.radius-param.thickness), float2(0,1), param.thickness, param.thinness) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.e_hbar), float2(param.i_pos+param.thickness, -param.e_hbar-param.thickness)) );
    }
    else if(c == UNICODE_LESS_THAN_SIGN)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,-0.5f*param.ascend), float2(param.width, -param.ascend), param.thinness) );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,-0.5f*param.ascend), float2(param.width, 0), param.thinness) );
    }
    else if(c == UNICODE_EQUALS_SIGN)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -1.5f*param.radius-param.thinness), float2(param.width, -1.5f*param.radius)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -0.5f*param.radius), float2(param.width, -0.5f*param.radius+param.thinness)) );
    }
    else if(c == UNICODE_GREATER_THAN_SIGN)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0, -param.ascend), float2(param.width,-0.5f*param.ascend), param.thinness) );
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0, 0), float2(param.width,-0.5f*param.ascend), param.thinness) );
    }
    else if(c == UNICODE_QUESTION_MARK)
    {
        aa_Intensity Circle1intensity = geom_Disc(frg, C1, param.radius);
        aa_Intensity Circle4intensity = geom_Disc(frg, C4, param.radius);
        aa_Intensity Ellipse1intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio);
        aa_Intensity Ellipse4intensity = geom_EllipticDisc_CenterHalfAxisRatio(frg, C4, float2(param.radius-param.thickness, 0), ellipseRatio);

        if(C4.y > C1.y) { intensity = aa_union( intensity, aa_union(Circle1intensity, Circle4intensity)); }
        else            { intensity = aa_union( intensity, aa_inter(Circle1intensity, Circle4intensity)); }
        if(C4.y > C1.y) { intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, C1.y), float2(param.width, C4.y)) ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.i_pos+0.5f*param.thickness, 0)) ); }
        else             { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, 0.5f*(C1.y+C4.y)), float2(param.i_pos+0.5f*param.thickness, 0)) ); }

        float2 A = float2(param.i_pos+0.5f*param.thickness, -param.xheight-param.radius);
        float2 B = float2(param.i_pos+0.5f*param.thickness, -param.e_hbar);
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, A, B, float2(0,1), param.thickness, param.thinness) );

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.thickness), float2(param.i_pos+param.thickness, 0)) );

        if(C4.y > C1.y) { intensity = aa_sub( intensity, Ellipse4intensity ); }
        if(C4.y >= C1.y) { intensity = aa_sub( intensity, Ellipse1intensity ); }
        else             { intensity = aa_sub( intensity, aa_inter(Ellipse1intensity, Ellipse4intensity) ); }
        if(C4.y > C1.y) { intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, C1.y), float2(param.width-param.thickness, C4.y)) ); }

        if(C4.y < C1.y)
        {
            intensity = aa_union( intensity, aa_inter(
                aa_sub( Circle1intensity, Ellipse1intensity ),
                aa_inter(geom_HalfPlane_PointNormal(frg, C1, float2(0,1)), geom_HalfPlane_PointNormal(frg, C1, float2(1,1))) ));
        }
    }
    else if(c == UNICODE_COMMERCIAL_AT)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Disc(frg, C1, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend+param.radius), float2(param.width, -param.radius)) );

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.ascend+param.radius), float2(param.width-param.thickness, -param.radius)) );

        intensity = aa_sub( intensity,
            aa_inter(
                geom_HalfPlane_PointNormal(frg, C0, float2(-safeoblique, -1)),
                geom_HalfPlane_PointNormal(frg, C0, float2(-param.oblique, 1))
            )
        );

        float r = 0.65f*param.radius;
        float2 small_C0 = float2(param.width-r, -param.xheight);
        float small_ellipseRatio = (r-param.thinness) / (r-param.thickness);
        intensity = aa_union( intensity, geom_Disc(frg, small_C0, r));
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, small_C0, float2(r-param.thickness, 0), small_ellipseRatio) );
    }
    else if(c == UNICODE_LEFT_SQUARE_BRACKET)
    {
        //intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.ascend), float2(param.i_pos+0.5f*param.radius, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.ascend), float2(param.width, 0)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.i_pos+param.thickness, -param.ascend+param.thinness), float2(param.width+overrun, -param.thinness)) );
    }
    else if(c == UNICODE_REVERSE_SOLIDUS)
    {
        intensity = aa_union( intensity, circlesfont_ObliqueSegment(frg, float2(0,-param.ascend), float2(param.width,0), param.thinness) );
    }
    else if(c == UNICODE_RIGHT_SQUARE_BRACKET)
    {
        //intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos+param.thickness-0.5f*param.radius, -param.ascend), float2(param.i_pos+param.thickness, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.i_pos+param.thickness, 0)) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, -param.ascend+param.thinness), float2(param.i_pos, -param.thinness)) );
    }
    else if(c == UNICODE_CIRCUMFLEX_ACCENT)
    {
        float2 Ex;
        float2 Ey;
        circlesfont_ObliqueSegmentGetEdges(float2(param.i_pos, -param.dotpos), float2(param.width, -param.dotpos-0.5f*param.radius), param.thickness, Ex, Ey);
        float Px = param.i_pos+param.thickness-0.5f*Ex.x;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(Px, -param.dotpos-0.5f*param.radius), float2(0.5f*Ex.x, -param.dotpos), float2(0, 1), Ex.x, Ex.x*param.thinness/param.thickness) );
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(Px, -param.dotpos-0.5f*param.radius), float2(param.width-0.5f*Ex.x, -param.dotpos), float2(0, 1), Ex.x, Ex.x*param.thinness/param.thickness) );
    }
    else if(c == UNICODE_LOW_LINE)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, 0), float2(param.width, param.thinness)) );
    }
    else if(c == UNICODE_GRAVE_ACCENT)
    {
        float2 Ex;
        float2 Ey;
        circlesfont_ObliqueSegmentGetEdges(float2(param.i_pos, -param.dotpos), float2(param.width, -param.dotpos-0.5f*param.radius), param.thickness, Ex, Ey);
        float Px = param.i_pos+param.thickness-0.5f*Ex.x;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(param.width-0.5f*Ex.x, -param.dotpos-0.5f*param.radius), float2(Px, -param.dotpos), float2(0, 1), Ex.x, Ex.x*param.thinness/param.thickness) );
    }
    else if(c == UNICODE_LEFT_CURLY_BRACKET)
    {
        float radius = 0.75f*param.radius;
        float2 c0 = float2(param.i_pos+radius, -radius);
        float2 c1 = float2(param.i_pos+radius, -param.ascend+radius);
        float2 c2 = float2(param.i_pos+param.thickness-radius, -0.5f*param.ascend-0.5f*param.thinness+radius);
        float2 c3 = float2(param.i_pos+param.thickness-radius, -0.5f*param.ascend+0.5f*param.thinness-radius);
        intensity = aa_union( intensity, geom_Disc(frg, c0, radius) );
        intensity = aa_union( intensity, geom_Disc(frg, c1, radius) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.i_pos+param.thickness, c1.y), float2(param.width+overrun, c0.y)) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, c0, float2(-1,0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, c1.y), float2(param.i_pos+param.thickness, c0.y)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, c3, float2(param.i_pos+param.thickness, c2.y)) );

        intensity = aa_sub( intensity, aa_sub(
            geom_Box_MinMax(frg, c3, float2(param.width+overrun, c2.y)),
            aa_union(geom_Disc(frg, c2, radius), geom_Disc(frg, c3, radius)))
        );

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c0, float2(radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c1, float2(radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c2, float2(radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c3, float2(radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == UNICODE_VERTICAL_LINE)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, -param.ascend), float2(param.i_pos+param.thickness, 0)) );
    }
    else if(c == UNICODE_RIGHT_CURLY_BRACKET)
    {
        float radius = 0.75f*param.radius;
        float2 c0 = float2(param.i_pos+param.thickness-radius, -radius);
        float2 c1 = float2(param.i_pos+param.thickness-radius, -param.ascend+radius);
        float2 c2 = float2(param.i_pos+radius, -0.5f*param.ascend-0.5f*param.thinness+radius);
        float2 c3 = float2(param.i_pos+radius, -0.5f*param.ascend+0.5f*param.thinness-radius);
        intensity = aa_union( intensity, geom_Disc(frg, c0, radius) );
        intensity = aa_union( intensity, geom_Disc(frg, c1, radius) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(-overrun, c1.y), float2(param.i_pos, c0.y)) );
        intensity = aa_sub( intensity, geom_HalfPlane_PointNormal(frg, c0, float2(1,0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, c1.y), float2(param.i_pos+param.thickness, c0.y)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.i_pos, c3.y), c2) );

        intensity = aa_sub( intensity, aa_sub(
            geom_Box_MinMax(frg, float2(-overrun, c3.y), c2),
            aa_union(geom_Disc(frg, c2, radius), geom_Disc(frg, c3, radius)))
        );

        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c0, float2(radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c1, float2(radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c2, float2(radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, c3, float2(radius-param.thickness, 0), ellipseRatio) );
    }
    else if(c == UNICODE_TILDE)
    {
        float radius = 0.5f*param.radius;
        float2 c0 = float2(param.i_pos+param.thickness-radius, -param.dotpos);
        float2 c1 = float2(param.i_pos+radius, -param.dotpos);

        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.dotpos-radius), float2(param.width,-param.dotpos+radius)) );

        float a = 0.5f*(param.thickness-param.thinness);
        float r2 = radius - param.thickness + a;
        intensity = aa_sub( intensity, geom_Disc(frg, c0-float2(a,0), r2) );
        intensity = aa_sub( intensity, geom_Disc(frg, c1+float2(a,0), r2) );

        intensity = aa_sub( intensity, aa_sub(
            geom_Box_MinMax(frg, float2(-overrun, -param.dotpos-radius-overrun), float2(param.width+overrun,c0.y)),
            geom_Disc(frg, c0, radius) ));
        intensity = aa_sub( intensity, aa_sub(
            geom_Box_MinMax(frg, float2(-overrun, c0.y), float2(param.width+overrun,-param.dotpos+radius+overrun)),
            geom_Disc(frg, c1, radius) ));
    }
    else if(c == UNICODE_DIAERESIS)
    {
        intensity = aa_union( intensity, geom_Box_MinMax(frg,
            float2(param.i_pos-0.5f*param.radius, -param.dotpos-param.thickness),
            float2(param.i_pos-0.5f*param.radius+param.thickness, -param.dotpos)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg,
            float2(param.i_pos+0.5f*param.radius, -param.dotpos-param.thickness),
            float2(param.i_pos+0.5f*param.radius+param.thickness, -param.dotpos)) );
    }
    else if(c == UNICODE_ACUTE_ACCENT)
    {
        float2 Ex;
        float2 Ey;
        circlesfont_ObliqueSegmentGetEdges(float2(param.i_pos, -param.dotpos), float2(param.width, -param.dotpos-0.5f*param.radius), param.thickness, Ex, Ey);
        float Px = param.i_pos+param.thickness-0.5f*Ex.x;
        intensity = aa_union( intensity, circlesfont_PinchedParallelogram(frg, float2(0.5f*Ex.x, -param.dotpos-0.5f*param.radius), float2(Px, -param.dotpos), float2(0, 1), Ex.x, Ex.x*param.thinness/param.thickness) );

    }
    else if(c == UNICODE_MICRO_SIGN)
    {
        intensity = aa_union( intensity, geom_Disc(frg, C0, param.radius) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, 0)) );
        intensity = aa_union( intensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, param.descend)) );
        intensity = aa_sub( intensity, geom_EllipticDisc_CenterHalfAxisRatio(frg, C0, float2(param.radius-param.thickness, 0), ellipseRatio) );
        intensity = aa_sub( intensity, geom_Box_MinMax(frg, float2(param.thickness, -param.xheight-overrun), float2(param.width-param.thickness, -param.radius)) );
    }
    else if(c == UNICODE_MULTIPLICATION_SIGN)
    {
        float sqrt2 = sqrt(2);
        float oosqrt2 = 1.f/sqrt2;
        float2 C = float2(param.radius, -param.radius);
        float2 Ex = float2(param.width-param.thickness, param.width-param.thickness);
        float2 Ey = oosqrt2 * float2(-param.thickness, param.thickness);
        intensity = aa_union( intensity, geom_Parallelogram_CenterEdges(frg, C, Ex, Ey) );
        intensity = aa_union( intensity, geom_Parallelogram_CenterEdges(frg, C, float2(Ex.y, -Ex.x), float2(Ey.y, -Ey.x)) );
    }

    //intensity = aa_union( intensity, circlesfont_DrawGlyphBox(frg, param) );
    return intensity;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity circlesfont_DrawGlyph(in aa_Fragment frg, in circlesfont_Parameters param, in float2 P, in uint c)
{
    frg = add(frg, -P);
    frg = mul(frg, 1.f/param.size);
    float2x2 m = {
        1, param.italic_inclination,
        0, 1
    };
    frg = mul(m, frg);
    frg = add(frg, -param.offset);
    param.size = 1.f;
    return circlesfont_DrawGlyph(frg, param, c);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity circlesfont_DrawTests(in aa_Fragment frg, in circlesfont_Parameters param, in float2 P)
{
    aa_Intensity intensity = aa_CreateIntensity(0.f);

    frg = add(frg, -P);
    frg = mul(frg, 1.f/param.size);
    param.size = 1.f;

    if(false) // bounding box
    {
        float2 min = float2(0, -param.ascend);
        float2 max = float2(param.width, 0);
        float2 e = 0.01f * param.ascend * float2(1,1);
        aa_Intensity bbIntensity = aa_sub(geom_Box_MinMax(frg, min, max), geom_Box_MinMax(frg, min+e, max-e));
        bbIntensity.value *= 0.02f;
        intensity = aa_union(intensity, bbIntensity);
    }
    if(false) // lines
    {
        aa_Intensity linesIntensity = aa_CreateIntensity(0.f);
        float e = 0.01f * param.ascend;
        float2 C0 = float2(0.5f * param.width, -param.radius);
        linesIntensity = aa_union( linesIntensity, geom_Circle(frg, C0, param.radius, e) );
        linesIntensity = aa_union( linesIntensity, geom_Circle(frg, C0, param.radius-param.thickness, e) );
        float2 C1 = float2(0.5f * param.width, -param.ascend+param.radius);
        linesIntensity = aa_union( linesIntensity, geom_Circle(frg, C1, param.radius, e) );
        linesIntensity = aa_union( linesIntensity, geom_Circle(frg, C1, param.radius-param.thickness, e) );
        float2 C2 = float2(0.5f * param.width, param.descend-param.radius);
        linesIntensity = aa_union( linesIntensity, geom_Circle(frg, C2, param.radius, e) );
        linesIntensity = aa_union( linesIntensity, geom_Circle(frg, C2, param.radius-param.thickness, e) );
        linesIntensity.value *= 0.02f;
        intensity = aa_union(intensity, linesIntensity);
    }
    if(false) // test forms
    {
        aa_Intensity bfIntensity = aa_CreateIntensity(0.f);
        float2 C0 = float2(0.5f * param.width, -param.radius);
        //bfIntensity = aa_union( bfIntensity, aa_sub(geom_Disc(frg, C0, param.radius), geom_Disc(frg, C0, param.radius-param.thickness)) );
        float2 C1 = float2(0.5f * param.width, -param.ascend+param.radius);
        //bfIntensity = aa_union( bfIntensity, aa_sub(geom_Disc(frg, C1, param.radius), geom_Disc(frg, C1, param.radius-param.thickness)) );
        #if 0
        bfIntensity = aa_union( bfIntensity,
            aa_sub(
                aa_sub( geom_Disc(frg, C1, param.radius), geom_Disc(frg, C1, param.radius-param.thickness)),
                geom_HalfPlane_PointNormal(frg, float2(0, -param.ascend+param.radius), float2(0, -1))
            )
        );
        #endif
        #if 1
        float ellipseRatio = (param.radius - param.thinness) / (param.radius-param.thickness);
        bfIntensity = aa_union( bfIntensity,
            aa_sub(
                aa_sub( geom_Disc(frg, C1, param.radius), geom_EllipticDisc_CenterHalfAxisRatio(frg, C1, float2(param.radius-param.thickness, 0), ellipseRatio)),
                geom_HalfPlane_PointNormal(frg, float2(0, -param.ascend+param.radius), float2(0, -1))
            )
        );
        #endif
        #if 0
        bfIntensity = aa_union( bfIntensity,
            aa_sub(
                aa_sub( geom_Disc(frg, C1, param.radius), geom_Disc(frg, C1, param.radius-param.thickness)),
                aa_union( geom_HalfPlane_PointNormal(frg, float2(0, -param.ascend+param.radius), float2(0, -1)),
                          geom_HalfPlane_PointNormal(frg, float2(param.radius, -param.ascend+param.radius), float2(-1, -1)) )
            )
        );
        #endif
        float2 C2 = float2(0.5f * param.width, param.descend-param.radius);
        //bfIntensity = aa_union( bfIntensity, aa_sub(geom_Disc(frg, C2, param.radius), geom_Disc(frg, C2, param.radius-param.thickness)) );

        //bfIntensity = aa_union( bfIntensity, geom_Box_MinMax(frg, float2(0, -param.xheight), float2(param.thickness, 0)) );
        bfIntensity = aa_union( bfIntensity, geom_Box_MinMax(frg, float2(0, -param.ascend+param.radius), float2(param.thickness, 0)) );
        //bfIntensity = aa_union( bfIntensity, geom_Box_MinMax(frg, float2(0, -param.ascend), float2(param.thickness, 0)) );

        //bfIntensity = aa_union( bfIntensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.xheight), float2(param.width, 0)) );
        //bfIntensity = aa_union( bfIntensity, geom_Box_MinMax(frg, float2(param.width-param.thickness, -param.ascend), float2(param.width, 0)) );

        bfIntensity = aa_union( bfIntensity, geom_Box_MinMax(frg, float2(0, -param.xheight-param.thickness), float2(param.radius, -param.xheight)) );

        intensity = aa_union(intensity, bfIntensity);
    }

    float2 pos = float2(0, 0);
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), a_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), b_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), c_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), d_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), e_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), f_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), g_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), h_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), i_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), j_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), k_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), l_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), m_)); pos.x += 1;
    //pos.y += 1;  pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), n_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), o_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), p_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), q_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), r_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), s_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), t_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), u_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), v_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), w_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), x_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), y_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), z_)); pos.x += 1;
    //pos.y += 1; pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), A_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), B_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), C_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), D_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), E_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), F_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), G_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), H_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), I_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), J_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), K_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), L_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), M_)); pos.x += 1;
    //pos.y += 1;  pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), N_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), O_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), P_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), Q_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), R_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), S_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), T_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), U_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), V_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), W_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), X_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), Y_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), Z_)); pos.x += 1;
    //pos.y += 1;  pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _0_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _1_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _2_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _3_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _4_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _5_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _6_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _7_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _8_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), _9_)); pos.x += 1;
    //pos.y += 1;  pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_EXCLAMATION_MARK )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_QUOTATION_MARK   )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_NUMBER_SIGN      )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_DOLLAR_SIGN      )); pos.x += 1; 
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_PERCENT_SIGN     )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_AMPERSAND        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_APOSTROPHE       )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_LEFT_PARENTHESIS )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_RIGHT_PARENTHESIS)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_ASTERISK         )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_PLUS_SIGN        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_COMMA            )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_HYPHEN_MINUS     )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_FULL_STOP        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_SOLIDUS          )); pos.x += 1;
    //pos.y += 1;  pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_COLON               )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_SEMICOLON           )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_LESS_THAN_SIGN      )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_EQUALS_SIGN         )); pos.x += 1; 
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_GREATER_THAN_SIGN   )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_QUESTION_MARK       )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_COMMERCIAL_AT       )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_LEFT_SQUARE_BRACKET )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_REVERSE_SOLIDUS     )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_RIGHT_SQUARE_BRACKET)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_CIRCUMFLEX_ACCENT   )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_LOW_LINE            )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_GRAVE_ACCENT        )); pos.x += 1;
    //pos.y += 1;  pos.x = 0;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_LEFT_CURLY_BRACKET  )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_VERTICAL_LINE       )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_RIGHT_CURLY_BRACKET )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_TILDE               )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_DIAERESIS           )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_ACUTE_ACCENT        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_MICRO_SIGN          )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_MULTIPLICATION_SIGN )); pos.x += 1;

    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_GRAVE_ACCENT        ));
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), e_        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_CIRCUMFLEX_ACCENT        ));
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), a_        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), i_)); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_DIAERESIS        ));
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), e_        )); pos.x += 1;
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_TILDE        ));
    //intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), n_        )); pos.x += 1;

    //pos.y += 1;  pos.x = 0;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_QUOTATION_MARK)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), H_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), e_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), l_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), l_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), o_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_SPACE)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), w_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), o_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), r_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), l_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), d_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_EXCLAMATION_MARK)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_QUOTATION_MARK)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_SPACE)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_ASTERISK)); pos.x += 1;
    pos.y += 1;  pos.x = 0;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_LEFT_PARENTHESIS)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_ASTERISK)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_RIGHT_PARENTHESIS)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), B_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), o_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), n_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), j_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), o_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), u_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), r_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_SPACE)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), l_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), e_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_SPACE)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), m_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), o_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), n_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), d_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), e_)); pos.x += 1;
    intensity = aa_union(intensity, circlesfont_DrawGlyph(frg, param, pos * float2(param.advance, param.lineheight), UNICODE_EXCLAMATION_MARK)); pos.x += 1;

    return intensity;
}
//=============================================================================

#endif

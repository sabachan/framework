#ifndef Geometry_HLSL
#define Geometry_HLSL

#include "AntiAliasing.hlsl"
#include "Math.hlsl"

//=============================================================================
aa_Intensity geom_HalfPlane_PointNormal(in aa_Fragment frg, in float2 O, in float2 N)
{
    float value = dot(frg.pos - O, N);
    if(0 != frg.aaradius)
    {
        float delta = abs(dot(frg.dposdu, N)) + abs(dot(frg.dposdv, N));
        delta *= frg.aaradius;
        aa_Intensity i = aa_Heaviside(-value, delta);
        return i;
    }
    else
    {
        aa_Intensity i = aa_CreateIntensity(value > 0 ? 0 : 1);
        return i;
    }
}
//=============================================================================
// NB: also works if N is not normalised, but width is relative to normal length.
aa_Intensity geom_Line_PointNormal_AssumeNormalised(in aa_Fragment frg, in float2 O, in float2 N, in float width)
{
    float value = dot(frg.pos - O, N);
    if(0 != frg.aaradius)
    {
        float delta = abs(dot(frg.dposdu, N)) + abs(dot(frg.dposdv, N));
        delta *= frg.aaradius;
        float oohwidth = 2.f / width;
        aa_Intensity i = aa_CenteredUnitRect(value * oohwidth, delta * oohwidth);
        return i;
    }
    else
    {
        float i1 = value + 0.5f*width > 0 ? 1 : 0;
        float i2 = value - 0.5f*width > 0 ? 1 : 0;
        aa_Intensity i = aa_CreateIntensity(i1 - i2);
        return i;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Line_PointNormal(in aa_Fragment frg, in float2 O, in float2 N, in float width)
{
    float2 nN = normalize(N);
    return geom_Line_PointNormal_AssumeNormalised(frg, O, nN, width);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Line_PointDir(in aa_Fragment frg, in float2 O, in float2 D, in float width)
{
    return geom_Line_PointNormal(frg, O, float2(-D.y, D.x), width);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Line_2Points(in aa_Fragment frg, in float2 A, in float2 B, in float width)
{
    float2 D = B - A;
    return geom_Line_PointNormal(frg, A, float2(-D.y, D.x), width);
}
//=============================================================================
aa_Intensity geom_Segment(in aa_Fragment frg, in float2 A, in float2 B, in float width)
{
    aa_Intensity i = geom_Line_2Points(frg, A, B, width);

    float2 D = B - A;
    float2 d = normalize(D);
    float2 AP = frg.pos - A;
    float value = dot(AP, D);
    float D2 = dot(D, D);
    if(0 != frg.aaradius)
    {
        float delta = abs(dot(frg.dposdu, D)) + abs(dot(frg.dposdv, D));
        delta *= frg.aaradius;
        aa_Intensity j = aa_xor(aa_Heaviside(value, delta), aa_Heaviside(value - D2, delta));
        return aa_unaligned_inter(i,j);
    }
    else
    {
        float i1 = value > 0 ? 1 : 0;
        float i2 = value > D2 ? 1 : 0;
        aa_Intensity j = aa_aligned_xor(aa_CreateIntensity(i1), aa_CreateIntensity(i2));
        return aa_unaligned_inter(i,j);
    }
}
//=============================================================================
aa_Intensity geom_Parallelogram_CenterEdges(in aa_Fragment frg, in float2 C, in float2 Ex, in float2 Ey)
{
    float2 Nx = float2(Ey.y, -Ey.x);
    float2 Ny = float2(-Ex.y, Ex.x);
    float2 CP = frg.pos - C;
    float valuex = dot(CP, Nx);
    float valuey = dot(CP, Ny);
    float hENx = 0.5f * dot(Ex, Nx);
    float hENy = 0.5f * dot(Ey, Ny);

    if(0 != frg.aaradius)
    {
        float deltax = abs(dot(frg.dposdu, Nx)) + abs(dot(frg.dposdv, Nx));
        deltax *= frg.aaradius;
        float deltay = abs(dot(frg.dposdu, Ny)) + abs(dot(frg.dposdv, Ny));
        deltay *= frg.aaradius;
        aa_Intensity ix = aa_aligned_xor(aa_Heaviside(valuex + hENx, deltax), aa_Heaviside(valuex - hENx, deltax));
        aa_Intensity iy = aa_aligned_xor(aa_Heaviside(valuey + hENy, deltay), aa_Heaviside(valuey - hENy, deltay));
        aa_Intensity i = aa_unaligned_inter(ix, iy);
        return i;
    }
    else
    {
        float i1 = valuex + hENx > 0 ? 1 : 0;
        float i2 = valuex - hENx > 0 ? 1 : 0;
        float i3 = valuey + hENy > 0 ? 1 : 0;
        float i4 = valuey - hENy > 0 ? 1 : 0;
        aa_Intensity ix = aa_aligned_xor(aa_CreateIntensity(i1), aa_CreateIntensity(i2));
        aa_Intensity iy = aa_aligned_xor(aa_CreateIntensity(i3), aa_CreateIntensity(i4));
        aa_Intensity i = aa_unaligned_inter(ix, iy);
        return i;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Parallelogram_OriginEdges(in aa_Fragment frg, in float2 O, in float2 Ex, in float2 Ey)
{
    return geom_Parallelogram_CenterEdges(frg, O+0.5f*Ex+0.5f*Ey, Ex, Ey);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Rect_CenterAxisRatio(in aa_Fragment frg, in float2 C, in float2 Ex, in float ratio)
{
    return geom_Parallelogram_CenterEdges(frg, C, Ex, ratio * float2(Ex.y, -Ex.x));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Box_CenterDelta(in aa_Fragment frg, in float2 C, in float2 delta)
{
    return geom_Parallelogram_CenterEdges(frg, C, float2(delta.x, 0), float2(0, delta.y));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Box_MinMax(in aa_Fragment frg, in float2 Min, in float2 Max)
{
    return geom_Box_CenterDelta(frg, 0.5f*(Min+Max), Max-Min);
}
//=============================================================================
aa_Intensity geom_Disc(in aa_Fragment frg, in float2 C, in float R)
{
    float2 CP = frg.pos - C;
    if(aa_true == frg.aamethod.useDistanceFieldIFP || (aa_undefined == frg.aamethod.useDistanceFieldIFP && 2 < frg.aaradius))
    {
        float d2 = dot(CP,CP);
        if(d2 < math_sq(0.001 * R))
        {
            // TODO: do correct computation for case when R is small relatively
            // to pixel size
            aa_Intensity i = aa_CreateIntensity(1);
            return i;
        }
        float d = sqrt(d2);
        float value = d - R;
        // f(X) = sqrt((X-C)*(X-C)) - R
        // f'(X) = 1/2*(X-C)*1/sqrt((X-C)*(X-C))
        float2 grad = 1.f/d * CP;
        float delta;
        if(aa_true == frg.aamethod.useEllipticSamplingIFP || aa_undefined == frg.aamethod.useEllipticSamplingIFP)
            delta = sqrt( math_sq(dot(frg.dposdu, grad)) + math_sq(dot(frg.dposdv, grad)));
        else
            delta = abs(dot(frg.dposdu, grad)) + abs(dot(frg.dposdv, grad));
        delta *= frg.aaradius;
        aa_Intensity i = aa_Heaviside(-value, delta);
        return i;
    }
    else
    {
        float value = dot(CP,CP) - R*R;
        if(0 != frg.aaradius)
        {
            // f(X) = (X-C)*(X-C) - R*R
            // f'(X) = 2*X - 2*C
            float2 grad = 2 * CP;
            float delta;
            if(aa_true == frg.aamethod.useEllipticSamplingIFP || aa_undefined == frg.aamethod.useEllipticSamplingIFP)
                delta = sqrt( math_sq(dot(frg.dposdu, grad)) + math_sq(dot(frg.dposdv, grad)));
            else
                delta = abs(dot(frg.dposdu, grad)) + abs(dot(frg.dposdv, grad));
            delta *= frg.aaradius;
            aa_Intensity i = aa_Heaviside(-value, delta);
            return i;
        }
        else
        {
            float i = value > 0 ? 0 : 1;
            return aa_CreateIntensity(i);
        }
    }
}
//=============================================================================
aa_Intensity geom_EllipticDisc_CenterHalfAxisRatio(in aa_Fragment frg, in float2 C, in float2 Ex, in float ratio)
{
    float2 Ey = float2(Ex.y, -Ex.x);
    float normSqEx = dot(Ex, Ex);
    float oonormSqEx = 1.f/normSqEx;
    float ooratio = 1.f/ratio;
    float2x2 m = {
        Ex.x, Ex.y,
        Ey.x, Ey.y
    };
    frg = add( frg, -C );
    frg = mul( m, frg );
    frg = mul( frg, oonormSqEx );
    frg = mul(frg, float2(1, ooratio));
    return geom_Disc(frg, float2(0,0), 1);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void geom_Disc3Points_ComputeCenterRadius(in float2 A, in float2 B, in float2 C, out float2 O, out float r)
{
    // We search for a circle (O,r) such that A, B and C belong to it, ie
    //      |A-O| = r, |B-O| = r, and |C-O| = r,
    // Then O belongs to the bisections of AB, AC and BC.
    // The bisection of AB verify the eq:
    //      AB.AO = 1/2 * AB.AB
    // (Bx-Ax) (Ox-Ax) + (By-Ay) (Oy-Ay) = 1/2 |AB|²
    // (Cx-Ax) (Ox-Ax) + (Cy-Ay) (Oy-Ay) = 1/2 |AC|²
    // It has the form
    //      a x + b y = c
    //      d x + e y = f
    // by Cramer's rule,
    //      x = | c  b | / | a  b |
    //          | f  e |   | d  e |
    float2 AB = B-A;
    float2 AC = C-A;
    float a = AB.x;
    float b = AB.y;
    float c = 0.5f*dot(AB,AB) + dot(AB,A);
    float d = AC.x;
    float e = AC.y;
    float f = 0.5f*dot(AC,AC) + dot(AC,A);
    float det = a*e - d*b;
    float detx = c*e - f*b;
    float dety = a*f - d*c;
    O = 1.f/det * float2(detx, dety);
    float2 OA = A-O;
    r = sqrt(dot(OA,OA));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
aa_Intensity geom_Disc3Points(in aa_Fragment frg, in float2 A, in float2 B, in float2 C)
{
    float2 O;
    float r;
    geom_Disc3Points_ComputeCenterRadius(A, B, C, O, r);
    return geom_Disc(frg, O, r);
}
//=============================================================================
aa_Intensity geom_Circle(in aa_Fragment frg, in float2 C, in float R, in float width)
{
    float2 CP = frg.pos - C;
    float hWidth = 0.5f * width;
    if(aa_true == frg.aamethod.useDistanceFieldIFP || (aa_undefined == frg.aamethod.useDistanceFieldIFP && 2 < frg.aaradius))
    {
        float oohWidth = 1.f / hWidth;
        float d = sqrt(dot(CP,CP));
        float value = d - R;
        // f(X) = sqrt((X-C)*(X-C)) - R
        // f'(X) = 1/2*(X-C)*1/sqrt((X-C)*(X-C))
        float2 grad = 1.f/d * CP;
        float delta;
        if(aa_true == frg.aamethod.useEllipticSamplingIFP || aa_undefined == frg.aamethod.useEllipticSamplingIFP)
            delta = sqrt( dot(frg.dposdu, grad) * dot(frg.dposdu, grad) + dot(frg.dposdv, grad) * dot(frg.dposdv, grad));
        else
            delta = abs(dot(frg.dposdu, grad)) + abs(dot(frg.dposdv, grad));
        delta *= frg.aaradius;
        aa_Intensity i = aa_CenteredUnitRect(value * oohWidth, delta * oohWidth);
        return i;
    }
    else
    {
        float valuein = dot(CP,CP) - (R-hWidth)*(R-hWidth);
        float valueout = dot(CP,CP) - (R+hWidth)*(R+hWidth);
        if(0 != frg.aaradius)
        {
            // f(X) = (X-C)*(X-C) - R*R
            // f'(X) = 2*X - 2*C
            float2 grad = 2 * CP;
            float delta;
            if(aa_true == frg.aamethod.useEllipticSamplingIFP || aa_undefined == frg.aamethod.useEllipticSamplingIFP)
                delta = sqrt( dot(frg.dposdu, grad) * dot(frg.dposdu, grad) + dot(frg.dposdv, grad) * dot(frg.dposdv, grad));
            else
                delta = abs(dot(frg.dposdu, grad)) + abs(dot(frg.dposdv, grad));
            delta *= frg.aaradius;
            aa_Intensity i = aa_sub( aa_Heaviside(-valueout, delta), aa_Heaviside(-valuein, delta) );
            return i;
        }
        else
        {
            float i = valuein > 0 && valueout < 0 ? 1 : 0;
            return aa_CreateIntensity(i);
        }
    }
}
//=============================================================================

#endif

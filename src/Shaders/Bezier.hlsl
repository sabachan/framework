#ifndef Bezier_HLSL
#define Bezier_HLSL

#include "AntiAliasing.hlsl"
#include "Geometry.hlsl"
#include "Math.hlsl"

//=============================================================================
float2 bezier_ComputeDistanceVectorToQuadratic(in float2 P0, in float2 P1, in float2 P2, in float2 P)
{
    // How to compute distance from P to curve B(t) ?
    // 1- We can try to find t such that d(P,B(t)) is minimal.
    //    In that case dot(dB/dt, PB(t)) = 0
    // B(t) = P0 * (1-t)² + 2 * P1 * t*(1-t) + P2 * t^2
    //      = (P0 - 2*P1 + P2) * t^2 + (-2*P0 + 2*P1) * t + P0
    // PB(t) = B(t) - P 
    //       = (P0 - 2*P1 + P2) * t^2 + (-2*P0 + 2*P1) * t + P0-P
    //       = U2 * t^2 + U1 * t + U0
    // dB/dt = 2 * U2 * t + U1
    //       = V1 * t + V0
    // dot(dB/dt, B(t)P) = U2.V1 * t^3 + (U2.V0 + U1.V1) t^2 + (U1.V0 + U0.V1) * t + U0.V0
    //                   = a * t^3 + b * t^2 + c * t + d
    //                   = F(t)
    // We need to find the zero of this function F(t).
    // dF/dt = 3 * a * t^2 + 2 * b * t + c
    float2 U2 = P0 - 2*P1 + P2;
    float2 U1 = -2*P0 + 2*P1;
    float2 U0 = P0-P;
    float2 V1 = 2 * U2;
    float2 V0 = U1;
    float a = dot(U2,V1);
    float b = dot(U2,V0) + dot(U1,V1);
    float c = dot(U1,V0) + dot(U0,V1);
    float d = dot(U0,V0);

    float t = 0.5f;
    if(false)
    {
        float2 P0P = P-P0;
        float2 P0P2 = P2-P0;
        t = dot(P0P, P0P2) / dot(P0P2, P0P2);
    }
    else if(true)
    {
        float2 M = 0.5f * (P0+P2);
        float2 P1M = M-P1;
        float2 orthoP1M = float2(P1M.y, -P1M.x);
        float2 P0P = P-P0;
        float2 P0P2 = P2-P0;
        t = dot(P0P, orthoP1M) / dot(P0P2, orthoP1M);
    }

    //int N = 4;
    //for(int i = 0; i < N; i ++)
    {
        float F = a * t*t*t + b * t*t + c*t + d;
        float dFdt = 3*a * t*t + 2 * b * t + c;
        t = t - F / dFdt;
    }

    float2 B = P0 * (1-t)*(1-t) + 2 * P1 * t*(1-t) + P2 * t*t;
    float2 PB = B-P;
    return PB;
}
//=============================================================================
float2 bezier_ComputeDistanceVectorToCubic(in float2 P0, in float2 P1, in float2 P2, in float2 P3, in float2 P)
{
    // TODO
}
//=============================================================================

#endif
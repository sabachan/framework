#ifndef PBR_HLSL
#define PBR_HLSL

// Physically-Based Rendering

#include "math.hlsl"

//=============================================================================
struct pbr_DirectionalLight
{
    float3 direction;
    float3 color;
};
pbr_DirectionalLight pbr_CreateDirectionalLight()
{
    pbr_DirectionalLight l;
    l.direction = float3(0,0,1);
    l.color = float3(1,1,1);
    return l;
}
//=============================================================================
struct pbr_AmbiantLight
{
    float3 color;
};
pbr_AmbiantLight pbr_CreateAmbiantLight()
{
    pbr_AmbiantLight l;
    l.color = float3(1,1,1);
    return l;
}
//=============================================================================
struct pbr_Material
{
    float3 albedo;
    float3 baseReflectivity;
    float roughness; // TODO: this could be anisotropic
};
pbr_Material pbr_CreateMaterial()
{
    pbr_Material m;
    m.albedo = float3(1,1,1);
    m.baseReflectivity = float3(1,1,1);
    m.roughness = 0;
    return m;
}
//=============================================================================
struct pbr_Context
{
    float3 normal;
    float3 viewDirection;
};
pbr_Context pbr_CreateContext()
{
    pbr_Context c;
    c.normal = float3(0,0,1);
    c.viewDirection = float3(0,0,1);
    return c;
}
//=============================================================================
// Metals are characterized by a high relflectance and tinted specular, but
// no diffuse lighing (light is directly absorbed if not reflected).
// Non-metals have no tinted relection, low reflectance, but an important
// diffuse term
float3 pbr_AlbedoOfMetal()               { return float3(0,0,0); }
//=============================================================================
// cf. http://renderwonk.com/publications/s2010-shading-course
// Metals
float3 pbr_BaseReflectivityOfGold()      { return float3(1.00, 0.71, 0.29); }
float3 pbr_BaseReflectivityOfSilver()    { return float3(0.95, 0.93, 0.88); }
float3 pbr_BaseReflectivityOfCopper()    { return float3(0.95, 0.64, 0.54); }
float3 pbr_BaseReflectivityOfIron()      { return float3(0.56, 0.57, 0.58); }
float3 pbr_BaseReflectivityOfAluminium() { return float3(0.91, 0.92, 0.92); }
// Non metals
float3 pbr_BaseReflectivityOfWater()       { return 0.02 * float3(1,1,1); }
float3 pbr_BaseReflectivityOfPlasticLow()  { return 0.03 * float3(1,1,1); }
float3 pbr_BaseReflectivityOfPlasticHigh() { return 0.05 * float3(1,1,1); }
float3 pbr_BaseReflectivityOfGlassLow()    { return 0.03 * float3(1,1,1); }
float3 pbr_BaseReflectivityOfGlassHigh()   { return 0.08 * float3(1,1,1); }
float3 pbr_BaseReflectivityOfRuby()        { return 0.08 * float3(1,1,1); }
float3 pbr_BaseReflectivityOfDiamond()     { return 0.17 * float3(1,1,1); }
//=============================================================================
// cf. https://en.wikipedia.org/wiki/List_of_refractive_indices
float pbr_RefractiveIndexOfWaterIce() { return 1.31; }
float pbr_RefractiveIndexOfWater() { return 1.33; }
float pbr_RefractiveIndexOfGlass() { return 1.5; }
float pbr_RefractiveIndexOfRefractiveGlass() { return 1.8; }
float pbr_RefractiveIndexOfDiamond() { return 2.417; }
//=============================================================================
// Schlick's approximation of Fresnel reflectivity coefficient.
// This term must be used to lerp between base reflectivity and 100%
float pbr_SchlickApproximation(float cosAngle)
{
    return pow(1-cosAngle, 5);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float pbr_ApproximationToFresnel(float cosAngle)
{
    return math_sq(math_sq(1-cosAngle));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float pbr_Fresnel(float cosAngle)
{
    return pbr_SchlickApproximation(cosAngle);
}
//=============================================================================
// In general, n1 will be 1, the index of air.
float pbr_BaseReflectivityFromRefractiveIndices(float n1, float n2)
{
    // https://en.wikipedia.org/wiki/Fresnel_equations
    return math_sq((n1 - n2) / (n1 + n2));
}
//=============================================================================
// Specular BRDF model from Cook and Torrance:
// f(l,v) = F(l,h) G(l,v,h) D(h) / 4(n.l)(n.v)
// with F(l,h) the Fresnel term
//      G(l,v,h) the geometric term (how microfacets are shadowed and masked by others)
//      D(h) the microfacet distribution
// TODO: where does the 4(n.l)(n.v) comes from?
//=============================================================================
// Note that the solid angle of the half space is TAU.
//
// cosAngle is the dot product n.h, where n is the normal of the surface, h is
// the half vector between light and view directions.
// These functions come from
// http://simonstechblog.blogspot.jp/2011/12/microfacet-brdf.html
// and should be checked
float pbr_NormalDensityBlinnPhong(float cosAngle, float roughness)
{
    float a = 2 / math_sq(max(roughness, 0.01)) - 2;
    float nh = cosAngle;
    //float D = (a + 2) / math_TAU * pow(nh, a);
    float D = pow(nh, a);
    return D;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float pbr_NormalDensityBeckmann(float cosAngle, float roughness)
{
    float m = roughness;
    float m2 = math_sq(m);
    float nh = cosAngle;
    float nh2 = math_sq(nh);
    float nh4 = math_sq(nh2);
    float a = exp((nh2 - 1) / (m2 * nh2));
    float b = math_PI * m2 * nh4;
    float D = a / b;
}
//=============================================================================
// Diffuse BRDF models
float3 pbr_DiffuseLambert(float3 inputLight, float refractedLightRatio, float cosThetai, float3 albedo)
{
    return (refractedLightRatio * cosThetai) * albedo * inputLight;
}
float3 pbr_DiffuseOrenNayar()
{
    // TODO
}
//=============================================================================
// General BRDF: f(l,v)
// General equation (without scattering):
//      Lo(v) = integral on w of f(l,v) x Li(v) n.l dw
//
// Lighting can be split into a specular term and a diffuse term. The specular
// term comes from the first reflection at the surface of the material. The
// diffuse term comes from light that has been refracted into the material,
// filtered by various mechanisms and re-emited in random directions.
// When considering an incident ray, part of it will be directly reflected
// according to Fresnel's law, the rest will contribute to diffuse.
//
// TODO: describe roughness
float3 pbr_ApplyDirectionalLight(pbr_Context context, pbr_Material material, pbr_DirectionalLight light)
{
    // TODO:
    // - split input light in specular and diffuse contribution (view independant)
    //      - take care of the normal distribution
    // - compute specular
    // - compute diffuse (view idependant)
    // - sum both terms
    float3 n = context.normal;
    float3 l = light.direction;
    float dotnl = dot(n, l);
    float dotnl_ = max(dotnl, 0);
    // NB: Using the fact that metals have zero albedo and non-metals have non
    // tinted specular, we can use the green channel.
    float inputReflectedRatio = lerp(material.baseReflectivity.g, 1, pbr_Fresnel(dotnl)); // approx, considering roughness = 0
    float inputRefratedRatio = 1 - inputReflectedRatio;
    float3 diffuse = pbr_DiffuseLambert(light.color, inputRefratedRatio, dotnl_, material.albedo);

    float3 v = context.viewDirection;
    float3 preh = l + v;
    float dotnpreh = dot(n, preh);
    if(dotnpreh <= 0)
        preh = -n;
    float3 h = normalize(preh);
    float dotnv = dot(n, v);
    float dotnh = dot(n, h);
    float dotlh = dot(l, h);
    float dotnv_ = max(dotnv, 0);
    float dotnh_ = max(dotnh, 0);
    float dotlh_ = max(dotlh, 0);
    float3 F = lerp(material.baseReflectivity, float3(1,1,1), pbr_Fresnel(dotlh_));
    float G = 1;
    float D = pbr_NormalDensityBlinnPhong(dotnh_, material.roughness);
    //float D = pow(dotnh_, 8);
    //float3 specular = F * (G * D * (1/ max(4 * dotnl_ * dotnv_, 0.01)));
    float3 specular = F * G * D;
    //float3 specular = 10 * material.baseReflectivity * D;

    //return v;
    //return dot(v,n);
    //return 0.3 * n;
    return diffuse + specular;
}
//=============================================================================
float3 pbr_ApplyAmbiantLight(pbr_Context context, pbr_Material material, pbr_AmbiantLight light)
{
    // NB: Using the fact that metals have zero albedo and non-metals have non
    // tinted specular, we can use the green channel.
    float inputReflectedRatio = material.baseReflectivity.g; // approx, should be integral over angles
    float inputRefratedRatio = 1 - inputReflectedRatio;
    float3 diffuse = pbr_DiffuseLambert(light.color, inputRefratedRatio, 1.f, material.albedo);

    float3 n = context.normal;
    float3 v = context.viewDirection;
    float dotnv = dot(n, v);
    float dotnv_ = max(dotnv, 0);
    // TODO: check this
    float3 F = (1 / (4 * math_PI)) * lerp(material.baseReflectivity, float3(1,1,1), pbr_Fresnel(dotnv_));
    float G = 1;
    float D = 1;
    float3 specular = F * G * D;
    return diffuse + specular;
}
//=============================================================================

#endif

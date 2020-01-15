#include "stdafx.h"

#include "CollisionDetection.h"

#include <Math/NumericalUtils.h>

namespace sg {
namespace physics {
//=============================================================================
namespace {
//=============================================================================
#define DETECT_CONTACT_POINT_ARGS \
    ArrayList<ContactPoint>& oContacts, \
    ICollisionPrimitive const& iPrimitiveA, \
    float3x4 const iConfigurationA, \
    ICollisionPrimitive const& iPrimitiveB, \
    float3x4 const iConfigurationB, \
    ContactPointTemplate const& iTemplate

#define FORWARD_DETECT_CONTACT_POINT_ARGS oContacts, iPrimitiveA, iConfigurationA, iPrimitiveB, iConfigurationB, iTemplate
#define FORWARD_DETECT_CONTACT_POINT_ARGS_REVERSE oContacts, iPrimitiveB, iConfigurationB, iPrimitiveA, iConfigurationA, iTemplate
#define SG_UNUSED_DETECT_CONTACT_POINT_ARGS SG_UNUSED((oContacts, iPrimitiveA, iConfigurationA, iPrimitiveB, iConfigurationB, iTemplate))
typedef bool (*DetectContactPointsReturnContact_Fct)(DETECT_CONTACT_POINT_ARGS);
//=============================================================================
bool DetectContactPointsReturnContact_SphereSphere(DETECT_CONTACT_POINT_ARGS)
{
    SG_UNUSED_DETECT_CONTACT_POINT_ARGS;
    CollisionSphere const& sphereA = checked_cast<CollisionSphere const&>(iPrimitiveA);
    float const rA = sphereA.Radius();
    float3 const CA = iConfigurationA * sphereA.Center().xyz1();
    CollisionSphere const& sphereB = checked_cast<CollisionSphere const&>(iPrimitiveB);
    float const rB = sphereB.Radius();
    float3 const CB = iConfigurationB * sphereB.Center().xyz1();

    float sumR = rA + rB;
    float3 const AB = CB - CA;
    float const lsq = AB.LengthSq();
    if(lsq <= sq(sumR))
    {
        ContactPoint& contact = oContacts.EmplaceBack();
        ApplyTemplate<false>(contact, iTemplate);
        bool areCentersMerged = lsq <= 1.e-10f * sq(sumR);
        float const ool = areCentersMerged ? 0.f : 1.f/sqrt(lsq); // TODO: rsqrt
        float const interpenetration = areCentersMerged ? -std::max(rA, rB) : 1.f / ool - sumR;
        float3 const normal = areCentersMerged ? float3(0,0,1) : AB.Normalised();
        contact.interpenetration = interpenetration;
        contact.normal = normal;
        contact.position = CA + (rA + (0.5f * interpenetration)) * normal;
        return true;
    }
    return false;
}
//=============================================================================
template <bool reverseTemplate>
bool DetectContactPointsReturnContact_SphereHalfSpace_Impl(DETECT_CONTACT_POINT_ARGS)
{
    SG_UNUSED_DETECT_CONTACT_POINT_ARGS;
    CollisionSphere const& sphereA = checked_cast<CollisionSphere const&>(iPrimitiveA);
    float const rA = sphereA.Radius();
    float3 const CA = iConfigurationA * sphereA.Center().xyz1();
    CollisionHalfSpace const& halfSpaceB = checked_cast<CollisionHalfSpace const&>(iPrimitiveB);
    float4 const eqB = halfSpaceB.Equation();
    float3 const normalB = iConfigurationB * eqB.xyz0();
    float const constantB = eqB.w() - dot(iConfigurationB.Col(3), normalB);

    float const d = dot(CA, normalB) - rA + constantB;

    if(d <= 0.f)
    {
        ContactPoint& contact = oContacts.EmplaceBack();
        ApplyTemplate<reverseTemplate>(contact, iTemplate);
        contact.interpenetration = d;
        contact.normal = -normalB;
        contact.position = CA - (rA + (0.5f * d)) * normalB;
        return true;
    }
    return false;
}
bool DetectContactPointsReturnContact_SphereHalfSpace(DETECT_CONTACT_POINT_ARGS)
{
    return DetectContactPointsReturnContact_SphereHalfSpace_Impl<false>(FORWARD_DETECT_CONTACT_POINT_ARGS);
}
bool DetectContactPointsReturnContact_HalfSpaceSphere(DETECT_CONTACT_POINT_ARGS)
{
    return DetectContactPointsReturnContact_SphereHalfSpace_Impl<true>(FORWARD_DETECT_CONTACT_POINT_ARGS_REVERSE);
}
//=============================================================================
template <bool reverseTemplate>
bool DetectContactPointsReturnContact_SphereHeightField_Impl(DETECT_CONTACT_POINT_ARGS)
{
    SG_UNUSED_DETECT_CONTACT_POINT_ARGS;
    CollisionSphere const& sphereA = checked_cast<CollisionSphere const&>(iPrimitiveA);
    float const rA = sphereA.Radius();
    float3 const CA = iConfigurationA * sphereA.Center().xyz1();
    ICollisionHeightField const& heightFieldB = checked_cast<ICollisionHeightField const&>(iPrimitiveB);
    box3f const& boxB = heightFieldB.Box();
    float3 const exB = iConfigurationB.Col(0);
    float3 const eyB = iConfigurationB.Col(1);
    float3 const ezB = iConfigurationB.Col(2);
    float3 const CB = iConfigurationB.Col(3);
    float3 const BA = CA - CB;
    float const u = tweening::linearStep(boxB.Min().x(), boxB.Max().x(), dot(BA, exB));
    float const v = tweening::linearStep(boxB.Min().y(), boxB.Max().y(), dot(BA, eyB));
    float const hA = dot(BA, ezB);
    float2 const uv = float2(u,v);
    uint2 const sampleCount = heightFieldB.SampleCount();
    float2 const texPos = uv * sampleCount;
    int2 const texPosi = int2(round(texPos));
    if(AllLessStrict(uint2(texPosi - 1), sampleCount-2))
    {
        box2u const sampleBox = box2u::FromCenterDelta(uint2(texPosi), uint2(3));
        float tabh[9];
        heightFieldB.GetHeights(AsArrayView(tabh), AsArrayView(&sampleBox, 1));
        float2 const a = texPos-texPosi;
        uint2 const b = uint2(floor(a)) + 1;
        float2 const f = a - b + 1;
        SG_ASSERT(AllLessEqual(b, uint2(1)));
        float const t = lerp(
            lerp(tabh[b.x()+0 +3*(b.y()+0)], tabh[b.x()+1 +3*(b.y()+0)], f.x()),
            lerp(tabh[b.x()+0 +3*(b.y()+1)], tabh[b.x()+1 +3*(b.y()+1)], f.x()),
            f.y());
        float const h = lerp(boxB.Min().z(), boxB.Max().z(), t);
        float const d = hA - h - rA;
        if(d <= 0)
        {
            float const dtdu00 = tabh[      1 +3*(b.y()+0)] - tabh[      0 +3*(b.y()+0)];
            float const dtdu10 = tabh[      2 +3*(b.y()+0)] - tabh[      1 +3*(b.y()+0)];
            float const dtdu01 = tabh[      1 +3*(b.y()+1)] - tabh[      0 +3*(b.y()+1)];
            float const dtdu11 = tabh[      2 +3*(b.y()+1)] - tabh[      1 +3*(b.y()+1)];
            float const dtdv00 = tabh[b.x()+0 +3*(      1)] - tabh[b.x()+0 +3*(      0)];
            float const dtdv10 = tabh[b.x()+1 +3*(      1)] - tabh[b.x()+1 +3*(      0)];
            float const dtdv01 = tabh[b.x()+0 +3*(      2)] - tabh[b.x()+0 +3*(      1)];
            float const dtdv11 = tabh[b.x()+1 +3*(      2)] - tabh[b.x()+1 +3*(      1)];
            float const dtdu = lerp(lerp(dtdu00, dtdu10, f.x()), lerp(dtdu11, dtdu01, f.x()), f.y());
            float const dtdv = lerp(lerp(dtdv00, dtdv10, f.x()), lerp(dtdv11, dtdv01, f.x()), f.y());

            float const hScale = boxB.Max().z() - boxB.Min().z();
            float2 const duvdxy = sampleCount * (1.f/ boxB.Delta().xy());
            float const dhdx = dtdu * hScale * duvdxy.x();
            float const dhdy = dtdv * hScale * duvdxy.y();
            float3 const tx = exB + dhdx * ezB;
            float3 const ty = eyB + dhdy * ezB;
            //float3 const normalB = ezB;
            float3 const normalB = cross(tx, ty).Normalised();
            SG_ASSERT(0.f < dot(normalB, ezB));
            ContactPoint& contact = oContacts.EmplaceBack();
            ApplyTemplate<reverseTemplate>(contact, iTemplate);
            contact.interpenetration = d;
            contact.normal = -normalB;
            contact.position = CA - (rA + (0.5f * d)) * ezB;
            return true;

        }
        SG_BREAKABLE_POS;
    }
    else
    {
    }
    //float4 const eqB = halfSpaceB.Equation();
    //float const constantB = eqB.w() - dot(iConfigurationB.Col(3), normalB);
    //
    //float const d = dot(CA, normalB) - rA + constantB;
    //
    //if(d <= 0.f)
    //{
    //}
    return false;
}
bool DetectContactPointsReturnContact_SphereHeightField(DETECT_CONTACT_POINT_ARGS)
{
    return DetectContactPointsReturnContact_SphereHeightField_Impl<false>(FORWARD_DETECT_CONTACT_POINT_ARGS);
}
bool DetectContactPointsReturnContact_HeightFieldSphere(DETECT_CONTACT_POINT_ARGS)
{
    return DetectContactPointsReturnContact_SphereHeightField_Impl<true>(FORWARD_DETECT_CONTACT_POINT_ARGS_REVERSE);
}
//=============================================================================
bool DetectContactPointsReturnContact_Unimplemented(DETECT_CONTACT_POINT_ARGS)
{
    SG_UNUSED_DETECT_CONTACT_POINT_ARGS;
    return false;
}
//=============================================================================
struct CollisionPrimitivePairTraits
{
#if SG_ENABLE_ASSERT
    CollisionPrimitiveType typeA;
    CollisionPrimitiveType typeB;
#endif
    DetectContactPointsReturnContact_Fct detectContactPoints;
};
//=============================================================================
CollisionPrimitivePairTraits collisionPrimitivePairTraits[] =
{
#define PAIR(TYPE_A, TYPE_B) SG_CODE_FOR_ASSERT(CollisionPrimitiveType::TYPE_A SG_COMMA CollisionPrimitiveType::TYPE_B SG_COMMA)
    PAIR(     Sphere,      Sphere) DetectContactPointsReturnContact_SphereSphere,
    PAIR(     Sphere,         Box) DetectContactPointsReturnContact_Unimplemented,
    PAIR(     Sphere,   HalfSpace) DetectContactPointsReturnContact_SphereHalfSpace,
    PAIR(     Sphere, HeightField) DetectContactPointsReturnContact_SphereHeightField,

    PAIR(        Box,      Sphere) DetectContactPointsReturnContact_Unimplemented,
    PAIR(        Box,         Box) DetectContactPointsReturnContact_Unimplemented,
    PAIR(        Box,   HalfSpace) DetectContactPointsReturnContact_Unimplemented,
    PAIR(        Box, HeightField) DetectContactPointsReturnContact_Unimplemented,

    PAIR(  HalfSpace,      Sphere) DetectContactPointsReturnContact_HalfSpaceSphere,
    PAIR(  HalfSpace,         Box) DetectContactPointsReturnContact_Unimplemented,
    PAIR(  HalfSpace,   HalfSpace) DetectContactPointsReturnContact_Unimplemented,
    PAIR(  HalfSpace, HeightField) DetectContactPointsReturnContact_Unimplemented,

    PAIR(HeightField,      Sphere) DetectContactPointsReturnContact_HeightFieldSphere,
    PAIR(HeightField,         Box) DetectContactPointsReturnContact_Unimplemented,
    PAIR(HeightField,   HalfSpace) DetectContactPointsReturnContact_Unimplemented,
    PAIR(HeightField, HeightField) DetectContactPointsReturnContact_Unimplemented,
#undef PAIR
};
//=============================================================================
}
//=============================================================================
bool DetectContactPointsReturnContact(
    ArrayList<ContactPoint>& oContacts,
    size_t iIndexA,
    RigidBodyCollisionShape const& iShapeA,
    float3x4 const& iConfigurationA,
    size_t iIndexB,
    RigidBodyCollisionShape const& iShapeB,
    float3x4 const& iConfigurationB)
{
    static_assert(SG_ARRAYSIZE(collisionPrimitivePairTraits) == sq(size_t(CollisionPrimitiveType::Count)), "");
    ArrayView<refptr<ICollisionPrimitive> const> primitivesA = iShapeA.Primitives();
    ArrayView<refptr<ICollisionPrimitive> const> primitivesB = iShapeB.Primitives();
    ContactPointTemplate contactPointTemplate;
    contactPointTemplate.indexA = iIndexA;
    contactPointTemplate.indexB = iIndexB;
    bool areInContact = false;
    for_range(size_t, i, 0, primitivesA.Size())
    {
        ICollisionPrimitive const& primitivei = *primitivesA[i];
        CollisionPrimitiveType const typei = primitivei.Type();
        contactPointTemplate.materialIdA = primitivei.GetMaterialId();
        for_range(size_t, j, 0, primitivesB.Size())
        {
            ICollisionPrimitive const& primitivej = *primitivesB[j];
            CollisionPrimitiveType const typej = primitivej.Type();
            contactPointTemplate.materialIdB = primitivej.GetMaterialId();
            {
                size_t const ti = size_t(typei);
                size_t const tj = size_t(typej);
                size_t const n = size_t(CollisionPrimitiveType::Count);
                size_t const index = n * ti + tj;
                SG_ASSERT(index < SG_ARRAYSIZE(collisionPrimitivePairTraits));
                SG_ASSERT(typei == collisionPrimitivePairTraits[index].typeA);
                SG_ASSERT(typej == collisionPrimitivePairTraits[index].typeB);
                bool const arePrimitivesInContact = collisionPrimitivePairTraits[index].detectContactPoints(oContacts, primitivei, iConfigurationA, primitivej, iConfigurationB, contactPointTemplate);
                if(arePrimitivesInContact)
                    areInContact = true;
            }
        }
    }
    return areInContact;
}
//=============================================================================
}
}
#undef DETECT_CONTACT_POINT_ARGS
#undef FORWARD_DETECT_CONTACT_POINT_ARGS
#undef SG_UNUSED_DETECT_CONTACT_POINT_ARGS

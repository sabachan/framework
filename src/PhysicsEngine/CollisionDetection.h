#ifndef PhysicsEngine_CollisionDetection_H
#define PhysicsEngine_CollisionDetection_H

#include "Common.h"
#include <Core/ArrayList.h>
#include <Core/SmartPtr.h>
#include <Math/Box.h>
#include <Math/Matrix.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

namespace sg {
namespace physics {
//=============================================================================
struct MaterialId
{
    size_t index;
};
//=============================================================================
struct ContactPoint
{
    size_t indexA;
    size_t indexB;
    float3 position;
    float3 normal;
    float interpenetration;
    MaterialId materialIdA;
    MaterialId materialIdB;
};
//=============================================================================
struct ContactPointTemplate
{
    size_t indexA;
    size_t indexB;
    MaterialId materialIdA;
    MaterialId materialIdB;
};
template <bool reverse>
void ApplyTemplate(ContactPoint& oContactPoint, ContactPointTemplate const& iTemplate)
{
    if(reverse)
    {
        oContactPoint.indexA = iTemplate.indexB;
        oContactPoint.indexB = iTemplate.indexA;
        oContactPoint.materialIdA = iTemplate.materialIdB;
        oContactPoint.materialIdB = iTemplate.materialIdA;
    }
    else
    {
        oContactPoint.indexA = iTemplate.indexA;
        oContactPoint.indexB = iTemplate.indexB;
        oContactPoint.materialIdA = iTemplate.materialIdA;
        oContactPoint.materialIdB = iTemplate.materialIdB;
    }
}
//=============================================================================
enum class CollisionPrimitiveType
{
    Sphere,
    Box,
    HalfSpace,
    HeightField,

    Count,
};
//=============================================================================
class ICollisionPrimitive : public RefAndSafeCountable
{
    SG_NON_COPYABLE(ICollisionPrimitive)
public:
    virtual ~ICollisionPrimitive() {}
    CollisionPrimitiveType Type() const { return m_type; }
    MaterialId GetMaterialId() const { return m_materialId; }
protected:
    ICollisionPrimitive(CollisionPrimitiveType iType, MaterialId iMaterialId) : m_type(iType), m_materialId(iMaterialId) {}
private:
    CollisionPrimitiveType const m_type;
    MaterialId m_materialId;
};
//=============================================================================
class CollisionSphere : public ICollisionPrimitive
{
public:
    CollisionSphere(float3 const& iCenter, float iRadius, MaterialId iMaterialId)
        : ICollisionPrimitive(CollisionPrimitiveType::Sphere, iMaterialId)
        , m_center(iCenter)
        , m_radius(iRadius)
    {}
    float3 const& Center() const { return m_center; }
    float Radius() const { return m_radius; }
private:
    float3 m_center;
    float m_radius;
};
//=============================================================================
class CollisionBox : public ICollisionPrimitive
{
public:
    CollisionBox(float3 const& iCenter, float3x3 const& iHalfAxes, MaterialId iMaterialId)
        : ICollisionPrimitive(CollisionPrimitiveType::Box, iMaterialId)
        , m_center(iCenter)
        , m_halfAxes(iHalfAxes)
    {
        SG_ASSERT(dot(iHalfAxes.Row(0), iHalfAxes.Row(1)) < 1.e-6f * (dot(iHalfAxes.Row(0), iHalfAxes.Row(0)) + dot(iHalfAxes.Row(1), iHalfAxes.Row(1))));
        SG_ASSERT(dot(iHalfAxes.Row(0), iHalfAxes.Row(2)) < 1.e-6f * (dot(iHalfAxes.Row(0), iHalfAxes.Row(0)) + dot(iHalfAxes.Row(2), iHalfAxes.Row(2))));
        SG_ASSERT(dot(iHalfAxes.Row(1), iHalfAxes.Row(2)) < 1.e-6f * (dot(iHalfAxes.Row(1), iHalfAxes.Row(1)) + dot(iHalfAxes.Row(2), iHalfAxes.Row(2))));
    }
    float3 const& Center() const { return m_center; }
    float3x3 const& HalfAxes() const { return m_halfAxes; }
private:
    float3 m_center;
    float3x3 m_halfAxes;
};
//=============================================================================
class CollisionHalfSpace : public ICollisionPrimitive
{
public:
    CollisionHalfSpace(float4 const& iEquation, MaterialId iMaterialId)
        : ICollisionPrimitive(CollisionPrimitiveType::HalfSpace, iMaterialId)
        , m_equation(iEquation)
    {
    }
    float4 const& Equation() const { return m_equation; }
private:
    float4 m_equation;
};
//=============================================================================
class ICollisionHeightField : public ICollisionPrimitive
{
public:
    enum BorderMode { Clamp, Empty };
    ICollisionHeightField(uint2 const& iSampleCounts, box3f const& iBox, MaterialId iMaterialId)
        : ICollisionPrimitive(CollisionPrimitiveType::HeightField, iMaterialId)
        , m_sampleCounts(iSampleCounts)
        , m_box(iBox)
    {
        SG_ASSERT(AllLessStrict(uint2(0,0), m_sampleCounts));
    }
    void GetHeights(ArrayView<float> oHeights, ArrayView<box2u const> iPositions) const
    {
#if SG_ENABLE_ASSERT
        size_t heightCount = 0;
        for(box2u const& p : iPositions)
        {
            SG_ASSERT(AllLessEqual(p.Max(), m_sampleCounts));
            SG_ASSERT(p.IsConvex());
            heightCount += p.NVolume_AssumeConvex();
        }
        SG_ASSERT(oHeights.size() == heightCount);
#endif
        VirtualGetHeights(oHeights, iPositions);
#if SG_ENABLE_ASSERT
        for(float h : oHeights)
        {
            SG_ASSERT(0.f <= h);
            SG_ASSERT(h <= 1.f);
        }
#endif
    }
    box3f const& Box() const { return m_box; }
    uint2 const SampleCount() const { return m_sampleCounts; }
protected:
    virtual void VirtualGetHeights(ArrayView<float> oHeights, ArrayView<box2u const> iPositions) const = 0;
private:
    uint2 m_sampleCounts;
    box3f m_box;
};
//=============================================================================
class RigidBodyCollisionShape : public RefAndSafeCountable
{
public:
    RigidBodyCollisionShape(ArrayView<refptr<ICollisionPrimitive>> iPrimitives)
    {
        m_primitives.Reserve(iPrimitives.Size());
        for(auto const& it : iPrimitives)
            m_primitives.EmplaceBack_AssumeCapacity(it.get());
    }
    ArrayView<refptr<ICollisionPrimitive> const> Primitives() const { return m_primitives.View(); }
private:
    ArrayList<refptr<ICollisionPrimitive>> m_primitives;
};
//=============================================================================
bool DetectContactPointsReturnContact(
    ArrayList<ContactPoint>& oContacts,
    size_t iIndexA,
    RigidBodyCollisionShape const& iShapeA,
    float3x4 const& iConfigurationA,
    size_t iIndexB,
    RigidBodyCollisionShape const& iShapeB,
    float3x4 const& iConfigurationB);
//=============================================================================
}
}

#endif


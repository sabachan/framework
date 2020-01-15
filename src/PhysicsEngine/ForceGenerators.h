#ifndef PhysicsEngine_ForceGenerators_H
#define PhysicsEngine_ForceGenerators_H

#include "Common.h"
#include <Core/ArrayList.h>
#include <Core/SmartPtr.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

namespace sg {
namespace physics {
//=============================================================================
struct ContactPoint;
struct RigidBodyProperties;
//=============================================================================
class IForceGenerator : public RefAndSafeCountable
{
public:
    virtual void Apply(ArrayView<RigidBodyProperties> ioRigidBodies, float dt) const = 0;
};
//=============================================================================
class GravityGenerator : public IForceGenerator
{
public:
    GravityGenerator(float3 const& gravity);
    virtual void Apply(ArrayView<RigidBodyProperties> ioRigidBodies, float dt) const;
private:
    float3 m_gravity;
};
//=============================================================================
}
}

#endif


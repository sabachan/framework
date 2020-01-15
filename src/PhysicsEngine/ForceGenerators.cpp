#include "stdafx.h"

#include "ForceGenerators.h"

#include "RigidBodySimulator.h"

namespace sg {
namespace physics {
//=============================================================================
GravityGenerator::GravityGenerator(float3 const& gravity)
    : m_gravity(gravity)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GravityGenerator::Apply(ArrayView<RigidBodyProperties> ioRigidBodies, float dt) const
{
    float3 const g = m_gravity;
    for(auto& it : ioRigidBodies)
    {
        if(it.isFixed)
            continue;
        it.linearVelocity += g * dt;
    }
}
//=============================================================================
}
}

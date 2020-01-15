#ifndef PhysicsEngine_Links_H
#define PhysicsEngine_Links_H

#include "Common.h"
#include <Core/ArrayList.h>
#include <Core/SmartPtr.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

namespace sg {
namespace physics {
//=============================================================================
enum class LinkType
{
    BallJoint,

};
//=============================================================================
struct Link
{
    LinkType type;
    RigidBodyId idA;
    RigidBodyId idB;
    float3 posInA;
    float3 posInB;
    float3x3 frameInA;
    float3x3 frameInB;
    // TODO: Limits, Spring Dampers and Motors
};
//=============================================================================
}
}

#endif


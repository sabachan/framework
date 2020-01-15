#ifndef PhysicsEngine_Common_H
#define PhysicsEngine_Common_H

#include <Core/ArrayList.h>
#include <Core/SmartPtr.h>
#include <Math/Matrix.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

namespace sg {
namespace physics {
//=============================================================================
typedef double phloat;
typedef math::Vector<phloat, 3> phloat3;
typedef math::Vector<phloat, 4> phloat4;
typedef math::Vector<phloat, 6> phloat6;
typedef math::Matrix<phloat, 3, 3> phloat3x3;
typedef math::TemplateQuaternion<phloat> phyquaternion;
//=============================================================================
//class RigidBodySimulation;
//=============================================================================
struct RigidBodyId
{
    size_t id;
};
//=============================================================================
}
}

#endif


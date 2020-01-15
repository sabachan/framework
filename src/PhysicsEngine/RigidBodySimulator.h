#ifndef PhysicsEngine_RigidBodySimulator_H
#define PhysicsEngine_RigidBodySimulator_H

#include "Common.h"
#include "CollisionDetection.h"
#include <Core/ArrayList.h>
#include <Core/SmartPtr.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

namespace sg {
namespace physics {
//=============================================================================
class ICollisionShape;
class IForceGenerator;
//=============================================================================
struct RigidBodyProperties
{
    float3 position;
    quaternion orientation; // transforms a point in local to a point in global
    float3 linearVelocity;
    float3 angularVelocity;
    float mass;
    float3 inertiaDiagonal;
    bool isFixed;
    refptr<RigidBodyCollisionShape> collisionShape;

    // The following variables are updated automaticaly
    float3x4 configuration;
    phloat3x3 ooGlobalInertiaMatrix;
    phloat ooMass;
    phloat3 ooInertiaDiagonal;
};
//=============================================================================
struct ConstraintLine
{
    size_t indices[2];
    phloat6 Js[2];
    phloat6 MJs[2];
    phloat lambdaLo;
    phloat lambdaHi;
    phloat targetVelocity;
    phloat targetPosition;
};
//=============================================================================
struct SparseMatrixElement
{
    size_t i;
    size_t j;
    phloat value;
};
//=============================================================================
class RigidBodySimulation : public RefAndSafeCountable
{
    friend class RigidBodySimulator;
public:
    RigidBodySimulation();
    ~RigidBodySimulation();

    // TODO: void SetTimeStep(float iDt) { m_dt = iDt; }

    RigidBodyId AddRigidBody(RigidBodyProperties const& iProperties);
    void GetRigidBodyProperties(RigidBodyProperties& oProperties, RigidBodyId iId);
    void RemoveRigidBody(RigidBodyId iId);

    void AddForceGenerator(IForceGenerator* iGenerator);
private:
    static void UpdateConfiguration(RigidBodyProperties& iRigidBody);
    static void UpdateInverseMass(RigidBodyProperties& iRigidBody);
private:
    ArrayList<RigidBodyProperties> m_rigidBodies;
    // ArrayList<size_t> m_rigidBodyMap; // for being able to compact and sort rigid bodies
    // TODO: size_t m_rigidBodyFreeIndex;

    ArrayList<refptr<IForceGenerator>> m_forceGenerators;
};
//=============================================================================
class RigidBodySimulator
{
public:
    void Update(RigidBodySimulation& ioSimulation, float dt);

private:
    void CollisionDetection(RigidBodySimulation& ioSimulation);
    void GenerateContactConstraints(RigidBodySimulation& ioSimulation);
    void GenerateConstraintMatrix(RigidBodySimulation& ioSimulation);
    void SolveConstraints(RigidBodySimulation& ioSimulation);
    void ApplyLambdas(RigidBodySimulation& ioSimulation);
private:
    ArrayList<ContactPoint> m_contacts;
    ArrayList<ConstraintLine> m_constraints;

    //ArrayList<SparseMatrixElement> m_matrix;
    // For Jacobi/GS/SOR
    ArrayList<SparseMatrixElement> m_LU;
    ArrayList<phloat> m_invD;
    ArrayList<phloat> m_lambdas;
};
//=============================================================================
}
}

#endif


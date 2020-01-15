#include "stdafx.h"

#include "RigidBodySimulator.h"

#include "CollisionDetection.h"
#include "ForceGenerators.h"

namespace sg {
namespace physics {
//=============================================================================
RigidBodySimulation::RigidBodySimulation()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RigidBodySimulation::~RigidBodySimulation()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RigidBodyId RigidBodySimulation::AddRigidBody(RigidBodyProperties const& iProperties)
{
    RigidBodyId id;
    id.id = m_rigidBodies.Size();
    m_rigidBodies.emplace_back(iProperties);
    UpdateInverseMass(m_rigidBodies.Back());
    UpdateConfiguration(m_rigidBodies.Back());
    return id;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulation::GetRigidBodyProperties(RigidBodyProperties& oProperties, RigidBodyId iId)
{
    oProperties = m_rigidBodies[iId.id];
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulation::RemoveRigidBody(RigidBodyId iId)
{
    SG_UNUSED(iId);
    SG_ASSERT_NOT_IMPLEMENTED();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulation::UpdateConfiguration(RigidBodyProperties& iRigidBody)
{
    float3x3 const Rfloat = matrix::Rotation(iRigidBody.orientation);
    phloat3x3 const R = phloat3x3(Rfloat);
    iRigidBody.configuration.SetSubMatrix(0,0, Rfloat);
    iRigidBody.configuration.SetCol(3, iRigidBody.position);

    // TODO: Check
    phloat3x3 const Rm1 = R.Transposed();
    iRigidBody.ooGlobalInertiaMatrix = R * matrix::Diagonal(iRigidBody.ooInertiaDiagonal) * Rm1;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulation::UpdateInverseMass(RigidBodyProperties& iRigidBody)
{
    if(iRigidBody.isFixed)
    {
        iRigidBody.ooMass = 0.f;
        iRigidBody.ooInertiaDiagonal = phloat3(0.f);
    }
    else
    {
        SG_ASSERT(0.f < iRigidBody.mass);
        SG_ASSERT(0.f < iRigidBody.inertiaDiagonal._[0]);
        SG_ASSERT(0.f < iRigidBody.inertiaDiagonal._[1]);
        SG_ASSERT(0.f < iRigidBody.inertiaDiagonal._[2]);
        iRigidBody.ooMass = phloat(1) / iRigidBody.mass;
        iRigidBody.ooInertiaDiagonal = phloat(1) / iRigidBody.inertiaDiagonal;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulation::AddForceGenerator(IForceGenerator* iGenerator)
{
    m_forceGenerators.EmplaceBack(iGenerator);
}
//=============================================================================
void RigidBodySimulator::Update(RigidBodySimulation& ioSimulation, float dt)
{
    // t0         t1         t2
    //  +----------+----------+
    // a0         a1         a2
    //     v1/2       v3/2
    // p0         p1         p2

    // Integrate positions
    // Project positions (this changes velocities and the computation of the forces)
    // Integrate velocities
    // Project positions (doing this here would be wrong, but thats ok)
    // Project velocities


    for_range(size_t, i, 0, ioSimulation.m_rigidBodies.Size())
    {
        RigidBodyProperties& rb = ioSimulation.m_rigidBodies[i];
        if(rb.isFixed)
            continue;
        rb.position += rb.linearVelocity * dt;
        float const angularSpeedSq = rb.angularVelocity.LengthSq();
        if(angularSpeedSq > 1.e-6f)
        {
            float const angularSpeed = sqrt(angularSpeedSq);
            float3 const axis = rb.angularVelocity * (1.f / angularSpeed);
            rb.orientation = quaternion::RepresentRotation_AssumeNormalisedAxis(axis, angularSpeed * dt) * rb.orientation;
            rb.orientation = rb.orientation.Normalised();
        }
        
//#if SG_ENABLE_ASSERT
//        float const normSq = rb.orientation.NormSq();
//        SG_ASSERT(abs(1.f - normSq) < std::numeric_limits<float>::epsilon() * 10.f);
//#endif
    }

    for_range(size_t, i, 0, ioSimulation.m_rigidBodies.Size())
    {
        RigidBodyProperties& rb = ioSimulation.m_rigidBodies[i];
        RigidBodySimulation::UpdateConfiguration(rb);
    }

    for_range(size_t, i, 0, ioSimulation.m_forceGenerators.Size())
    {
        ioSimulation.m_forceGenerators[i]->Apply(ioSimulation.m_rigidBodies.View(), dt);
    }

    SG_ASSERT(m_contacts.Empty());
    SG_ASSERT(m_constraints.Empty());
    CollisionDetection(ioSimulation);
    GenerateContactConstraints(ioSimulation);
    GenerateConstraintMatrix(ioSimulation);
    SolveConstraints(ioSimulation);
    ApplyLambdas(ioSimulation);

    m_contacts.Clear();
    m_constraints.Clear();

    SG_BREAKABLE_POS;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulator::CollisionDetection(RigidBodySimulation& ioSimulation)
{
    size_t const n = ioSimulation.m_rigidBodies.Size();
    for_range(size_t, i, 0, n)
    {
        RigidBodyProperties const& rbi = ioSimulation.m_rigidBodies[i];
        RigidBodyCollisionShape const* shapei = rbi.collisionShape.get();
        if(nullptr == shapei)
            continue;
        for_range(size_t, j, i+1, n)
        {
            RigidBodyProperties const& rbj = ioSimulation.m_rigidBodies[j];
            RigidBodyCollisionShape const* shapej = rbj.collisionShape.get();
            if(nullptr == shapej)
                continue;
            bool const contact = DetectContactPointsReturnContact(m_contacts, i, *shapei, rbi.configuration, j, *shapej, rbj.configuration);

#if 0
            if(contact)
            {
                ioSimulation.m_rigidBodies[i].linearVelocity = float3(0);
                ioSimulation.m_rigidBodies[i].angularVelocity = float3(0);
                ioSimulation.m_rigidBodies[i].mass = 0;
                ioSimulation.m_rigidBodies[i].inertiaDiagonal = float3(0);
                ioSimulation.m_rigidBodies[i].isFixed = true;
                ioSimulation.m_rigidBodies[j].linearVelocity = float3(0);
                ioSimulation.m_rigidBodies[j].angularVelocity = float3(0);
                ioSimulation.m_rigidBodies[j].mass = 0;
                ioSimulation.m_rigidBodies[j].inertiaDiagonal = float3(0);
                ioSimulation.m_rigidBodies[j].isFixed = true;
                RigidBodySimulation::UpdateInverseMass(ioSimulation.m_rigidBodies[i]);
                RigidBodySimulation::UpdateInverseMass(ioSimulation.m_rigidBodies[j]);
                RigidBodySimulation::UpdateConfiguration(ioSimulation.m_rigidBodies[i]);
                RigidBodySimulation::UpdateConfiguration(ioSimulation.m_rigidBodies[j]);
            }
#endif
        }
    }
    SG_BREAKABLE_POS;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t D, typename T = float>
class Dual
{
    static_assert(std::is_floating_point<T>::value, "");
public:
    static size_t const variable_count = D;
    typedef T value_type;
    typedef math::Vector<T, D> derivatives_type;
    static Dual Constant(value_type iConstant) { Dual c = Dual(iConstant); return c; }
    template <size_t I> static Dual Variable(T iValue) { static_assert(I < variable_count, ""); Dual x; x.m_value = iValue; x.m_derivatives[I] = T(1); }
    static Dual Variable(value_type iValue, size_t iVariableIndex) { SG_ASSERT(iVariableIndex < variable_count); Dual x; x.m_value = iValue; x.m_derivatives[iVariableIndex] = T(1); return x; }

    Dual() : m_value(0), m_derivatives(0) {}
    explicit Dual(value_type iConstant) : m_value(iConstant), m_derivatives(0) {}
    Dual(Dual const&) = default;
    template <typename U> explicit Dual(Dual<D, U> const& b) : m_value(T(b.m_value)), m_derivatives(b.m_derivatives) {}

    Dual& operator=(Dual const&) = default;

    value_type Value() const { return m_value; }
    derivatives_type Derivatives() const { return m_derivatives; }

    Dual& operator+= (Dual const& b)
    {
        m_value += b.m_value;
        m_derivatives += b.m_derivatives;
        return *this;
    }
    Dual& operator-= (Dual const& b)
    {
        m_value -= b.m_value;
        m_derivatives -= b.m_derivatives;
        return *this;
    }
    friend Dual operator+ (Dual a, Dual const& b)
    {
        a += b;
        return a;
    }
    friend Dual operator- (Dual a, Dual const& b)
    {
        a -= b;
        return a;
    }
    Dual& operator*= (Dual const& b)
    {
        m_value *= b.m_value;
        m_derivatives = m_derivatives * b.m_value + m_value * b.m_derivatives;
        return *this;
    }
    friend Dual operator* (Dual a, Dual const& b)
    {
        a *= b;
        return a;
    }
    Dual operator- ()
    {
        Dual r;
        r.m_value = -m_value;
        r.m_derivatives = -m_derivatives;
        retrun r;
    }
private:
    value_type m_value;
    derivatives_type m_derivatives;
};
namespace dual
{
    template <size_t D, typename T, size_t N>
    math::Vector<Dual<D, T>, N> MakeConstant(math::Vector<T, N> const& iVector)
    {
        math::Vector<Dual<D, T>, N> r;
        for_range(size_t, i, 0, N) { r._[i] = Dual<D, T>::Constant(iVector._[i]); }
        return r;
    }
    template <size_t D, size_t I, typename T, size_t N>
    math::Vector<Dual<D, T>, N> MakeVectorVariables(math::Vector<T, N> const& iVector)
    {
        static_assert(I + N <= D, "too many variables (or not enough)");
        math::Vector<Dual<D, T>, N> r;
        for_range(size_t, i, 0, N) { r._[i] = Dual<D, T>::Variable(iVector._[i], I + i); }
        return r;
    }
}
typedef Dual<6, phloat> phydual6;
typedef math::Vector<phydual6, 3> phydual6vec3;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulator::GenerateContactConstraints(RigidBodySimulation& ioSimulation)
{
    phloat const infinity = std::numeric_limits<phloat>::infinity();
    size_t const contactCount = m_contacts.Size();
    size_t const avgConstraintCountPerContact = 1;
    m_constraints.Reserve(m_constraints.Size() + avgConstraintCountPerContact * contactCount);
    for_range(size_t, i, 0, contactCount)
    {
        ContactPoint const& contact = m_contacts[i];
        m_constraints.Reserve(m_constraints.Size() + 3);
        ConstraintLine& line0 = m_constraints.EmplaceBack();
        //ConstraintLine& line1 = m_constraints.EmplaceBack();
        //ConstraintLine& line2 = m_constraints.EmplaceBack();
        line0.lambdaLo = 0;
        line0.lambdaHi = infinity;
        line0.targetPosition = -contact.interpenetration;
        line0.targetVelocity = 0.f;
        size_t const indices[] = { contact.indexA, contact.indexB };
        float const signs[] = { -1.f, 1.f };
        for_range(size_t, j, 0, 2)
        {
            float const sign = signs[j];
            RigidBodyProperties const& rb = ioSimulation.m_rigidBodies[indices[j]];
            phydual6vec3 const linerarVelocity = dual::MakeVectorVariables<6,0>(phloat3(rb.linearVelocity));
            phydual6vec3 const angularVelocity = dual::MakeVectorVariables<6,3>(phloat3(rb.angularVelocity));
            phydual6vec3 const rk = dual::MakeConstant<6>(phloat3(contact.position - rb.position));
            phydual6vec3 const vk = linerarVelocity + cross(angularVelocity, rk);
            phydual6vec3 const n = dual::MakeConstant<6>(phloat3(contact.normal));
            phydual6 const vkn = dot(vk, n);
            line0.indices[j] = indices[j];
            phloat6 const derivatives = sign * vkn.Derivatives();
            line0.Js[j] = derivatives;
            phloat3 const MJlinear = rb.ooMass * derivatives.SubVector<3>(0);
            phloat3 const MJangular = rb.ooGlobalInertiaMatrix * derivatives.SubVector<3>(3);
            line0.MJs[j].SetSubVector(0, MJlinear);
            line0.MJs[j].SetSubVector(3, MJangular);
            line0.targetVelocity -= sign * vkn.Value();
        }
    }
    SG_BREAKABLE_POS;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulator::GenerateConstraintMatrix(RigidBodySimulation& ioSimulation)
{
    SG_UNUSED(ioSimulation);

    auto EmplaceMatrixElement = [this](size_t i, size_t j, phloat iValue)
    {
        SparseMatrixElement& e = m_LU.EmplaceBack();
        e.i = i;
        e.j = j;
        e.value = iValue;
    };
    size_t const constraintCount = m_constraints.Size();
    m_invD.Resize(constraintCount);
    m_LU.Clear();
    for_range(size_t, i, 0, constraintCount)
    {
        ConstraintLine const& ci = m_constraints[i];
        // TODO: optimize research of linked lines
        for_range(size_t, j, 0, constraintCount)
        {
            if(i == j)
            {
                //EmplaceMatrixElement(i, j, dot(ci.Js[0], ci.MJs[0]) + dot(ci.Js[1], ci.MJs[1]));
                m_invD[i] = 1.f / (dot(ci.Js[0], ci.MJs[0]) + dot(ci.Js[1], ci.MJs[1]));
                continue;
            }
            ConstraintLine const& cj = m_constraints[j];
            if(ci.indices[0] == cj.indices[0])
            {
                if(ci.indices[1] == cj.indices[1])
                {
                    EmplaceMatrixElement(i, j, dot(ci.Js[0], cj.MJs[0]) + dot(ci.Js[1], cj.MJs[1]));
                }
                else
                {
                    EmplaceMatrixElement(i, j, dot(ci.Js[0], ci.MJs[0]));
                }
            }
            else if(ci.indices[0] == cj.indices[1])
            {
                if(ci.indices[1] == cj.indices[0])
                {
                    EmplaceMatrixElement(i, j, dot(ci.Js[0], cj.MJs[1]) + dot(ci.Js[1], cj.MJs[0]));
                }
                else
                {
                    EmplaceMatrixElement(i, j, dot(ci.Js[0], cj.MJs[1]));
                }
            }
            else if(ci.indices[1] == cj.indices[0])
            {
                EmplaceMatrixElement(i, j, dot(ci.Js[1], cj.MJs[0]));
            }
            else if(ci.indices[1] == cj.indices[1])
            {
                EmplaceMatrixElement(i, j, dot(ci.Js[1], cj.MJs[1]));
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulator::SolveConstraints(RigidBodySimulation& ioSimulation)
{
    SG_UNUSED(ioSimulation);

    // Solve A X = b
    // A = D + LU
    // xn+1 = D-1 . ( -LU . xn + b )

    size_t const constraintCount = m_constraints.Size();
    {
        SparseMatrixElement& end = m_LU.EmplaceBack();
        end.i = constraintCount;
        end.j = 0;
        end.value = 0;
    }
    m_lambdas.Clear();
    m_lambdas.Resize(constraintCount, phloat(0));
    // TODO: stop criterion
    for_range(size_t, iter, 0, 100)
    {
        SparseMatrixElement const* elt = m_LU.data();
        for_range(size_t, i, 0, constraintCount)
        {
            ConstraintLine const& ci = m_constraints[i];
            // TODO: fix the time dependence of targetPosition usage
            float oodt = 60.f;
            phloat lambda = ci.targetVelocity + 0.2f * oodt * ci.targetPosition;
            while(i == elt->i)
            {
                lambda -= elt->value * m_lambdas[elt->j];
                ++elt;
            }
            lambda *= m_invD[i];
            phloat const projectedLambda = clamp(lambda, ci.lambdaLo, ci.lambdaHi);
            m_lambdas[i] = projectedLambda;
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RigidBodySimulator::ApplyLambdas(RigidBodySimulation& ioSimulation)
{
    size_t const constraintCount = m_constraints.Size();
    for_range(size_t, i, 0, constraintCount)
    {
        ConstraintLine const& ci = m_constraints[i];
        for_range(size_t, j, 0, 2)
        {
            phloat6 const dv = ci.MJs[j] * m_lambdas[i];
            ioSimulation.m_rigidBodies[ci.indices[j]].linearVelocity  += float3(dv.SubVector<3>(0));
            ioSimulation.m_rigidBodies[ci.indices[j]].angularVelocity += float3(dv.SubVector<3>(3));
        }
    }
}
//=============================================================================
}
}

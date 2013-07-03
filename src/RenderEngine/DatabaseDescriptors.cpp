#include "stdafx.h"

#include "DatabaseDescriptors.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/ShaderConstantDatabase.h>
#include "Compositing.h"
#include "SamplerDescriptor.h"
#include "SurfaceDescriptors.h"
#include <d3d11.h>

namespace sg {
namespace renderengine {
//=============================================================================
AbstractConstantDatabaseDescriptor::AbstractConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
AbstractConstantDatabaseDescriptor::~AbstractConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
reflection::PrimitiveDataType GetVectorValueType(reflection::PrimitiveDataList const& iList)
{
    reflection::PrimitiveDataType valueType = reflection::PrimitiveDataType::Unknown;
    for(auto const& it : iList)
    {
        switch(it->GetType())
        {
        case reflection::PrimitiveDataType::Null:
        case reflection::PrimitiveDataType::Boolean:
            SG_ASSERT_MSG(false, "unsupported type");
        case reflection::PrimitiveDataType::Int32:
            {
                if(reflection::PrimitiveDataType::Unknown == valueType)
                    valueType = reflection::PrimitiveDataType::Int32;
                else
                    SG_ASSERT_MSG(reflection::PrimitiveDataType::Int32 == valueType || reflection::PrimitiveDataType::UInt32 == valueType,
                        "incompatible types in vector");
            }
            break;
        case reflection::PrimitiveDataType::UInt32:
            {
                if(reflection::PrimitiveDataType::Unknown == valueType || reflection::PrimitiveDataType::Int32 == valueType)
                    valueType = reflection::PrimitiveDataType::UInt32;
                else
                    SG_ASSERT_MSG(reflection::PrimitiveDataType::UInt32 == valueType,
                        "incompatible types in vector");
            }
            break;
        case reflection::PrimitiveDataType::Float:
            {
                if(reflection::PrimitiveDataType::Unknown == valueType)
                    valueType = reflection::PrimitiveDataType::Float;
                else
                    SG_ASSERT_MSG(reflection::PrimitiveDataType::Float == valueType,
                        "incompatible types in vector");
            }
            break;
        case reflection::PrimitiveDataType::String:
        case reflection::PrimitiveDataType::Object:
        case reflection::PrimitiveDataType::List:
        case reflection::PrimitiveDataType::NamedList:
            SG_ASSERT_MSG(false, "unsupported type");
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }
    return valueType;
}
template <typename value_type, size_t dim>
void AddVectorConstant(rendering::ShaderConstantName iName, reflection::PrimitiveDataList const& iList, rendering::ShaderConstantDatabase* ioDatabase)
{
    typedef math::Vector<value_type, dim> vector_type;
    vector_type v;
    size_t i = 0;
    for(auto const& it : iList)
    {
        SG_ASSERT(i < dim);
        value_type const x = it->As<value_type>();
        v[i] = x;
        ++i;
    }
    rendering::ShaderVariable<vector_type>* var = new rendering::ShaderVariable<vector_type>();
    var->Set(v);
    ioDatabase->AddVariable(iName, var);
}
typedef void (*AddVectorConstantFct)(rendering::ShaderConstantName iName, reflection::PrimitiveDataList const& iList, rendering::ShaderConstantDatabase* ioDatabase);
AddVectorConstantFct addVectorConstantFcts[3][3] = {
        { AddVectorConstant<i32,   2>, AddVectorConstant<i32,   3>, AddVectorConstant<i32,   4> },
        { AddVectorConstant<u32,   2>, AddVectorConstant<u32,   3>, AddVectorConstant<u32,   4> },
        { AddVectorConstant<float, 2>, AddVectorConstant<float, 3>, AddVectorConstant<float, 4> },
    };
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void AbstractConstantDatabaseDescriptor::PopulateDatabase(rendering::ShaderConstantDatabase* ioDatabase) const
{
    for(auto const& it : m_constants)
    {
        switch(it.second->GetType())
        {
            case reflection::PrimitiveDataType::Null:
            case reflection::PrimitiveDataType::Boolean:
                SG_ASSERT_MSG(false, "unsupported type");
            case reflection::PrimitiveDataType::Int32:
                {
                    rendering::ShaderVariable<i32>* var = new rendering::ShaderVariable<i32>();
                    var->Set(it.second->As<i32>());
                    ioDatabase->AddVariable(it.first, var);
                }
                break;
            case reflection::PrimitiveDataType::UInt32:
                {
                    rendering::ShaderVariable<u32>* var = new rendering::ShaderVariable<u32>();
                    var->Set(it.second->As<u32>());
                    ioDatabase->AddVariable(it.first, var);
                }
                break;
            case reflection::PrimitiveDataType::Float:
                {
                    rendering::ShaderVariable<float>* var = new rendering::ShaderVariable<float>();
                    var->Set(it.second->As<float>());
                    ioDatabase->AddVariable(it.first, var);
                }
                break;
            case reflection::PrimitiveDataType::List:
                {
                    reflection::PrimitiveDataList list;
                    it.second->As(&list);
                    size_t const dim = list.size();
                    SG_ASSERT_MSG(0 < dim, "vector must have a non-nul dimension");
                    SG_ASSERT_MSG(dim <= 4, "vectors with a dimension greater than 4 are not supported");
                    reflection::PrimitiveDataType valueType = GetVectorValueType(list);
                    switch(valueType)
                    {
                    case reflection::PrimitiveDataType::Int32:  addVectorConstantFcts[0][dim-2](it.first, list, ioDatabase); break;
                    case reflection::PrimitiveDataType::UInt32: addVectorConstantFcts[1][dim-2](it.first, list, ioDatabase); break;
                    case reflection::PrimitiveDataType::Float:  addVectorConstantFcts[2][dim-2](it.first, list, ioDatabase); break;
                    default:
                        SG_ASSUME_NOT_REACHED();
                    }
                }
                break;
            case reflection::PrimitiveDataType::String:
            case reflection::PrimitiveDataType::Object:
            case reflection::PrimitiveDataType::NamedList:
                SG_ASSERT_MSG(false, "unsupported type");
            default:
                SG_ASSERT_NOT_REACHED();
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void AbstractConstantDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    for(auto const& it : m_constants)
    {
        switch(it.second->GetType())
        {
            case reflection::PrimitiveDataType::Null:
            case reflection::PrimitiveDataType::Boolean:
                SG_ASSERT_MSG(false, "unsupported type");
            case reflection::PrimitiveDataType::Int32:
            case reflection::PrimitiveDataType::UInt32:
            case reflection::PrimitiveDataType::Float:
            case reflection::PrimitiveDataType::List:
                break;
            case reflection::PrimitiveDataType::String:
            case reflection::PrimitiveDataType::Object:
            case reflection::PrimitiveDataType::NamedList:
                SG_ASSERT_MSG(false, "unsupported type");
            default:
                SG_ASSERT_NOT_REACHED();
        }
    }
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, renderengine), AbstractConstantDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(constants, "list of pairs [key, value], where value can be int, float or float vector or float matrix")
REFLECTION_CLASS_END
//=============================================================================
InputConstantDatabaseDescriptor::InputConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
InputConstantDatabaseDescriptor::~InputConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderConstantDatabase* InputConstantDatabaseDescriptor::CreateDatabase(rendering::IShaderConstantDatabase const* iInputDatabase) const
{
    SG_ASSERT(nullptr != iInputDatabase);
    rendering::ShaderConstantDatabase* database = new rendering::ShaderConstantDatabase();
    SG_CODE_FOR_ASSERT(refptr<rendering::ShaderConstantDatabase> databaseByRef = database;)
    PopulateDatabase(database);
    rendering::IShaderConstantDatabase* databasepair = new rendering::ShaderConstantDatabasePair<false, true>(iInputDatabase, database);
    return databasepair;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void InputConstantDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), InputConstantDatabaseDescriptor)
REFLECTION_CLASS_DOC("Constants for shaders, that can be ovewritten and extended by client")
REFLECTION_CLASS_END
//=============================================================================
ConstantDatabaseDescriptor::ConstantDatabaseDescriptor()
: m_parent()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ConstantDatabaseDescriptor::~ConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderConstantDatabase* ConstantDatabaseDescriptor::CreateDatabase(Compositing* iCompositing) const
{
    rendering::ShaderConstantDatabase* database = new rendering::ShaderConstantDatabase();
    PopulateDatabase(database);
    if(nullptr != m_parent)
    {
        SG_CODE_FOR_ASSERT(refptr<rendering::ShaderConstantDatabase> databaseByRef = database;)
        rendering::IShaderConstantDatabase const* parentDatabase = nullptr == m_parent ? nullptr : iCompositing->GetShaderConstantDatabase(m_parent.get());
        rendering::IShaderConstantDatabase* databasePair = new rendering::ShaderConstantDatabasePair<true, false>(database, parentDatabase);
        return databasePair;
    }
    else
        return database;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void ConstantDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ConstantDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(parent, "parent database descriptor")
REFLECTION_CLASS_END
//=============================================================================
AbstractShaderResourceDatabaseDescriptor::AbstractShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
AbstractShaderResourceDatabaseDescriptor::~AbstractShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void AbstractShaderResourceDatabaseDescriptor::PopulateDatabase(Compositing* iCompositing, rendering::ShaderResourceDatabase* ioDatabase) const
{
    for(auto const& it : m_resources)
    {
        rendering::IShaderResource const* shaderResource = it.second->GetAsShaderResource(iCompositing);
        ioDatabase->AddResource(it.first, shaderResource);
    }

    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    for(auto const& it : m_samplers)
    {
        comptr<ID3D11SamplerState> sampler;
        it.second->CreateSamplerState(renderDevice, sampler);
        ioDatabase->AddSampler(it.first, sampler.get());
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void AbstractShaderResourceDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, renderengine), AbstractShaderResourceDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(resources, "")
    REFLECTION_m_PROPERTY_DOC(samplers, "")
REFLECTION_CLASS_END
//=============================================================================
InputShaderResourceDatabaseDescriptor::InputShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
InputShaderResourceDatabaseDescriptor::~InputShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResourceDatabase* InputShaderResourceDatabaseDescriptor::CreateDatabase(Compositing* iCompositing, rendering::IShaderResourceDatabase const* iInputDatabase) const
{
    SG_ASSERT(nullptr != iInputDatabase);
    rendering::ShaderResourceDatabase* database = new rendering::ShaderResourceDatabase();
    SG_CODE_FOR_ASSERT(refptr<rendering::ShaderResourceDatabase> databaseByRef = database;)
    PopulateDatabase(iCompositing, database);
    rendering::IShaderResourceDatabase* databasePair = new rendering::ShaderResourceDatabasePair<false, true>(iInputDatabase, database);
    return databasePair;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void InputShaderResourceDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), InputShaderResourceDatabaseDescriptor)
REFLECTION_CLASS_DOC("Resources for shaders, that can be ovewritten and extended by client")
REFLECTION_CLASS_END
//=============================================================================
ShaderResourceDatabaseDescriptor::ShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderResourceDatabaseDescriptor::~ShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResourceDatabase* ShaderResourceDatabaseDescriptor::CreateDatabase(Compositing* iCompositing) const
{
    rendering::ShaderResourceDatabase* database = new rendering::ShaderResourceDatabase();
    AbstractShaderResourceDatabaseDescriptor::PopulateDatabase(iCompositing, database);
    if(nullptr != m_parent)
    {
        SG_CODE_FOR_ASSERT(refptr<rendering::ShaderResourceDatabase> databaseByRef = database;)
        rendering::IShaderResourceDatabase const* parentDatabase = iCompositing->GetShaderResourceDatabase(m_parent.get());
        rendering::IShaderResourceDatabase* databasePair = new rendering::ShaderResourceDatabasePair<true, false>(database, parentDatabase);
        return databasePair;
    }
    else
        return database;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ShaderResourceDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(parent, "parent database descriptor")
REFLECTION_CLASS_END
//=============================================================================
}
}


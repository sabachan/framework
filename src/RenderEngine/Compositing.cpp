#include "stdafx.h"

#include "Compositing.h"

#include "CompositingInstruction.h"
#include "CompositingLayer.h"
#include "DatabaseDescriptors.h"
#include "ResolutionDescriptors.h"
#include "SurfaceDescriptors.h"
#include <Rendering/RenderDevice.h>
#include <Rendering/Surface.h>
#include <Rendering/TextureFromFile.h>

namespace sg {
namespace renderengine {
//=============================================================================
Compositing::Compositing(
    CompositingDescriptor const* iDescriptor,
    rendering::RenderDevice const* iRenderDevice,
    ArrayView<rendering::IShaderResource*> const& iInputSurfaces,
    ArrayView<rendering::IRenderTarget*> const& iOutputSurfaces,
    ArrayView<rendering::IShaderConstantDatabase const*> const& iConstantDatabases,
    ArrayView<rendering::IShaderResourceDatabase const*> const& iShaderResourceDatabases)
    : m_descriptor(iDescriptor)
    , m_renderDevice(iRenderDevice)
    , m_currentShaderConstantDatabase(nullptr)
    , m_currentShaderResourceDatabase(nullptr)
#if SG_ENABLE_ASSERT
    , m_isInExecute(false)
#endif
{
    SG_ASSERT(nullptr != m_descriptor);
    SG_ASSERT_MSG(iInputSurfaces.size() == m_descriptor->m_inputSurfaces.size(), "Client must provide expected count of input surfaces");
    m_inputSurfaces.reserve(iInputSurfaces.size());
    for(auto const& it : iInputSurfaces)
        m_inputSurfaces.push_back(it);
    SG_ASSERT_MSG(iOutputSurfaces.size() == m_descriptor->m_outputSurfaces.size(), "Client must provide expected count of output surfaces");
    m_outputSurfaces.reserve(iOutputSurfaces.size());
    for(auto const& it : iOutputSurfaces)
        m_outputSurfaces.push_back(it);
    size_t const constantDatabaseCount = iConstantDatabases.size();

    SG_ASSERT_MSG(m_descriptor->m_inputConstantDatabases.size() == constantDatabaseCount, "Client must provide expected count of input databases");
    for(size_t i = 0; i < constantDatabaseCount; ++i)
    {
        InputConstantDatabaseDescriptor const* desc = m_descriptor->m_inputConstantDatabases[i].second.get();
        m_instanceList.SetInstance(desc, desc->CreateInstance(iConstantDatabases[i]));
    }

    size_t const shaderResourcesDatabaseCount = iShaderResourceDatabases.size();
    SG_ASSERT_MSG(m_descriptor->m_inputShaderResourcesDatabases.size() == shaderResourcesDatabaseCount, "Client must provide expected count of input shader resource databases");
    m_shaderResourceDatabases.reserve(shaderResourcesDatabaseCount);
    for(size_t i = 0; i < shaderResourcesDatabaseCount; ++i)
    {
        InputShaderResourceDatabaseDescriptor const* desc = m_descriptor->m_inputShaderResourcesDatabases[i].second.get();
        m_shaderResourceDatabases.emplace(desc, desc->CreateDatabase(this, iShaderResourceDatabases[i]));
    }

    auto const& instructions = m_descriptor->m_instructions;
    m_instructions.reserve(instructions.size());
    for(auto const& it : instructions)
    {
        auto& instructionVector = m_instructions[it.first];
        SG_ASSERT(instructionVector.empty());
        for(auto const& jt : it.second)
        {
            instructionVector.push_back(jt->CreateInstance(this));
            SG_ASSERT(instructionVector.back() != nullptr);
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Compositing::~Compositing()
{
    SG_ASSERT(nullptr == m_currentShaderConstantDatabase);
    SG_ASSERT(nullptr == m_currentShaderResourceDatabase);
    // Note that members have been ordered to be deleted in correct order
    // regarding safe pointers (eg, ResolutionInstances must be deleted after
    // surfaces, and so on).
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::ResolutionServer const* Compositing::GetResolutionServer(AbstractResolutionDescriptor const* iResolutionDescriptor)
{
    auto const f = m_resolutionServers.find(iResolutionDescriptor);
    if(f == m_resolutionServers.end())
    {
        rendering::ResolutionServer const* resolutionServer = iResolutionDescriptor->CreateInstance(this);
        m_resolutionServers.emplace(iResolutionDescriptor, resolutionServer);
        return resolutionServer;
    }
    else
        return f->second.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IRenderTarget const* Compositing::GetRenderTarget(OutputSurfaceDescriptor const* iSurfaceDescriptor)
{
    size_t const count = m_descriptor->m_outputSurfaces.size();
    for(size_t i = 0; i < count; ++i)
    {
        if(m_descriptor->m_outputSurfaces[i].second == iSurfaceDescriptor)
            return m_outputSurfaces[i].get();
    }
    SG_ASSERT_NOT_REACHED();
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* Compositing::GetShaderResource(InputSurfaceDescriptor const* iSurfaceDescriptor)
{
    size_t const count = m_descriptor->m_inputSurfaces.size();
    for(size_t i = 0; i < count; ++i)
    {
        if(m_descriptor->m_inputSurfaces[i].second == iSurfaceDescriptor)
            return m_inputSurfaces[i].get();
    }
    SG_ASSERT_NOT_REACHED();
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* Compositing::GetShaderResource(TextureSurfaceDescriptor const* iSurfaceDescriptor)
{
    auto const f = m_textures.find(iSurfaceDescriptor);
    if(f == m_textures.end())
    {
        rendering::TextureFromFile const* texture = iSurfaceDescriptor->CreateTexture(this);
        m_textures.emplace(iSurfaceDescriptor, texture);
        return texture;
    }
    else
        return f->second.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::Surface* Compositing::GetMutableSurface(SurfaceDescriptor const* iSurfaceDescriptor)
{
    rendering::BaseSurface* surface = GetBaseSurface(static_cast<AbstractSurfaceDescriptor const*>(iSurfaceDescriptor));
    return checked_cast<rendering::Surface*>(surface);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::Surface const* Compositing::GetSurface(SurfaceDescriptor const* iSurfaceDescriptor)
{
    rendering::BaseSurface const* surface = GetBaseSurface(static_cast<AbstractSurfaceDescriptor const*>(iSurfaceDescriptor));
    return checked_cast<rendering::Surface const*>(surface);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::BCSurface const* Compositing::GetBCSurface(BCSurfaceDescriptor const* iSurfaceDescriptor)
{
    rendering::BaseSurface const* surface = GetBaseSurface(static_cast<AbstractSurfaceDescriptor const*>(iSurfaceDescriptor));
    return checked_cast<rendering::BCSurface const*>(surface);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::DepthStencilSurface const* Compositing::GetDepthStencilSurface(DepthStencilSurfaceDescriptor const* iSurfaceDescriptor)
{
    rendering::BaseSurface const* surface = GetBaseSurface(static_cast<AbstractSurfaceDescriptor const*>(iSurfaceDescriptor));
    return checked_cast<rendering::DepthStencilSurface const*>(surface);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::BaseSurface* Compositing::GetBaseSurface(AbstractSurfaceDescriptor const* iSurfaceDescriptor)
{
    SG_ASSERT(nullptr != iSurfaceDescriptor);
    auto const f = m_surfaces.find(iSurfaceDescriptor);
    if(f == m_surfaces.end())
    {
        rendering::BaseSurface* surface = iSurfaceDescriptor->CreateSurface(this);
        m_surfaces.emplace(iSurfaceDescriptor, surface);
        return surface;
    }
    else
        return f->second.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderConstantDatabase const* Compositing::GetShaderConstantDatabase(AbstractConstantDatabaseDescriptor const* iDescriptor)
{
    SG_ASSERT(nullptr != iDescriptor);
    GenericConstantDatabaseInstance const* instance = m_instanceList.GetInstanceIFP(iDescriptor);
    if(nullptr == instance)
    {
        ConstantDatabaseDescriptor const* desc = checked_cast<ConstantDatabaseDescriptor const*>(iDescriptor);
        ConstantDatabaseInstance* newInstance = desc->CreateInstance(this);
        m_instanceList.SetInstance(desc, newInstance);
        instance = newInstance;
    }
    return instance->GetDatabase();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::ShaderConstantDatabase* Compositing::GetWritableShaderConstantDatabase(ConstantDatabaseDescriptor const* iDescriptor)
{
    ConstantDatabaseInstance* instance = m_instanceList.GetOrCreateInstance(iDescriptor, this);
    return instance->GetWritableDatabase();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResourceDatabase const* Compositing::GetShaderResourceDatabase(AbstractShaderResourceDatabaseDescriptor const* iDescriptor)
{
    // TODO: use m_instanceList
    SG_ASSERT(nullptr != iDescriptor);
    auto const f = m_shaderResourceDatabases.find(iDescriptor);
    if(f == m_shaderResourceDatabases.end())
    {
        ShaderResourceDatabaseDescriptor const* desc = checked_cast<ShaderResourceDatabaseDescriptor const*>(iDescriptor);
        rendering::IShaderResourceDatabase* database = desc->CreateDatabase(this);
        m_shaderResourceDatabases.emplace(desc, database);
        return database;
    }
    else
        return f->second.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Compositing::SetCurrentShaderConstantDatabase(rendering::IShaderConstantDatabase const* iDatabase)
{
    SG_ASSERT(m_isInExecute);
    m_currentShaderConstantDatabase = iDatabase;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Compositing::SetCurrentShaderResourceDatabase(rendering::IShaderResourceDatabase const* iDatabase)
{
    SG_ASSERT(m_isInExecute);
    m_currentShaderResourceDatabase = iDatabase;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
QuadForShaderPass* Compositing::GetQuadForShaderPass()
{
    if(nullptr == m_quadForShaderPass)
        m_quadForShaderPass.reset(new QuadForShaderPass(m_renderDevice.get()));
    return m_quadForShaderPass.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Compositing::RegisterResolutionInstanceForHandling(ResolutionInstance* iInstance)
{
    m_resolutionInstanceHandles.push_back(iInstance);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Compositing::HasInstructionBlock(FastSymbol iInstructionBlockName)
{
    SG_ASSERT(!m_isInExecute);
    auto const& f = m_instructions.find(iInstructionBlockName);
    return m_instructions.end() != f;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Compositing::Execute(FastSymbol iInstructionBlockName)
{
    SG_ASSERT(!m_isInExecute);
    SG_ASSERT(nullptr == m_currentShaderConstantDatabase);
    SG_ASSERT(nullptr == m_currentShaderResourceDatabase);
    SG_CODE_FOR_ASSERT(m_isInExecute = true;)
    m_renderDevice->SetDefaultState();
    auto const& f = m_instructions.find(iInstructionBlockName);
    SG_ASSERT_MSG(m_instructions.end() != f, "unknown instruction block");
    for(auto const& it : f->second)
    {
        SG_ASSERT(nullptr != it);
        it->Execute(this);
    }
    SG_ASSERT(m_isInExecute);
    SG_CODE_FOR_ASSERT(m_isInExecute = false;)
    m_currentShaderConstantDatabase = nullptr;
    m_currentShaderResourceDatabase = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::BaseSurface const* Compositing::GetExposedSurfaceIFP(FastSymbol iName)
{
    SG_ASSERT(!m_isInExecute);
    auto const& f = m_descriptor->m_exposedSurfaces.find(iName);
    if(m_descriptor->m_exposedSurfaces.end() == f)
        return nullptr;
    return f->second->GetAsSurface(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* Compositing::GetExposedShaderResourceIFP(FastSymbol iName)
{
    SG_ASSERT(!m_isInExecute);
    auto const& f = m_descriptor->m_exposedSurfaces.find(iName);
    if(m_descriptor->m_exposedSurfaces.end() == f)
        return nullptr;
    return f->second->GetAsShaderResource(this);
}
//=============================================================================
CompositingLayer* Compositing::GetLayer(FastSymbol iInstructionBlockName, ArrayView<FastSymbol const> const& iSpecRequest)
{
    auto const& f = m_descriptor->m_instructions.find(iInstructionBlockName);
    SG_ASSERT_MSG(m_descriptor->m_instructions.end() != f, "unknown instruction block");
    size_t const instructionCount = f->second.size();
    auto const& g = m_instructions.find(iInstructionBlockName);
    SG_ASSERT(m_instructions.end() != g);
    SG_ASSERT(g->second.size() == instructionCount);
    CompositingLayer* foundLayer = nullptr;
    for(size_t i = 0; i < instructionCount; ++i)
    {
        AbstractCompositingInstructionDescriptor const* it = f->second[i].get();
        SG_ASSERT(nullptr != it);
        if(it->GetMetaclass() == CompositingLayerDescriptor::StaticGetMetaclass())
        {
            CompositingLayerDescriptor const* desc = checked_cast<CompositingLayerDescriptor const*>(it);
            if(desc->DoesMatch(iSpecRequest))
            {
                ICompositingInstruction* inst = g->second[i].get();
                SG_ASSERT(nullptr != inst);
                CompositingLayer* layer = checked_cast<CompositingLayer*>(inst);
#if SG_ENABLE_ASSERT
                SG_ASSERT_MSG(nullptr == foundLayer, "Multiple layers match the request. Please be more precise.");
                foundLayer = layer;
#else
                return layer;
#endif
            }
        }
    }
    SG_ASSERT_MSG(nullptr != foundLayer, "No CompositingLayer matches request");
    return foundLayer;
}
//=============================================================================
CompositingDescriptor::CompositingDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CompositingDescriptor::~CompositingDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ICompositing* CompositingDescriptor::CreateInstance(
    rendering::RenderDevice const* iRenderDevice,
    ArrayView<rendering::IShaderResource*> const& iInputSurfaces,
    ArrayView<rendering::IRenderTarget*> const& iOutputSurfaces,
    ArrayView<rendering::IShaderConstantDatabase const*> const& iConstantDatabases,
    ArrayView<rendering::IShaderResourceDatabase const*> const& iShaderResourceDatabases) const
{
    return new Compositing(this, iRenderDevice, iInputSurfaces, iOutputSurfaces, iConstantDatabases, iShaderResourceDatabases);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void CompositingDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,renderengine), CompositingDescriptor)
    REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(instructions, "")
    REFLECTION_m_PROPERTY_DOC(inputSurfaces, "")
    REFLECTION_m_PROPERTY_DOC(outputSurfaces, "")
    REFLECTION_m_PROPERTY_DOC(exposedSurfaces, "surfaces that can be read as shader resources from outside")
    REFLECTION_m_PROPERTY_DOC(inputConstantDatabases, "")
    REFLECTION_m_PROPERTY_DOC(inputShaderResourcesDatabases, "")
REFLECTION_CLASS_END
//=============================================================================
}
}


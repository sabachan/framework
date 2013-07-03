#include "stdafx.h"

#include "RenderBatchDico.h"

#include "Material.h"
#include "RenderBatchSet.h"
#include "RenderDevice.h"
#include "ShaderResource.h"
#include <unordered_map>

namespace sg {
namespace rendering {
//=============================================================================
RenderBatchDico::RenderBatchDico(RenderBatchSet* iRenderBatchSetOwner)
{
    SG_UNUSED(iRenderBatchSetOwner);
    SG_CODE_FOR_ASSERT(m_renderDeviceForCheck = rendering::RenderDevice::GetIFP());
    SG_ASSERT(nullptr != m_renderDeviceForCheck);
    SG_CODE_FOR_ASSERT(m_renderBatchSetForCheck = iRenderBatchSetOwner);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RenderBatchDico::~RenderBatchDico()
{
    SG_ASSERT_MSG(m_transientDico.renderBatches.empty(), "It is the responsibility of owner to clear dico");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TransientRenderBatch* RenderBatchDico::GetOrCreateTransientRenderBatch(
    RenderBatchSet* iRenderBatchSetOwner,
    Material const& iMaterial,
    TransientRenderBatch::Properties const& iProperties)
{
    SG_ASSERT(m_renderBatchSetForCheck == iRenderBatchSetOwner);
    rendering::RenderDevice* renderDevice = rendering::RenderDevice::GetIFP();
    SG_ASSERT(m_renderDeviceForCheck == renderDevice);

    SG_ASSERT(0 == (uintptr_t(&iMaterial) & 0x3));
    SG_ASSERT(2 == iProperties.indexSize || 4 == iProperties.indexSize);
    uintptr_t ptrHash = uintptr_t(&iMaterial)
        ^ (iProperties.indexSize >> 2);

    auto fromPtrRange = m_transientDico.indexFromPtr.equal_range(ptrHash);
    for(auto it = fromPtrRange.first; it != fromPtrRange.second; ++it)
    {
        size_t const index = it->second;
        SG_ASSERT(index < m_transientDico.renderBatches.size());
        auto const& batch = m_transientDico.renderBatches[index];
        TransientRenderBatch* renderBatch = batch.get();
        SG_ASSERT(nullptr != renderBatch);
        if(iMaterial == *renderBatch->GetMaterial() && iProperties == renderBatch->GetProperties())
            return renderBatch;
    }

    auto const fromPtrIter = m_transientDico.indexFromPtr.emplace_hint(fromPtrRange.second, ptrHash, -1);

    MaterialKey materialKey(&iMaterial, iProperties.indexSize);
    auto foundInFromMat = m_transientDico.indexFromMat.find(materialKey);
    if(m_transientDico.indexFromMat.end() != foundInFromMat)
    {
        size_t const index = foundInFromMat->second;
        SG_ASSERT(index < m_transientDico.renderBatches.size());
        TransientRenderBatch* renderBatch = m_transientDico.renderBatches[index].get();
        SG_ASSERT(nullptr != renderBatch);
        SG_ASSERT(iMaterial == *renderBatch->GetMaterial());
        SG_ASSERT(iProperties == renderBatch->GetProperties());
        fromPtrIter->second = index;
        return renderBatch;
    }
    else
    {
        size_t const index = m_transientDico.renderBatches.size();
        Material const* materialCopy = new Material(iMaterial);
        auto const fromMatResult = m_transientDico.indexFromMat.emplace(MaterialKey(materialCopy, iProperties.indexSize), index);
        SG_ASSERT(fromMatResult.second);
        fromPtrIter->second = index;
        TransientRenderBatch* renderBatch = new TransientRenderBatch(renderDevice, materialCopy, iProperties);
        m_transientDico.renderBatches.emplace_back(renderBatch);
        iRenderBatchSetOwner->Insert(renderBatch);
        SG_ASSERT(materialCopy->IsRefCounted_ForAssert());
        return renderBatch;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatchDico::Clear(RenderBatchSet* iRenderBatchSetOwner)
{
    SG_ASSERT(m_renderBatchSetForCheck == iRenderBatchSetOwner);
    for(auto const& it : m_transientDico.renderBatches)
    {
        iRenderBatchSetOwner->Remove(it.get());
    }
    m_transientDico.indexFromPtr.clear();
    m_transientDico.indexFromMat.clear();
    m_transientDico.renderBatches.clear();
}
//=============================================================================
}
}

#include "stdafx.h"

#include "RenderBatchSet.h"
#include "RenderBatch.h"
#include "RenderBatchDico.h"
#include <Core/ArrayList.h>
#include <Core/For.h>
#include <random>

namespace sg {
namespace rendering {
//=============================================================================
RenderBatchSet::RenderBatchSet()
: m_renderBatches()
, m_needPrioritySort(false)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RenderBatchSet::~RenderBatchSet()
{
    SG_ASSERT(m_renderBatches.empty());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatchSet::Insert(IRenderBatch* iRenderBatch, RenderBatchPassId iPassId)
{
    SG_ASSERT(nullptr == iRenderBatch->m_setWhereRegistered);
    auto r = m_renderBatches.Emplace(RenderBatchPass { iRenderBatch, iPassId });
    SG_ASSERT_MSG(r.second, "batch already in set !");
    SG_CODE_FOR_ASSERT(iRenderBatch->m_setWhereRegistered = this;)
    int const priority = iRenderBatch->GetPriority(iPassId);
    if(0 != priority)
        m_needPrioritySort = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatchSet::Remove(rendering::IRenderBatch* iRenderBatch, RenderBatchPassId iPassId)
{
    SG_ASSERT(this == iRenderBatch->m_setWhereRegistered);
    bool const r = m_renderBatches.Erase(RenderBatchPass { iRenderBatch, iPassId });
    SG_ASSERT_MSG_AND_UNUSED(r, "batch was not in set !");
    SG_CODE_FOR_ASSERT(iRenderBatch->m_setWhereRegistered = nullptr;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatchSet::Execute(RenderDevice const* iRenderDevice,
                             IShaderConstantDatabase const* iShaderConstantDatabase,
                             IShaderResourceDatabase const* iShaderResourceDatabase)
{
    struct BatchToRender
    {
        rendering::IRenderBatch* batch;
        rendering::RenderBatchPassId pass;
        int priority;
        size_t nextSublayer;
        size_t maxSublayer;
    };
    ArrayList<BatchToRender> remainingBatches[2];
    remainingBatches[0].reserve(m_renderBatches.size());
    remainingBatches[1].reserve(m_renderBatches.size());
    size_t writeBufferIndex = 0;
    size_t subLayer = 0;
    size_t nextSubLayer = all_ones;
#if SG_ENABLE_ASSERT
    bool const randomizeOrder = true;
#else
    bool const randomizeOrder = false;
#endif
    if(SG_POTENTIAL_CONSTANT_CONDITION(randomizeOrder || m_needPrioritySort))
    {
        bool stillNeedPrioritySort = false;
        for(auto const& it : m_renderBatches)
        {
            int const priority = it.batch->GetPriority(it.pass);
            SG_ASSERT(priority == priority);
            if(0 != priority)
                stillNeedPrioritySort = true;
            remainingBatches[writeBufferIndex].EmplaceBack_AssumeCapacity(BatchToRender { it.batch.get(), it.pass, priority, 0, 0 });
        }
#if SG_ENABLE_ASSERT
        if(SG_CONSTANT_CONDITION(randomizeOrder))
        {
            static unsigned int seed = 0;
            std::mt19937 gen(seed);
            size_t const size = remainingBatches[writeBufferIndex].size();
            for_range(size_t, i, 0, size)
            {
                std::uniform_int_distribution<size_t> dist(i, size-1);
                size_t const src = dist(gen);
                if(src != i)
                {
                    using std::swap;
                    swap(remainingBatches[writeBufferIndex][i], remainingBatches[writeBufferIndex][src]);
                }
            }
            seed = gen();
        }
#endif
        if(stillNeedPrioritySort)
        {
            std::sort(
                remainingBatches[writeBufferIndex].begin(),
                remainingBatches[writeBufferIndex].end(),
                [](BatchToRender const& a, BatchToRender const& b)
                {
                    return a.priority < b.priority;
                });
        }
        else
        {
            m_needPrioritySort = false;
        }

        size_t const readBufferIndex = writeBufferIndex;
        writeBufferIndex = (writeBufferIndex + 1) & 1;
        for(auto const& it : remainingBatches[readBufferIndex])
        {
            it.batch->PreExecute(it.pass, iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase);
            size_t const subLayerEnd = it.batch->GetSubLayerEnd(it.pass);
            if(subLayerEnd > 0)
            {
                size_t const maxSubLayer = subLayerEnd - 1;
                size_t nextSubLayerForIt = subLayer + 1;
                it.batch->Execute(it.pass, iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase, subLayer, nextSubLayerForIt);
                SG_ASSERT(subLayer < nextSubLayerForIt);
                if(maxSubLayer > subLayer)
                {
                    remainingBatches[writeBufferIndex].EmplaceBack_AssumeCapacity(BatchToRender { it.batch, it.pass, it.priority, nextSubLayerForIt, maxSubLayer });
                    nextSubLayer = std::min(nextSubLayer, nextSubLayerForIt);
                }
            }
        }
    }
    else
    {
        for(auto const& it : m_renderBatches)
        {
            it.batch->PreExecute(it.pass, iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase);
            size_t const subLayerEnd = it.batch->GetSubLayerEnd(it.pass);
            if(subLayerEnd > 0)
            {
                size_t const maxSubLayer = subLayerEnd - 1;
                size_t nextSubLayerForIt = subLayer + 1;
                it.batch->Execute(it.pass, iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase, subLayer, nextSubLayerForIt);
                SG_ASSERT(subLayer < nextSubLayerForIt);
                if(maxSubLayer > subLayer)
                {
                    int const priority = 0;
                    remainingBatches[writeBufferIndex].EmplaceBack_AssumeCapacity(BatchToRender { it.batch.get(), it.pass, priority, nextSubLayerForIt, maxSubLayer });
                    nextSubLayer = std::min(nextSubLayer, nextSubLayerForIt);
                }
            }
        }
    }
    remainingBatches[1].reserve(remainingBatches[0].size());
    while(!remainingBatches[writeBufferIndex].empty())
    {
        SG_ASSERT(subLayer < nextSubLayer);
        subLayer = nextSubLayer;
        nextSubLayer = all_ones;
        size_t const readBufferIndex = writeBufferIndex;
        writeBufferIndex = (writeBufferIndex + 1) & 1;
        remainingBatches[writeBufferIndex].clear();
        for(auto const& it : remainingBatches[readBufferIndex])
        {
            size_t nextSubLayerForIt = it.nextSublayer;
            SG_ASSERT(nextSubLayerForIt >= subLayer);
            if(nextSubLayerForIt == subLayer)
            {
                nextSubLayerForIt += 1;
                it.batch->Execute(it.pass, iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase, subLayer, nextSubLayerForIt);
                SG_ASSERT(subLayer < nextSubLayerForIt);
            }
            size_t const maxSubLayer = it.maxSublayer;
            size_t const subLayerEnd = it.batch->GetSubLayerEnd(it.pass);
            SG_ASSERT(it.batch->GetSubLayerEnd(it.pass) == maxSubLayer + 1);
            if(maxSubLayer > subLayer)
            {
                remainingBatches[writeBufferIndex].EmplaceBack_AssumeCapacity(BatchToRender { it.batch, it.pass, it.priority, nextSubLayerForIt, maxSubLayer });
                nextSubLayer = std::min(nextSubLayer, nextSubLayerForIt);
            }
        }
    }
    for(auto const& it : m_renderBatches)
    {
        it.batch->PostExecute(it.pass);
    }
}
//=============================================================================
}
}

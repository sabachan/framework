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
void RenderBatchSet::Insert(rendering::IRenderBatch* iRenderBatch)
{
    SG_ASSERT(nullptr == iRenderBatch->m_setWhereRegistered);
    auto r = m_renderBatches.insert(iRenderBatch);
    SG_ASSERT_MSG(r.second, "batch already in set !");
    SG_CODE_FOR_ASSERT(iRenderBatch->m_setWhereRegistered = this;)
    int const priority = iRenderBatch->GetPriority();
    if(0 != priority)
        m_needPrioritySort = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatchSet::Remove(rendering::IRenderBatch* iRenderBatch)
{
    SG_ASSERT(this == iRenderBatch->m_setWhereRegistered);
    size_t const r = m_renderBatches.erase(iRenderBatch);
    SG_ASSERT_MSG_AND_UNUSED(r, "batch was not in set !");
    SG_CODE_FOR_ASSERT(iRenderBatch->m_setWhereRegistered = nullptr;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatchSet::Execute(RenderDevice const* iRenderDevice,
                             IShaderConstantDatabase const* iShaderConstantDatabase,
                             IShaderResourceDatabase const* iShaderResourceDatabase)
{
    typedef std::tuple<rendering::IRenderBatch*, size_t, size_t> batch_tuple_type;
    ArrayList<batch_tuple_type> remainingBatches[2];
    remainingBatches[0].reserve(m_renderBatches.size());
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
            int const priority = it->GetPriority();
            SG_ASSERT(int(size_t(priority)) == priority);
            if(0 != priority)
                stillNeedPrioritySort = true;
            remainingBatches[writeBufferIndex].push_back(std::make_tuple(it.get(), size_t(priority), 0));
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
                [](batch_tuple_type const& a, batch_tuple_type const& b)
                {
                    return int(std::get<1>(a)) < int(std::get<1>(b));
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
            std::get<0>(it)->PreExecute(iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase);
            size_t const subLayerCount = std::get<0>(it)->GetSubLayerCount();
            if(subLayerCount > 0)
            {
                size_t const maxSubLayer = subLayerCount - 1;
                size_t nextSubLayerForIt = subLayer + 1;
                std::get<0>(it)->Execute(iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase, subLayer, nextSubLayerForIt);
                SG_ASSERT(subLayer < nextSubLayerForIt);
                if(maxSubLayer > subLayer)
                {
                    remainingBatches[writeBufferIndex].emplace_back(std::get<0>(it), nextSubLayerForIt, maxSubLayer);
                    nextSubLayer = std::min(nextSubLayer, nextSubLayerForIt);
                }
            }
        }
    }
    else
    {
        for(auto const& it : m_renderBatches)
        {
            it->PreExecute(iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase);
            size_t const subLayerCount = it->GetSubLayerCount();
            if(subLayerCount > 0)
            {
                size_t const maxSubLayer = subLayerCount - 1;
                size_t nextSubLayerForIt = subLayer + 1;
                it->Execute(iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase, subLayer, nextSubLayerForIt);
                SG_ASSERT(subLayer < nextSubLayerForIt);
                if(maxSubLayer > subLayer)
                {
                    remainingBatches[writeBufferIndex].emplace_back(it.get(), nextSubLayer, maxSubLayer);
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
            size_t nextSubLayerForIt = std::get<1>(it);
            SG_ASSERT(nextSubLayerForIt >= subLayer);
            if(nextSubLayerForIt == subLayer)
            {
                nextSubLayerForIt += 1;
                std::get<0>(it)->Execute(iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase, subLayer, nextSubLayerForIt);
                SG_ASSERT(subLayer < nextSubLayerForIt);
            }
            size_t const maxSubLayer = std::get<2>(it);
            SG_ASSERT(std::get<0>(it)->GetSubLayerCount() - 1 == maxSubLayer);
            if(maxSubLayer > subLayer)
            {
                remainingBatches[writeBufferIndex].emplace_back(std::get<0>(it), nextSubLayerForIt, maxSubLayer);
                nextSubLayer = std::min(nextSubLayer, nextSubLayerForIt);
            }
        }
    }
    for(auto const& it : m_renderBatches)
    {
        it->PostExecute();
    }
}
//=============================================================================
}
}

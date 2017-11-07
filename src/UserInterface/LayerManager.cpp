#include "stdafx.h"

#include "LayerManager.h"
#include <Core/Cast.h>
#include <Core/For.h>
#include <Core/Log.h>
#include <Core/MaxSizeVector.h>
#include <Core/StringFormat.h>
#include <Math/Box.h>
#if SG_ENABLE_UNIT_TESTS
#include <Core/TestFramework.h>
#include <random>
#endif
#if SG_ENABLE_ASSERT
#include <Image/DebugImage.h>
#include <sstream>
#endif

namespace sg {
namespace ui {
//=============================================================================

#define ENABLE_LAYER_MANAGER_STATS (SG_ENABLE_ASSERT && 0)

#if ENABLE_LAYER_MANAGER_STATS
namespace
{
    size_t g_sumVisitedPreBoxCount = 0;
    size_t g_sumVisitedBoxCount = 0;
    size_t g_sumVisitedBoundingBoxCount = 0;
    size_t g_sumVisitedLayerCount = 0;
    size_t g_sumInputBoxCount = 0;
#if ENABLE_LAYER_MANAGER_STATS
    void PrintLayerManagerStatsAndClear()
    {
        SG_LOG_DEBUG("UI", Format("LayerManager - %0 visited layers / input box", float(g_sumVisitedLayerCount) / g_sumInputBoxCount));
        if(0 != g_sumVisitedPreBoxCount)
            SG_LOG_DEBUG("UI", Format("LayerManager - %0 visited pre boxes / input box", float(g_sumVisitedPreBoxCount) / g_sumInputBoxCount));
        SG_LOG_DEBUG("UI", Format("LayerManager - %0 visited boxes / input box", float(g_sumVisitedBoxCount) / g_sumInputBoxCount));
        if(0 != g_sumVisitedBoundingBoxCount)
            SG_LOG_DEBUG("UI", Format("LayerManager - %0 visited bounding boxes / input box", float(g_sumVisitedBoundingBoxCount) / g_sumInputBoxCount));
        g_sumVisitedPreBoxCount = 0;
        g_sumVisitedBoxCount = 0;
        g_sumVisitedBoundingBoxCount = 0;
        g_sumVisitedLayerCount = 0;
        g_sumInputBoxCount = 0;
    }
#endif
}
#endif
//=============================================================================
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ONE_LAYER_PER_REQUEST || defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_OneLayerPerRequest::LayerManager_OneLayerPerRequest()
: m_index(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_OneLayerPerRequest::~LayerManager_OneLayerPerRequest()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerRange LayerManager_OneLayerPerRequest::Lock(box2f const& iBox, size_t iLayerCount)
{
    SG_UNUSED(iBox);
    size_t const end = m_index + iLayerCount;
    LayerRange const ret(checked_numcastable(m_index), checked_numcastable(end));
    m_index = end;
    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_OneLayerPerRequest::Clear()
{
    m_index = 0;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ALL_BOXES || defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_AllBoxes::LayerManager_AllBoxes()
    : m_layers()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_AllBoxes::~LayerManager_AllBoxes()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerRange LayerManager_AllBoxes::Lock(box2f const& iBox, size_t iLayerCount)
{
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumInputBoxCount;
#endif
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    size_t const layerCount = m_layers.size();
    size_t layerIndex = layerCount;
    bool collisionFound = false;
    do
    {
        if (0 == layerIndex)
        {
            collisionFound = true;
            break;
        }
#if ENABLE_LAYER_MANAGER_STATS
        ++g_sumVisitedLayerCount;
#endif
        --layerIndex;
        SG_ASSERT(layerIndex < layerCount);
        std::vector<box2f> const& layerBoxes = m_layers[layerIndex];
        for (auto const& box : layerBoxes)
        {
#if ENABLE_LAYER_MANAGER_STATS
            ++g_sumVisitedBoxCount;
#endif
            if (IntersectStrict(iBox, box))
            {
                ++layerIndex;
                collisionFound = true;
                break;
            }
        }
    } while (!collisionFound);

    SG_ASSERT(layerIndex <= layerCount);
    size_t const end = layerIndex + iLayerCount;
    while (end >= m_layers.size())
        m_layers.emplace_back();

    SG_ASSERT(end <= m_layers.size());
    m_layers[end - 1].emplace_back(iBox);
    LayerRange ret(checked_numcastable(layerIndex), checked_numcastable(end));
    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_AllBoxes::Clear()
{
    //SG_LOG_DEBUG("UI", Format("LayerManager - %0 layers", m_layers.size()));
#if ENABLE_LAYER_MANAGER_STATS
    PrintLayerManagerStatsAndClear();
#endif
    m_layers.clear();
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ONE_BOX_PER_LAYER || defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_OneBoxPerLayer::LayerManager_OneBoxPerLayer()
    : m_layers()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_OneBoxPerLayer::~LayerManager_OneBoxPerLayer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerRange LayerManager_OneBoxPerLayer::Lock(box2f const& iBox, size_t iLayerCount)
{
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumInputBoxCount;
#endif
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    size_t const layerCount = m_layers.size();
    size_t layerIndex = layerCount;
    bool collisionFound = false;
    do
    {
        if (0 == layerIndex)
        {
            collisionFound = true;
            break;
        }
#if ENABLE_LAYER_MANAGER_STATS
        ++g_sumVisitedLayerCount;
#endif
        --layerIndex;
        SG_ASSERT(layerIndex < layerCount);
        box2f const& layerBox = m_layers[layerIndex];
#if ENABLE_LAYER_MANAGER_STATS
        ++g_sumVisitedBoxCount;
#endif
        if (IntersectStrict(iBox, layerBox))
        {
            ++layerIndex;
            collisionFound = true;
            break;
        }
    } while (!collisionFound);

    SG_ASSERT(layerIndex <= layerCount);
    size_t const end = layerIndex + iLayerCount;
    while (end >= m_layers.size())
        m_layers.emplace_back();

    SG_ASSERT(end <= m_layers.size());
    SG_ASSERT(!IntersectStrict(iBox, m_layers[end-1]));
    m_layers[end - 1].Grow(iBox);
    LayerRange ret(checked_numcastable(layerIndex), checked_numcastable(end));
    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_OneBoxPerLayer::Clear()
{
    //SG_LOG_DEBUG("UI", Format("LayerManager - %0 layers", m_layers.size()));
#if ENABLE_LAYER_MANAGER_STATS
    PrintLayerManagerStatsAndClear();
#endif
    m_layers.clear();
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_N_BOXES_PER_LAYER || defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_NBoxesPerLayer::LayerManager_NBoxesPerLayer()
    : m_layers()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_NBoxesPerLayer::~LayerManager_NBoxesPerLayer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerRange LayerManager_NBoxesPerLayer::Lock(box2f const& iBox, size_t iLayerCount)
{
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumInputBoxCount;
#endif
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    size_t const layerCount = m_layers.size();
    size_t layerIndex = layerCount;
    bool collisionFound = false;
    do
    {
        if (0 == layerIndex)
        {
            collisionFound = true;
            break;
        }
#if ENABLE_LAYER_MANAGER_STATS
        ++g_sumVisitedLayerCount;
#endif
        --layerIndex;
        SG_ASSERT(layerIndex < layerCount);
        MaxSizeVector<box2f, N>& boxes = m_layers[layerIndex];
        for_range(size_t, i, 0, boxes.size())
        {
            box2f const& layerBox = boxes[i];
#if ENABLE_LAYER_MANAGER_STATS
            ++g_sumVisitedBoxCount;
#endif
            if(IntersectStrict(iBox, layerBox))
            {
                ++layerIndex;
                collisionFound = true;
                break;
            }
        }
    } while (!collisionFound);

    size_t const layerEndIndex = layerIndex + iLayerCount;
    SG_ASSERT(layerIndex <= layerCount);
    if (layerEndIndex > layerCount)
    {
        for(size_t i = layerCount; i < layerEndIndex; ++i)
        {
            m_layers.emplace_back();
        }
        SG_ASSERT(layerEndIndex == m_layers.size());
        m_layers.back().emplace_back(iBox);
    }
    else
    {
        MaxSizeVector<box2f, N>& boxes = m_layers[layerEndIndex-1];
        if(boxes.size() < N)
        {
#if SG_ENABLE_ASSERT
            for_range(size_t, i, 0, boxes.size())
            {
                SG_ASSERT(!IntersectStrict(iBox, boxes[i]));
            }
#endif
            boxes.emplace_back(iBox);
        }
        else
        {
            std::tuple<size_t, size_t, float> best = std::make_tuple(all_ones, all_ones, std::numeric_limits<float>::max());
            float const boxVolume = iBox.NVolume_AssumeConvex();
            for_range(size_t, i, 0, boxes.size())
            {
                box2f const& boxi = boxes[i];
                SG_ASSERT(!IntersectStrict(iBox, boxi));
                float const boxiVolume = boxi.NVolume_AssumeConvex();
                {
                    box2f growBB = boxi;
                    growBB.Grow(iBox);
                    float const growVolume = growBB.NVolume_AssumeConvex();
                    float const unusedVolume = growVolume - boxiVolume - boxVolume;
                    if(unusedVolume < std::get<2>(best))
                    {
                        best = std::make_tuple(i, all_ones, unusedVolume);
                    }
                }
                for_range(size_t, j, i+1, boxes.size())
                {
                    box2f const& boxj = boxes[j];
                    box2f growBB = boxi;
                    growBB.Grow(boxj);
                    float const growVolume = growBB.NVolume_AssumeConvex();
                    float const boxjVolume = boxj.NVolume_AssumeConvex();
                    float const unusedVolume = growVolume - boxiVolume - boxjVolume;
                    if(unusedVolume < std::get<2>(best))
                    {
                        best = std::make_tuple(i, j, unusedVolume);
                    }
                }
            }
            if(all_ones == std::get<1>(best))
            {
                size_t const i = std::get<0>(best);
                boxes[i].Grow(iBox);
            }
            else
            {
                size_t const i = std::get<0>(best);
                size_t const j = std::get<1>(best);
                boxes[i].Grow(boxes[j]);
                boxes[j] = iBox;
            }
        }
    }
    LayerRange ret(checked_numcastable(layerIndex), checked_numcastable(layerEndIndex));
    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_NBoxesPerLayer::Clear()
{
    //SG_LOG_DEBUG("UI", Format("LayerManager - %0 layers", m_layers.size()));
#if ENABLE_LAYER_MANAGER_STATS
    PrintLayerManagerStatsAndClear();
#endif
    m_layers.clear();
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_QUAD_TREE_LIKE || defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_QuadTreeLike::LayerManager_QuadTreeLike()
: m_nodes()
, m_layers()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_QuadTreeLike::~LayerManager_QuadTreeLike()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
    template<typename Container>
    void PushIFN(Container& toExplore, size_t nodeIndex)
    {
        if(all_ones != nodeIndex)
            toExplore.push_back(nodeIndex);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//              |       |
//      0       |   4   |       1
// -------------+-------+--------------
//      5       |  Box  |       6
// -------------+-------+--------------
//      2       |   7   |       3
//              |       |
// 0, 1, 2 and 3 are quarter-planes,
// 4, 5, 6 and 7 are half-planes, including previous areas.
LayerRange LayerManager_QuadTreeLike::Lock(box2f const& iBox, size_t iLayerCount)
{
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumInputBoxCount;
#endif
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    size_t const layerCount = m_layers.size();
    size_t reverseLayer = 0;
    MaxSizeVector<size_t, 1024> toExplore;
    MaxSizeVector<std::pair<size_t, size_t>, 256> insertPoints;
    SG_ASSERT_MSG(m_layers.size() < insertPoints.capacity() / 2, "If this assert fails, insertPoints capacity should be increased");
    bool collisionFound = false;
    do
    {
        if(reverseLayer >= layerCount)
        {
            collisionFound = true;
            break;
        }
#if ENABLE_LAYER_MANAGER_STATS
        ++g_sumVisitedLayerCount;
#endif
        bool insertPointFound = false;
        size_t nodeIndexWhereToInsert = all_ones;
        size_t childIndexWhereToInsert = all_ones;
        SG_ASSERT(reverseLayer < layerCount);
        size_t nodeIndex = m_layers[layerCount - 1 - reverseLayer];
        do {
#if ENABLE_LAYER_MANAGER_STATS
            ++g_sumVisitedBoxCount;
#endif
            SG_ASSERT(nodeIndex < m_nodes.size());
            Node const& node = m_nodes[nodeIndex];
            box2f const& nodeBox = node.box;
            SG_ASSERT(AllGreaterStrict(nodeBox.Delta(), float2(0)));
            size_t mainChildIndexToExplore = all_ones;
            if(iBox.max.y() <= nodeBox.min.y())
            {
                if(iBox.max.x() <= nodeBox.min.x())
                {
                    mainChildIndexToExplore = 0;
                    PushIFN(toExplore, node.children[4]);
                    PushIFN(toExplore, node.children[5]);
                }
                else if(iBox.min.x() >= nodeBox.max.x())
                {
                    mainChildIndexToExplore = 1;
                    PushIFN(toExplore, node.children[4]);
                    PushIFN(toExplore, node.children[6]);
                }
                else
                {
                    mainChildIndexToExplore = 4;
                    if(iBox.min.x() < nodeBox.min.x())
                    {
                        PushIFN(toExplore, node.children[0]);
                        PushIFN(toExplore, node.children[5]);
                    }
                    if(iBox.max.x() > nodeBox.max.x())
                    {
                        PushIFN(toExplore, node.children[1]);
                        PushIFN(toExplore, node.children[6]);
                    }
                }
            }
            else if(iBox.min.y() >= nodeBox.max.y())
            {
                if(iBox.max.x() <= nodeBox.min.x())
                {
                    mainChildIndexToExplore = 2;
                    PushIFN(toExplore, node.children[5]);
                    PushIFN(toExplore, node.children[7]);
                }
                else if(iBox.min.x() >= nodeBox.max.x())
                {
                    mainChildIndexToExplore = 3;
                    PushIFN(toExplore, node.children[6]);
                    PushIFN(toExplore, node.children[7]);
                }
                else
                {
                    mainChildIndexToExplore = 7;
                    if(iBox.min.x() < nodeBox.min.x())
                    {
                        PushIFN(toExplore, node.children[2]);
                        PushIFN(toExplore, node.children[5]);
                    }
                    if(iBox.max.x() > nodeBox.max.x())
                    {
                        PushIFN(toExplore, node.children[3]);
                        PushIFN(toExplore, node.children[6]);
                    }
                }
            }
            else if(iBox.max.x() <= nodeBox.min.x())
            {
                mainChildIndexToExplore = 5;
                if(iBox.min.y() < nodeBox.min.y())
                {
                    PushIFN(toExplore, node.children[0]);
                    PushIFN(toExplore, node.children[4]);
                }
                if(iBox.max.y() > nodeBox.max.y())
                {
                    PushIFN(toExplore, node.children[2]);
                    PushIFN(toExplore, node.children[7]);
                }
            }
            else if(iBox.min.x() >= nodeBox.max.x())
            {
                mainChildIndexToExplore = 6;
                if(iBox.min.y() < nodeBox.min.y())
                {
                    PushIFN(toExplore, node.children[1]);
                    PushIFN(toExplore, node.children[4]);
                }
                if(iBox.max.y() > nodeBox.max.y())
                {
                    PushIFN(toExplore, node.children[3]);
                    PushIFN(toExplore, node.children[7]);
                }
            }
            else
            {
                collisionFound = true;
                break;
            }

            size_t nextNodeIndex = node.children[mainChildIndexToExplore];

            if(all_ones == nextNodeIndex)
            {
                if(!insertPointFound)
                {
                    nodeIndexWhereToInsert = nodeIndex;
                    childIndexWhereToInsert = mainChildIndexToExplore;
                }
                if(!toExplore.empty())
                {
                    SG_ASSERT_MSG(toExplore.size() < toExplore.capacity() / 2, "If this assert fails, toExplore capacity should be increased");
                    insertPointFound = true;
                    nextNodeIndex = toExplore.back();
                    toExplore.pop_back();
                    SG_ASSERT(all_ones != nextNodeIndex);
                }
                else
                {
                    SG_ASSERT(all_ones != nodeIndexWhereToInsert);
                    SG_ASSERT(all_ones != childIndexWhereToInsert);
                    insertPoints.emplace_back(nodeIndexWhereToInsert, childIndexWhereToInsert);
                    ++reverseLayer;
                    break;
                }
            }

            nodeIndex = nextNodeIndex;

            SG_ASSERT(!collisionFound);
            SG_ASSERT(all_ones != nodeIndex);
        } while(true);

        //u8 mask = 0;
        //if(iBox.max.x() < node.box.min.x())         { mask |= 0x1; }
        //else if(iBox.min.x() > node.box.max.x())    { mask |= 0x2; }
        //if(iBox.max.y() < node.box.min.y())         { mask |= 0x4; }
        //else if(iBox.min.y() > node.box.max.y())    { mask |= 0x8; }

    } while(!collisionFound);

    size_t const layerBegin = layerCount - reverseLayer;
    size_t const layerEnd = layerBegin + iLayerCount;

    // insert point
    if(insertPoints.size() < iLayerCount)
    {
        // add layers
        m_layers.emplace_back(m_nodes.size());
        while(m_layers.size() < layerEnd)
        {
            // layer must point to a valid box, so any new empty layer is
            // assigned the input box. Not optimal, but this code is not
            // expected to be used.
            m_nodes.emplace_back(iBox);
            m_layers.emplace_back(m_nodes.size());
        }
    }
    else
    {
        // use insert point
        std::pair<size_t, size_t> insertPoint = insertPoints[insertPoints.size() - iLayerCount];
        SG_ASSERT(all_ones == m_nodes[insertPoint.first].children[insertPoint.second]);
        m_nodes[insertPoint.first].children[insertPoint.second] = m_nodes.size();
    }
    m_nodes.emplace_back(iBox);

    LayerRange ret(checked_numcastable(layerBegin), checked_numcastable(layerEnd));

    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_QuadTreeLike::Clear()
{
    //SG_LOG_DEBUG("UI", Format("LayerManager - %0 layers", m_layers.size()));
#if ENABLE_LAYER_MANAGER_STATS
    PrintLayerManagerStatsAndClear();
#endif
    m_nodes.clear();
    m_layers.clear();
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_RTREE_PER_LAYER || defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_RTreePerLayer::LayerManager_RTreePerLayer()
    : m_nodes()
    , m_layers()
#if SG_ENABLE_ASSERT
    , m_boxCount(0)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_RTreePerLayer::~LayerManager_RTreePerLayer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool LayerManager_RTreePerLayer::Find(box2f const& iBox, size_t iLayerIndex, Path& oInsertPoint)
{
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumVisitedLayerCount;
#endif
    SG_ASSERT(iLayerIndex < m_layers.size());
    size_t nodeIndex = m_layers[iLayerIndex];
    SG_ASSERT(all_ones != nodeIndex);
    float bestAreaIncrease = std::numeric_limits<float>::max(); // TODO: Why that global ?
    SG_ASSERT(oInsertPoint.empty());
    struct NodeToExplore
    {
        size_t index;
        MaxSizeVector<size_t, M> children;
        NodeToExplore(size_t iIndex) : index(iIndex) {}
    };
    MaxSizeVector<NodeToExplore, MAX_DEPTH> toExplore;
    SG_ASSERT(toExplore.empty());
    box2f const* pparentBox = nullptr;
    bool collisionFound = false;
    do {
        SG_ASSERT(all_ones != nodeIndex);
        SG_ASSERT(nodeIndex < m_nodes.size());
        Node const& node = m_nodes[nodeIndex];
        SG_ASSERT(!node.boxes.empty());
        if (all_ones == node.boxes[0].second)
        {
            // leaf
            for (auto const& it : node.boxes)
            {
#if ENABLE_LAYER_MANAGER_STATS
                ++g_sumVisitedBoxCount;
#endif
                box2f const& box = it.first;
                if (IntersectStrict(box, iBox))
                {
                    collisionFound = true;
                    break;
                }
            }
            if(!collisionFound)
            {
                if(nullptr == pparentBox)
                {
                    SG_ASSERT(oInsertPoint.empty());
                }
                else
                {
                    box2f const& parentBox = *pparentBox;
                    float const prevArea = parentBox.NVolume_AssumeConvex();
                    box2f mergeBox = parentBox;
                    mergeBox.Grow(iBox);
                    float const nextArea = mergeBox.NVolume_AssumeConvex();
#if 1
                    float const areaIncrease = nextArea;
#elif 1
                    float const areaIncrease = nextArea - prevArea;
#else
                    float const areaIncrease = nextArea / prevArea;
#endif
                    if(areaIncrease < bestAreaIncrease) // TODO: Why bestAreaIncrease is not modified ?
                    {
                        oInsertPoint.clear();
                        for(auto const& level : toExplore)
                            oInsertPoint.push_back(level.children.back());
                    }
                }

                if(!toExplore.empty())
                {
#if SG_ENABLE_ASSERT
                    size_t const parentNodeIndex = toExplore.back().index;
                    Node const& parentNode = m_nodes[parentNodeIndex];
                    size_t const child = toExplore.back().children.back();
                    SG_ASSERT(parentNode.boxes[child].second == nodeIndex);
#endif
                    toExplore.back().children.pop_back();
                }
            }
        }
        else
        {
            toExplore.emplace_back(nodeIndex);
            auto& childrenToExplore = toExplore.back().children;
            for (auto const& it : node.boxes)
            {
#if ENABLE_LAYER_MANAGER_STATS
                ++g_sumVisitedBoundingBoxCount;
#endif
                box2f const& box = it.first;
                if (IntersectStrict(box, iBox))
                {
                    size_t const childIndex = &it - node.boxes.data();
                    SG_ASSERT(all_ones != childIndex);
                    childrenToExplore.emplace_back(childIndex);
                }
            }
            // TODO: This can be done not here, but in Insert().
            if(childrenToExplore.empty() && oInsertPoint.empty())
            {
                float bestBBAreaIncrease = std::numeric_limits<float>::max();
                for (auto const& it : node.boxes)
                {
#if ENABLE_LAYER_MANAGER_STATS
                    ++g_sumVisitedBoundingBoxCount;
#endif
                    box2f const& box = it.first;
                    float const prevArea = box.NVolume_AssumeConvex();
                    box2f mergeBox = box;
                    mergeBox.Grow(iBox);
                    float const nextArea = mergeBox.NVolume_AssumeConvex();
#if 1
                    float const areaIncrease = nextArea;
#elif 1
                    float const areaIncrease = nextArea - prevArea;
#else
                    float const areaIncrease = nextArea / prevArea;
#endif
                    if(areaIncrease < bestBBAreaIncrease)
                    {
                        size_t const childIndex = &it - node.boxes.data();
                        SG_ASSERT(all_ones != childIndex);
                        childrenToExplore.emplace_back(childIndex);
                    }
                }
            }
            SG_ASSERT(!toExplore.empty());
        }

        if(!toExplore.empty())
        {
            SG_ASSERT(!toExplore.empty());
            while (toExplore.back().children.empty())
            {
                toExplore.pop_back();
                if (!toExplore.empty())
                    toExplore.back().children.pop_back();
                else
                    break;
                SG_ASSERT(!toExplore.empty());
            }
        }
        if (!toExplore.empty())
        {
            size_t const nodeIndexToExplore = toExplore.back().index;
            Node const& nodeToExplore = m_nodes[nodeIndexToExplore];
            size_t const childIndex = toExplore.back().children.back();
            SG_ASSERT(childIndex < nodeToExplore.boxes.size());
            pparentBox = &(nodeToExplore.boxes[childIndex].first);
            nodeIndex = nodeToExplore.boxes[childIndex].second;
            SG_ASSERT(all_ones != nodeIndex);
        }
        else
            break;
    } while (!collisionFound);
    return collisionFound;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTreePerLayer::SplitNode(size_t iNodeIndex, box2f const& iBox, size_t iChildNodeIndex, box2f& oOldNodeBox, box2f& oNewNodeBox, size_t& oNewNodeIndex)
{
    oNewNodeIndex = m_nodes.size();
    m_nodes.emplace_back();
    Node& node2 =  m_nodes.back();
    Node& node1 = m_nodes[iNodeIndex];

    MaxSizeVector<std::pair<box2f, size_t>, M+1 > boxes;
    {
        Node& oldNode = node1;
        SG_ASSERT(iNodeIndex < m_nodes.size());
        for(auto const& it : oldNode.boxes)
        {
            boxes.emplace_back(it);
        }
        boxes.emplace_back(iBox, iChildNodeIndex);
        oldNode.boxes.clear();
    }
#if 0 // dummy split
    for(size_t i = 0; i < (M+1)/2; ++i)
    {
        node1.boxes.emplace_back(boxes.back());
        boxes.pop_back();
    }
    while(!boxes.empty())
    {
        node2.boxes.emplace_back(boxes.back());
        boxes.pop_back();
    }
    oOldNodeBox = node1.boxes[0].first;
    for(size_t i = 1; i < node1.boxes.size(); ++i)
    {
        oOldNodeBox.Grow(node1.boxes[i].first);
    }
    oNewNodeBox = node2.boxes[0].first;
    for(size_t i = 1; i < node2.boxes.size(); ++i)
    {
        oNewNodeBox.Grow(node2.boxes[i].first);
    }
#elif 1
    // Own Linear Split
    // - Find 2 boxes with good seaparation
    //      compare distance between boxes with sum of boxes length
    // - For each box
    //      add to best node
    {
        box2f boundingBox;
        float2 sumDelta(0, 0);
        std::pair<size_t, float> bestMinX(all_ones, boundingBox.max.x());
        std::pair<size_t, float> bestMaxX(all_ones, boundingBox.min.x());
        std::pair<size_t, float> bestMinY(all_ones, boundingBox.max.y());
        std::pair<size_t, float> bestMaxY(all_ones, boundingBox.min.y());
        SG_ASSERT(M+1 == boxes.size());
        for(size_t i = 0; i < M+1; ++i)
        {
            box2f const& box = boxes[i].first;
            if(box.min.x() > bestMinX.second)
                bestMinX = std::make_pair(i, box.min.x());
            if(box.max.x() < bestMaxX.second)
                bestMaxX = std::make_pair(i, box.max.x());
            if(box.min.y() > bestMinY.second)
                bestMinY = std::make_pair(i, box.min.y());
            if(box.max.y() < bestMaxY.second)
                bestMaxY = std::make_pair(i, box.max.y());
            boundingBox.Grow(box);
            sumDelta += box.Delta();
        }
        // NB: multiply by sumDelta.y() instead of dividing by sumDelta.x().
        float const scoreX = (bestMinX.second - bestMaxX.second) * sumDelta.y();
        float const scoreY = (bestMinY.second - bestMaxY.second) * sumDelta.x();
        SG_ASSERT(scoreX > 0.f || scoreY > 0 || all_ones != boxes[0].second);
        if(scoreX >= scoreY)
        {
            SG_ASSERT(bestMinX.first < boxes.size());
            SG_ASSERT(bestMaxX.first < boxes.size());
            SG_ASSERT(bestMinX.first != bestMaxX.first);
            node1.boxes.emplace_back(boxes[bestMinX.first]);
            node2.boxes.emplace_back(boxes[bestMaxX.first]);
            using std::swap;
            size_t const boxCount = boxes.size();
            if(bestMinX.first < boxCount - 2)
                if(bestMaxX.first == boxCount - 1)
                    swap(boxes[bestMinX.first], boxes[boxCount - 2]);
                else
                    swap(boxes[bestMinX.first], boxes[boxCount - 1]);
            if(bestMaxX.first < boxCount - 2)
                if(bestMinX.first == boxCount - 2)
                    swap(boxes[bestMaxX.first], boxes[boxCount - 1]);
                else
                    swap(boxes[bestMaxX.first], boxes[boxCount - 2]);
            boxes.pop_back();
            boxes.pop_back();
        }
        else
        {
            SG_ASSERT(bestMinY.first < boxes.size());
            SG_ASSERT(bestMaxY.first < boxes.size());
            SG_ASSERT(bestMinY.first != bestMaxY.first);
            node1.boxes.emplace_back(boxes[bestMinY.first]);
            node2.boxes.emplace_back(boxes[bestMaxY.first]);
            using std::swap;
            size_t const boxCount = boxes.size();
            if(bestMinY.first < boxCount - 2)
                if(bestMaxY.first == boxCount - 1)
                    swap(boxes[bestMinY.first], boxes[boxCount - 2]);
                else
                    swap(boxes[bestMinY.first], boxes[boxCount - 1]);
            if(bestMaxY.first < boxCount - 2)
                if(bestMinY.first == boxCount - 2)
                    swap(boxes[bestMaxY.first], boxes[boxCount - 1]);
                else
                    swap(boxes[bestMaxY.first], boxes[boxCount - 2]);
            boxes.pop_back();
            boxes.pop_back();
        }
    }

    box2f bb1(node1.boxes[0].first);
    box2f bb2(node2.boxes[0].first);
    for(auto const& it : boxes)
    {
        box2f const& box = it.first;
        box2f growbb1 = bb1;
        growbb1.Grow(box);
        float const growbb1Volume = growbb1.NVolume_AssumeConvex();
        box2f growbb2 = bb2;
        growbb2.Grow(box);
        float const growbb2Volume = growbb2.NVolume_AssumeConvex();
        box2f const intersection1 = Intersection(growbb1, bb2);
        box2f const intersection2 = Intersection(growbb2, bb1);
        float const overlap1 = intersection1.NVolume_NegativeIfNonConvex();
        float const overlap2 = intersection2.NVolume_NegativeIfNonConvex();
        // ad-hoc score. Can be improved.
        float const score1 = (overlap1 > 0 ? -overlap1 : 1.f) * growbb2Volume * (M/2 + node2.boxes.size());
        float const score2 = (overlap2 > 0 ? -overlap2 : 1.f) * growbb1Volume * (M/2 + node1.boxes.size());
        if(score1 > score2)
        {
            node1.boxes.emplace_back(it);
            bb1 = growbb1;
        }
        else
        {
            node2.boxes.emplace_back(it);
            bb2 = growbb2;
        }
    }
    oOldNodeBox = bb1;
    oNewNodeBox = bb2;
    // TODO: check min number of boxes in Node.

#if SG_ENABLE_ASSERT
    if(all_ones == node1.boxes[0].second)
    {
        SG_ASSERT(!node1.boxes.empty());
        size_t const node1Count = node1.boxes.size();
        for(size_t i = 0; i < node1Count-1; ++i)
        {
            for(size_t j = i+1; j < node1Count; ++j)
            {
                SG_ASSERT(!IntersectStrict(node1.boxes[i].first, node1.boxes[j].first));
            }
        }
        SG_ASSERT(!node2.boxes.empty());
        size_t const node2Count = node2.boxes.size();
        for(size_t i = 0; i < node2Count-1; ++i)
        {
            for(size_t j = i+1; j < node2Count; ++j)
            {
                SG_ASSERT(!IntersectStrict(node2.boxes[i].first, node2.boxes[j].first));
            }
        }
        for(size_t i = 0; i < node1Count; ++i)
        {
            for(size_t j = 0; j < node2Count; ++j)
            {
                SG_ASSERT(!IntersectStrict(node1.boxes[i].first, node2.boxes[j].first));
            }
        }
    }
#endif
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTreePerLayer::Insert(box2f const& iBox, size_t iLayerIndex, Path const& iInsertPoint)
{
    Path absPath;
    {
        SG_ASSERT(iLayerIndex < m_layers.size());
        size_t nodeIndex = m_layers[iLayerIndex];
        SG_ASSERT(nodeIndex < m_nodes.size());
        absPath.push_back(nodeIndex);
        size_t const pathLength = iInsertPoint.size();
        for(size_t i = 0; i < pathLength; ++i)
        {
            SG_ASSERT(nodeIndex < m_nodes.size());
            Node const& node = m_nodes[nodeIndex];
            size_t const childIndex = iInsertPoint[i];
            SG_ASSERT(childIndex < node.boxes.size());
            nodeIndex = node.boxes[childIndex].second;
            SG_ASSERT(nodeIndex < m_nodes.size());
            absPath.push_back(nodeIndex);
        }
    }

    box2f boxToInsert = iBox;
    size_t indexToInsert = all_ones;

    do
    {
        size_t const nodeIndex = absPath.back();
        absPath.pop_back();
        Node& node = m_nodes[nodeIndex];
        if(node.boxes.size() == M)
        {
            box2f oldNodeBox(uninitialized);
            box2f newNodeBox(uninitialized);
            size_t newNodeIndex = all_ones;
            SplitNode(nodeIndex, boxToInsert, indexToInsert, oldNodeBox, newNodeBox, newNodeIndex);
            SG_ASSERT(all_ones != newNodeIndex);
            if(!absPath.empty())
            {
                Node& parentNode = m_nodes[absPath.back()];
                size_t const childrenIndex = iInsertPoint[absPath.size()-1];
                box2f& oldNodeBoundingBox = parentNode.boxes[childrenIndex].first;
                oldNodeBoundingBox = oldNodeBox;

                boxToInsert = newNodeBox;
                indexToInsert = newNodeIndex;
            }
            else
            {
                SG_ASSERT(m_layers[iLayerIndex] == nodeIndex);
                m_layers[iLayerIndex] = m_nodes.size();
                m_nodes.emplace_back();
                Node& newRootNode = m_nodes.back();
                newRootNode.boxes.emplace_back(oldNodeBox, nodeIndex);
                newRootNode.boxes.emplace_back(newNodeBox, newNodeIndex);
                break;
            }
        }
        else
        {
            SG_ASSERT(!node.boxes.empty());
            SG_ASSERT((all_ones == node.boxes[0].second) == (all_ones == indexToInsert));
            node.boxes.emplace_back(boxToInsert, indexToInsert);

            while(!absPath.empty())
            {
                indexToInsert = absPath.back();
                Node& parentNode = m_nodes[absPath.back()];
                absPath.pop_back();
                size_t const childrenIndex = iInsertPoint[absPath.size()];
                box2f& boundingBox = parentNode.boxes[childrenIndex].first;
                boundingBox.Grow(boxToInsert);
                boundingBox.Grow(iBox); // NB: iBox is not necessarilly in boxToInsert.
                boxToInsert = boundingBox;
            }
            break;
        }
    } while(true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerRange LayerManager_RTreePerLayer::Lock(box2f const& iBox, size_t iLayerCount)
{
#if SG_ENABLE_ASSERT
    ++m_boxCount;
#endif
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumInputBoxCount;
#endif
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    size_t const layerCount = m_layers.size();
    size_t reverseLayer = 0;
    typedef MaxSizeVector<size_t, MAX_DEPTH> Path;
    MaxSizeVector<Path, 256> insertPoints;
    SG_ASSERT_MSG(m_layers.size() < insertPoints.capacity() / 2, "If this assert fails, insertPoints capacity should be increased");
    bool collisionFound = false;
    do
    {
        SG_ASSERT(insertPoints.size() == reverseLayer);
        insertPoints.emplace_back();
        if (reverseLayer >= layerCount)
        {
            collisionFound = true;
            break;
        }
        SG_ASSERT(reverseLayer < layerCount);
        size_t const layerIndex = layerCount - 1 - reverseLayer;
        SG_ASSERT(insertPoints.back().empty());
        collisionFound = Find(iBox, layerIndex, insertPoints.back());
        if(collisionFound)
            break;
        ++reverseLayer;
    } while (true);

    insertPoints.pop_back();

    size_t const layerBegin = layerCount - reverseLayer;
    size_t const layerEnd = layerBegin + iLayerCount;

    if (insertPoints.size() < iLayerCount)
    {
        // add layers
        SG_ASSERT(m_layers.size() <= layerEnd);
        size_t const layerCountToAdd = iLayerCount - insertPoints.size();
        SG_ASSERT(layerEnd - m_layers.size() == layerCountToAdd);
        for(size_t i = 0; i < layerCountToAdd; ++i)
        {
            m_layers.emplace_back(m_nodes.size());
            m_nodes.emplace_back();
        }
        auto& node = m_nodes.back();
        node.boxes.emplace_back(iBox, all_ones);
        SG_ASSERT(m_layers.size() == layerEnd);
    }
    else
    {
        // use insert point
        SG_ASSERT(iLayerCount <= insertPoints.size());
        size_t const layerDepth = insertPoints.size() - iLayerCount;
        Path const& insertPoint = insertPoints[layerDepth];
        size_t const insertLayer = layerCount - 1 - layerDepth;
        SG_ASSERT(insertLayer < layerCount);
        Insert(iBox, insertLayer, insertPoint);
        SG_BREAKABLE_POS;
    }

    LayerRange ret(checked_numcastable(layerBegin), checked_numcastable(layerEnd));

    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTreePerLayer::Clear()
{
    //SG_LOG_DEBUG("UI", Format("LayerManager - %0 layers", m_layers.size()));
#if ENABLE_LAYER_MANAGER_STATS
    PrintLayerManagerStatsAndClear();
#endif
    m_nodes.clear();
    m_layers.clear();
#if SG_ENABLE_ASSERT
    ++m_boxCount = 0;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void LayerManager_RTreePerLayer::CheckIntegrity() const
{
    size_t totalBoxCount = 0;
    for(auto layer : m_layers)
    {
        std::vector<box2f> layerBoxes;
        struct NodeToExplore
        {
            size_t index;
            box2f box;
            NodeToExplore(size_t iIndex, box2f const& iBox) : index(iIndex), box(iBox) {}
        };
        std::vector<NodeToExplore> toExplore;
        toExplore.emplace_back(layer, box2f::FromMinMax(float2(-std::numeric_limits<float>::max()), float2(std::numeric_limits<float>::max())));
        while(!toExplore.empty())
        {
            NodeToExplore nodeToExplore = toExplore.back();
            toExplore.pop_back();
            SG_ASSERT(all_ones != nodeToExplore.index);
            SG_ASSERT(nodeToExplore.index < m_nodes.size());
            Node const& node = m_nodes[nodeToExplore.index];
            SG_ASSERT(!node.boxes.empty());
            if (all_ones == node.boxes[0].second)
            {
                // leaf
                for (auto const& it : node.boxes)
                {
                    SG_ASSERT(Intersection(it.first, nodeToExplore.box) == it.first);
                    layerBoxes.emplace_back(it.first);
                }
            }
            else
            {
                for (auto const& it : node.boxes)
                {
                    SG_ASSERT(Intersection(it.first, nodeToExplore.box) == it.first);
                    toExplore.emplace_back(it.second, it.first);
                }
            }
        }

        // TODO: check boxes do not overlap
        size_t const layerBoxCount = layerBoxes.size();
        for(size_t i = 0; i < layerBoxCount-1; ++i)
        {
            for(size_t j = i+1; j < layerBoxCount; ++j)
            {
                SG_ASSERT(!IntersectStrict(layerBoxes[i], layerBoxes[j]));
            }
        }
        totalBoxCount += layerBoxCount;
    }
    SG_ASSERT(totalBoxCount == m_boxCount);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_RTREE|| defined(ENABLE_LAYER_MANAGER_COMPARISON)
LayerManager_RTree::LayerManager_RTree()
    : m_nodes()
    , m_root(0)
#if SG_ENABLE_ASSERT
    , m_boxCount(0)
#endif
{
    m_nodes.emplace_back();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerManager_RTree::~LayerManager_RTree()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t LayerManager_RTree::FindReturnFreeLayer(box2f const& iBox, Path& oInsertPoint)
{
    SG_ASSERT(!m_nodes.empty());
    SG_ASSERT(oInsertPoint.empty());
    struct NodeToExplore
    {
        size_t index;
        bool inInsertPoint;
        NodeToExplore(size_t iIndex, bool iInInsertPoint): index(iIndex), inInsertPoint(iInInsertPoint) {}
    };
    MaxSizeVector<NodeToExplore, MAX_DEPTH * M> toExplore;
    SG_ASSERT(toExplore.empty());
    SG_ASSERT(all_ones != m_root);
    toExplore.emplace_back(m_root, true);
    size_t minLayerToInsert = 0;
    do {
        size_t const nodeIndex = toExplore.back().index;
        SG_ASSERT(all_ones != nodeIndex);
        SG_ASSERT(nodeIndex < m_nodes.size());
        Node const& node = m_nodes[nodeIndex];
        bool const inInsertPoint = toExplore.back().inInsertPoint;
        toExplore.pop_back();
        if(node.children.empty())
        {
            SG_ASSERT(nodeIndex == m_root);
            break;
        }
        else
        {
            SG_ASSERT(!node.children.empty());
            if(all_ones == node.children[0].index)
            {
                //leaf
                //for(auto const& it : node.children)
                // chances are that higher layers are at the end
                for(auto const& it : reverse_view(node.children))
                {
#if ENABLE_LAYER_MANAGER_STATS
                    ++g_sumVisitedPreBoxCount;
#endif
                    if(it.maxLayer >= minLayerToInsert)
                    {
#if ENABLE_LAYER_MANAGER_STATS
                        ++g_sumVisitedBoxCount;
#endif
                        box2f const& box = it.box;
                        if(IntersectStrict(box, iBox))
                        {
                            minLayerToInsert = it.maxLayer + 1;
                        }
                    }
                }
            }
            else
            {
                size_t const childCount = node.children.size();
                float bestAreaIncrease = std::numeric_limits<float>::max();
                size_t bestChildForInsertion = all_ones;
                size_t bestChildInExplore = all_ones;
                for(size_t i = 0; i < childCount; ++i)
                {
#if ENABLE_LAYER_MANAGER_STATS
                    ++g_sumVisitedBoundingBoxCount;
#endif
                    Branch const& branch = node.children[i];
                    box2f const& box = branch.box;
                    if(branch.maxLayer >= minLayerToInsert)
                    {
                        bool const intersect = IntersectStrict(box, iBox);
                        if(intersect)
                        {
                            toExplore.emplace_back(branch.index, false);
                        }
                    }
                    if(inInsertPoint)
                    {
                        float const prevArea = box.NVolume_AssumeConvex();
                        box2f mergeBox = box;
                        mergeBox.Grow(iBox);
                        float const nextArea = mergeBox.NVolume_AssumeConvex();
#if 0
                        float const areaIncrease = nextArea;
#elif 1
                        float const areaIncrease = 1.05f * nextArea - prevArea;
#elif 1
                        float const areaIncrease = nextArea - prevArea;
#else
                        float const areaIncrease = nextArea / prevArea;
#endif
                        if(areaIncrease < bestAreaIncrease)
                        {
                            bestAreaIncrease = areaIncrease;
                            bestChildForInsertion = i;
                            bestChildInExplore = !toExplore.empty() && toExplore.back().index == branch.index ? toExplore.size() - 1 : all_ones;
                        }
                    }
                }

                if(inInsertPoint)
                {
                    if(all_ones == bestChildInExplore)
                    {
                        SG_ASSERT(bestChildForInsertion < node.children.size());
                        Branch const& branch = node.children[bestChildForInsertion];
                        toExplore.emplace_back(branch.index, true);
                    }
                    else
                    {
                        SG_ASSERT(bestChildInExplore < toExplore.size());
                        toExplore[bestChildInExplore].inInsertPoint = true;
                    }
                    oInsertPoint.emplace_back(bestChildForInsertion);
                }
            }
        }
    } while (!toExplore.empty());

    return minLayerToInsert;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTree::BalanceNodes(Node& ioNodeLess, box2f& ioBBLess, Node& ioNodeMore, box2f& ioBBMore)
{
    SG_ASSERT(ioNodeLess.children.size() < m);
    SG_ASSERT(ioNodeMore.children.size() > M-m);
    SG_ASSERT(ioNodeLess.children.size() + ioNodeMore.children.size() > M);
    do
    {
        std::pair<size_t, float> best(all_ones, std::numeric_limits<float>::infinity());
        for_range(size_t, i, 0, ioNodeMore.children.size())
        {
            box2f const& box = ioNodeMore.children[i].box;
            box2f growbb = ioBBLess;
            growbb.Grow(box);
            float const growbbVolume = growbb.NVolume_AssumeConvex();
            if(growbbVolume < best.second)
                best = std::make_pair(i, growbbVolume);
        }
        SG_ASSERT(all_ones != best.first);
        ioNodeLess.children.emplace_back(ioNodeMore.children[best.first]);
        box2f const& bestBox = ioNodeMore.children[best.first].box;
        ioBBLess.Grow(bestBox);
        using std::swap;
        if(best.first + 1 != ioNodeMore.children.size())
            ioNodeMore.children[best.first] = ioNodeMore.children.back();
        ioNodeMore.children.pop_back();
    } while(ioNodeLess.children.size() < m);

    ioBBMore = ioNodeMore.children[0].box;
    for_range(size_t, i, 1, ioNodeMore.children.size())
    {
        box2f const& box = ioNodeMore.children[i].box;
        ioBBMore.Grow(box);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTree::SplitNode(size_t iNodeIndex, BoxAndMaxLayer const& iBoxAndLayer, size_t iChildNodeIndex, BoxAndMaxLayer& oOldNodeBoxAndLayer, BoxAndMaxLayer& oNewNodeBoxAndLayer, size_t& oNewNodeIndex)
{
#if SG_ENABLE_ASSERT
//#define LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG
#endif
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
    std::ostringstream dbgoss;
    //dbgoss << "___________________ LEGEND ___________________" << std::endl;
    //dbgoss << "B: node assigned to other parent for balancing" << std::endl;
    //dbgoss << "______________________________________________" << std::endl;
    size_t const dbgImgSize = 800;
    float oodbgImgSize = 1.f/dbgImgSize;
    image::RGBImage dbgSplitNode(uint2(dbgImgSize), ubyte3(0));
    auto brush = image::brush::Blend<image::blend::Classic>();
    box2f rootBoundingBox = iBoxAndLayer.box;
    for(auto const& c : m_nodes[m_root].children)
        rootBoundingBox.Grow(c.box);
    float2 const bbDelta = rootBoundingBox.Delta();
    float const bbDeltaMax = std::max(bbDelta.x(), bbDelta.y());
    float const scale = (dbgImgSize - 10.f) / bbDeltaMax;
    float2 const offset = - rootBoundingBox.Center() * scale + 0.5f * float2(float(dbgImgSize));
#endif
    oNewNodeIndex = m_nodes.size();
    m_nodes.emplace_back();
    Node& node2 = m_nodes.back();
    Node& node1 = m_nodes[iNodeIndex];

    MaxSizeVector<Branch, M+1 > children;
    {
        Node& oldNode = node1;
        SG_ASSERT(iNodeIndex < m_nodes.size());
        for(auto const& it : oldNode.children)
        {
            children.emplace_back(it);
        }
        children.emplace_back(iBoxAndLayer, iChildNodeIndex);
        oldNode.children.clear();
    }
#if 0 // dummy split
    for(size_t i = 0; i < (M+1)/2; ++i)
    {
        node1.children.emplace_back(children.back());
        children.pop_back();
    }
    while(!children.empty())
    {
        node2.children.emplace_back(children.back());
        children.pop_back();
    }
    oOldNodeBoxAndLayer = node1.children[0];
    for(size_t i = 1; i < node1.children.size(); ++i)
    {
        oOldNodeBoxAndLayer.box.Grow(node1.children[i].box);
        oOldNodeBoxAndLayer.maxLayer = std::max(oOldNodeBoxAndLayer.maxLayer, node1.children[i].maxLayer);
    }
    oNewNodeBoxAndLayer = node2.children[0];
    for(size_t i = 1; i < node2.children.size(); ++i)
    {
        oNewNodeBoxAndLayer.box.Grow(node2.children[i].box);
        oNewNodeBoxAndLayer.maxLayer = std::max(oNewNodeBoxAndLayer.maxLayer, node2.children[i].maxLayer);
    }
#elif 1
    // Own Linear Split
    // - Find 2 boxes with good seaparation
    //      compare distance between boxes with sum of boxes length
    // - For each box
    //      add to best node
    box2f boundingBox;
    {
        float2 sumDelta(0, 0);
        std::pair<size_t, float> bestMinX(all_ones, boundingBox.max.x());
        std::pair<size_t, float> bestMaxX(all_ones, boundingBox.min.x());
        std::pair<size_t, float> bestMinY(all_ones, boundingBox.max.y());
        std::pair<size_t, float> bestMaxY(all_ones, boundingBox.min.y());
        SG_ASSERT(M+1 == children.size());
        for(size_t i = 0; i < M+1; ++i)
        {
            box2f const& box = children[i].box;
            if(box.min.x() > bestMinX.second)
                bestMinX = std::make_pair(i, box.min.x());
            if(box.max.x() < bestMaxX.second)
                bestMaxX = std::make_pair(i, box.max.x());
            if(box.min.y() > bestMinY.second)
                bestMinY = std::make_pair(i, box.min.y());
            if(box.max.y() < bestMaxY.second)
                bestMaxY = std::make_pair(i, box.max.y());
            boundingBox.Grow(box);
            sumDelta += box.Delta();
        }
        // NB: multiply by sumDelta.y() instead of dividing by sumDelta.x().
        float const scoreX = (bestMinX.second - bestMaxX.second) * sumDelta.y();
        float const scoreY = (bestMinY.second - bestMaxY.second) * sumDelta.x();
        //SG_ASSERT(scoreX > 0.f || scoreY > 0 || all_ones != children[0].index);
        if(scoreX >= scoreY)
        {
            SG_ASSERT(bestMinX.first < children.size());
            SG_ASSERT(bestMaxX.first < children.size());
            SG_ASSERT(bestMinX.first != bestMaxX.first || (bestMinY.first == bestMaxY.first && 0 == bestMinX.first));
            if(bestMinX.first == bestMaxX.first) { bestMaxX.first = 1; }
            SG_ASSERT(bestMinX.first != bestMaxX.first);
            node1.children.emplace_back(children[bestMinX.first]);
            node2.children.emplace_back(children[bestMaxX.first]);
            using std::swap;
            size_t const boxCount = children.size();
            if(bestMinX.first < boxCount - 2)
                if(bestMaxX.first == boxCount - 1)
                    swap(children[bestMinX.first], children[boxCount - 2]);
                else
                    swap(children[bestMinX.first], children[boxCount - 1]);
            if(bestMaxX.first < boxCount - 2)
                if(bestMinX.first == boxCount - 2)
                    swap(children[bestMaxX.first], children[boxCount - 1]);
                else
                    swap(children[bestMaxX.first], children[boxCount - 2]);
            children.pop_back();
            children.pop_back();
        }
        else
        {
            SG_ASSERT(bestMinY.first < children.size());
            SG_ASSERT(bestMaxY.first < children.size());
            SG_ASSERT(bestMinY.first != bestMaxY.first);
            node1.children.emplace_back(children[bestMinY.first]);
            node2.children.emplace_back(children[bestMaxY.first]);
            using std::swap;
            size_t const boxCount = children.size();
            if(bestMinY.first < boxCount - 2)
                if(bestMaxY.first == boxCount - 1)
                    swap(children[bestMinY.first], children[boxCount - 2]);
                else
                    swap(children[bestMinY.first], children[boxCount - 1]);
            if(bestMaxY.first < boxCount - 2)
                if(bestMinY.first == boxCount - 2)
                    swap(children[bestMaxY.first], children[boxCount - 1]);
                else
                    swap(children[bestMaxY.first], children[boxCount - 2]);
            children.pop_back();
            children.pop_back();
        }
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
        dbgoss << "scoreX: " << scoreX << std::endl << "scoreY: " << scoreY << std::endl;
#endif
    }

    box2f bb1(node1.children[0].box);
    box2f bb2(node2.children[0].box);
    for(auto const& it : children)
    {
        box2f const& box = it.box;
        box2f growbb1 = bb1;
        growbb1.Grow(box);
        float const growbb1Volume = growbb1.NVolume_AssumeConvex();
        box2f growbb2 = bb2;
        growbb2.Grow(box);
        float const growbb2Volume = growbb2.NVolume_AssumeConvex();
        box2f const intersection1 = Intersection(growbb1, bb2);
        box2f const intersection2 = Intersection(growbb2, bb1);
        float const overlap1 = intersection1.NVolume_NegativeIfNonConvex();
        float const overlap2 = intersection2.NVolume_NegativeIfNonConvex();
        // ad-hoc score. Can be improved.
#if 0
        float const score1 = (overlap1 > 0 ? -overlap1 : 1.f) * growbb2Volume * (M/2 + node2.children.size());
        float const score2 = (overlap2 > 0 ? -overlap2 : 1.f) * growbb1Volume * (M/2 + node1.children.size());
#elif 1
        float const score1 = (overlap1 > 0 ? -overlap1 : 1.f) * growbb2Volume;
        float const score2 = (overlap2 > 0 ? -overlap2 : 1.f) * growbb1Volume;
#else
        float const score1 = growbb2Volume;
        float const score2 = growbb1Volume;
#endif
        if(score1 > score2)
        {
            node1.children.emplace_back(it);
            bb1 = growbb1;
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
            image::DrawText(dbgSplitNode, node1.children.back().box.min * scale + offset + float2(1,8), Format("%0", &it - children.data()), brush.Fill(ubyte4(255, 255, 0, 128)));
#endif
        }
        else
        {
            node2.children.emplace_back(it);
            bb2 = growbb2;
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
            image::DrawText(dbgSplitNode, node2.children.back().box.min * scale + offset + float2(1,8), Format("%0", &it - children.data()), brush.Fill(ubyte4(0, 255, 255, 128)));
#endif
        }
    }

    if(node1.children.size() < m)
    {
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
        size_t const prevSize = node1.children.size();
        image::DrawRect(dbgSplitNode, bb1 * scale + offset, brush.Stroke(ubyte4(255, 255, 0, 64), 1));
        image::DrawRect(dbgSplitNode, bb2 * scale + offset, brush.Stroke(ubyte4(0, 255, 255, 64), 1));
#endif
        BalanceNodes(node1, bb1, node2, bb2);
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
        for_range(size_t, i, prevSize, node1.children.size())
            image::DrawText(dbgSplitNode, node1.children[i].box.min * scale + offset + float2(1,8+10), Format("B%0", (i-prevSize)), brush.Fill(ubyte3(255, 255, 255)));
#endif
    }
    else if(node2.children.size() < m)
    {
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
        size_t const prevSize = node2.children.size();
        image::DrawRect(dbgSplitNode, bb1 * scale + offset, brush.Stroke(ubyte4(255, 255, 0, 64), 1));
        image::DrawRect(dbgSplitNode, bb2 * scale + offset, brush.Stroke(ubyte4(0, 255, 255, 64), 1));
#endif
        BalanceNodes(node2, bb2, node1, bb1);
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
        for_range(size_t, i, prevSize, node2.children.size())
            image::DrawText(dbgSplitNode, node2.children[i].box.min * scale + offset + float2(1,8+10), Format("B%0", (i-prevSize)), brush.Fill(ubyte3(255, 255, 255)));
#endif
    }

    oOldNodeBoxAndLayer.box = bb1;
    oNewNodeBoxAndLayer.box = bb2;

    // TODO: check min number of boxes in Node.


    oOldNodeBoxAndLayer.maxLayer =  0;
    for(auto const& it : node1.children)
    {
        oOldNodeBoxAndLayer.maxLayer = std::max(oOldNodeBoxAndLayer.maxLayer, it.maxLayer);
    }
    oNewNodeBoxAndLayer.maxLayer =  0;
    for(auto const& it : node2.children)
    {
        oNewNodeBoxAndLayer.maxLayer = std::max(oNewNodeBoxAndLayer.maxLayer, it.maxLayer);
    }

#if SG_ENABLE_ASSERT
    size_t const node1Count = node1.children.size();
    for(size_t i = 0; i < node1Count; ++i)
    {
        SG_ASSERT(node1.children[i].maxLayer <= oOldNodeBoxAndLayer.maxLayer);
    }
    size_t const node2Count = node2.children.size();
    for(size_t i = 0; i < node2Count; ++i)
    {
        SG_ASSERT(node2.children[i].maxLayer <= oNewNodeBoxAndLayer.maxLayer);
    }
    if(all_ones == node1.children[0].index)
    {
        for(size_t i = 0; i < node1Count-1; ++i)
        {
            for(size_t j = i+1; j < node1Count; ++j)
            {
                SG_ASSERT(node1.children[i].maxLayer != node1.children[j].maxLayer || !IntersectStrict(node1.children[i].box, node1.children[j].box));
            }
        }
        for(size_t i = 0; i < node2Count-1; ++i)
        {
            for(size_t j = i+1; j < node2Count; ++j)
            {
                SG_ASSERT(node2.children[i].maxLayer != node2.children[j].maxLayer || !IntersectStrict(node2.children[i].box, node2.children[j].box));
            }
        }
        for(size_t i = 0; i < node1Count; ++i)
        {
            for(size_t j = 0; j < node2Count; ++j)
            {
                SG_ASSERT(node1.children[i].maxLayer != node2.children[j].maxLayer || !IntersectStrict(node1.children[i].box, node2.children[j].box));
            }
        }
    }
#endif
#if SG_ENABLE_ASSERT && defined(LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG)
    box2f const saledbb = boundingBox * scale + offset;
    image::DrawRect(dbgSplitNode, saledbb, brush.Stroke(ubyte4(128, 128, 128, 128), 2));
    for_range(size_t, i, 0, node1Count)
    {
        box2f const b = node1.children[i].box * scale + offset;
        if(i==0)
            image::DrawRect(dbgSplitNode, b, brush.Fill(ubyte4(255, 255, 0, 8)).Stroke(ubyte4(255, 255, 255, 128)));
        else
            image::DrawRect(dbgSplitNode, b, brush.Fill(ubyte4(255, 255, 0, 8)));
    }
    for_range(size_t, i, 0, node2Count)
    {
        box2f const b = node2.children[i].box * scale + offset;
        if(i==0)
            image::DrawRect(dbgSplitNode, b, brush.Fill(ubyte4(0, 255, 255, 8)).Stroke(ubyte4(255, 255, 255, 128)));
        else
            image::DrawRect(dbgSplitNode, b, brush.Fill(ubyte4(0, 255, 255, 8)));
    }
    image::DrawRect(dbgSplitNode, iBoxAndLayer.box * scale + offset, brush.Stroke(ubyte4(0, 255, 0, 128), 1));
    box2f const saledbb1 = bb1 * scale + offset;
    image::DrawRect(dbgSplitNode, saledbb1, brush.Stroke(ubyte4(255, 255, 0, 128), 2));
    box2f const saledbb2 = bb2 * scale + offset;
    image::DrawRect(dbgSplitNode, saledbb2, brush.Stroke(ubyte4(0, 255, 255, 128), 2));
    image::DrawText(dbgSplitNode, uint2(5,10), dbgoss.str(), brush.Fill(ubyte3(255, 255, 255)));

    SG_DEBUG_IMAGE_STEP_INTO(dbgSplitNode, true);
#endif

#endif
}
#undef LAYER_MANAGER_SPLIT_NODE_VISUAL_DEBUG
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTree::Insert(BoxAndMaxLayer const& iBoxAndLayer, Path const& iInsertPoint)
{
    Path absPath;
    {
        size_t nodeIndex = m_root;
        SG_ASSERT(nodeIndex < m_nodes.size());
        absPath.push_back(nodeIndex);
        size_t const pathLength = iInsertPoint.size();
        for(size_t i = 0; i < pathLength; ++i)
        {
            SG_ASSERT(nodeIndex < m_nodes.size());
            Node const& node = m_nodes[nodeIndex];
            size_t const childIndex = iInsertPoint[i];
            SG_ASSERT(childIndex < node.children.size());
            nodeIndex = node.children[childIndex].index;
            SG_ASSERT(nodeIndex < m_nodes.size());
            absPath.push_back(nodeIndex);
        }
    }

    BoxAndMaxLayer boxToInsert = iBoxAndLayer;
    size_t indexToInsert = all_ones;

    do
    {
        size_t const nodeIndex = absPath.back();
        absPath.pop_back();
        Node& node = m_nodes[nodeIndex];
        if(node.children.size() == M)
        {
            BoxAndMaxLayer oldNodeBox(uninitialized);
            BoxAndMaxLayer newNodeBox(uninitialized);
            size_t newNodeIndex = all_ones;
            SplitNode(nodeIndex, boxToInsert, indexToInsert, oldNodeBox, newNodeBox, newNodeIndex);
            SG_ASSERT(all_ones != newNodeIndex);
            if(!absPath.empty())
            {
                Node& parentNode = m_nodes[absPath.back()];
                size_t const childrenIndex = iInsertPoint[absPath.size()-1];
                BoxAndMaxLayer& oldNodeBoundingBox = parentNode.children[childrenIndex];
                oldNodeBoundingBox = oldNodeBox;

                boxToInsert = newNodeBox;
                indexToInsert = newNodeIndex;
            }
            else
            {
                SG_ASSERT(m_root == nodeIndex);
                m_root = m_nodes.size();
                m_nodes.emplace_back();
                Node& newRootNode = m_nodes.back();
                newRootNode.children.emplace_back(oldNodeBox, nodeIndex);
                newRootNode.children.emplace_back(newNodeBox, newNodeIndex);
                break;
            }
        }
        else
        {
            SG_ASSERT(node.children.empty() || (all_ones == node.children[0].index) == (all_ones == indexToInsert));
            node.children.emplace_back(boxToInsert, indexToInsert);

            while(!absPath.empty())
            {
                indexToInsert = absPath.back();
                Node& parentNode = m_nodes[absPath.back()];
                absPath.pop_back();
                size_t const childrenIndex = iInsertPoint[absPath.size()];
                BoxAndMaxLayer& boundingBoxAndMaxLayer = parentNode.children[childrenIndex];
                boundingBoxAndMaxLayer.box.Grow(boxToInsert.box);
                boundingBoxAndMaxLayer.box.Grow(iBoxAndLayer.box); // NB: iBox is not necessarilly in boxToInsert.
                boundingBoxAndMaxLayer.maxLayer = std::max(boundingBoxAndMaxLayer.maxLayer, boxToInsert.maxLayer);
                boundingBoxAndMaxLayer.maxLayer = std::max(boundingBoxAndMaxLayer.maxLayer, iBoxAndLayer.maxLayer);
                boxToInsert = boundingBoxAndMaxLayer;
            }
            break;
        }
    } while(true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LayerRange LayerManager_RTree::Lock(box2f const& iBox, size_t iLayerCount)
{
    SG_ASSERT(0 < iLayerCount);
#if SG_ENABLE_ASSERT
    ++m_boxCount;
#endif
#if ENABLE_LAYER_MANAGER_STATS
    ++g_sumInputBoxCount;
#endif
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    typedef MaxSizeVector<size_t, MAX_DEPTH> Path;
    Path insertPoint;
    size_t const layerBegin = FindReturnFreeLayer(iBox, insertPoint);
    size_t const layerEnd = layerBegin + iLayerCount;
    Insert(BoxAndMaxLayer(iBox, layerEnd-1), insertPoint);

    LayerRange ret(checked_numcastable(layerBegin), checked_numcastable(layerEnd));

    return ret;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void LayerManager_RTree::Clear()
{
    //SG_LOG_DEBUG("UI", Format("LayerManager - %0 layers", m_layers.size()));
#if ENABLE_LAYER_MANAGER_STATS
    PrintLayerManagerStatsAndClear();
#endif
    m_nodes.clear();
    m_nodes.emplace_back();
    m_root = 0;
#if SG_ENABLE_ASSERT
    ++m_boxCount = 0;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void LayerManager_RTree::CheckIntegrity() const
{
    size_t totalBoxCount = 0;
    std::vector<std::vector<box2f> > BoxesPerLayers;
    std::vector<Branch> toExplore;
    toExplore.emplace_back(
        box2f::FromMinMax(float2(-std::numeric_limits<float>::max()), float2(std::numeric_limits<float>::max())),
        std::numeric_limits<size_t>::max(),
        m_root);
    while(!toExplore.empty())
    {
        Branch nodeToExplore = toExplore.back();
        toExplore.pop_back();
        SG_ASSERT(all_ones != nodeToExplore.index);
        SG_ASSERT(nodeToExplore.index < m_nodes.size());
        Node const& node = m_nodes[nodeToExplore.index];
        SG_ASSERT(!node.children.empty() || 0 == m_boxCount);
        if(!node.children.empty() && all_ones == node.children[0].index)
        {
            // leaf
            for(auto const& it : node.children)
            {
                SG_ASSERT(it.maxLayer <= nodeToExplore.maxLayer);
                SG_ASSERT(Intersection(it.box, nodeToExplore.box) == it.box);
                while(it.maxLayer >= BoxesPerLayers.size())
                    BoxesPerLayers.emplace_back();
                SG_ASSERT(it.maxLayer < BoxesPerLayers.size());
                BoxesPerLayers[it.maxLayer].emplace_back(it.box);
            }
        }
        else
        {
            for(auto const& it : node.children)
            {
                SG_ASSERT(it.maxLayer <= nodeToExplore.maxLayer);
                SG_ASSERT(Intersection(it.box, nodeToExplore.box) == it.box);
                toExplore.emplace_back(it);
            }
        }
    }

    // TODO: check boxes do not overlap
    for(auto const& layerBoxes : BoxesPerLayers)
    {
        size_t const layerBoxCount = layerBoxes.size();
        for(size_t i = 0; i < layerBoxCount-1; ++i)
        {
            for(size_t j = i+1; j < layerBoxCount; ++j)
            {
                SG_ASSERT(!IntersectStrict(layerBoxes[i], layerBoxes[j]));
            }
        }
        totalBoxCount += layerBoxCount;
    }
    SG_ASSERT(totalBoxCount == m_boxCount);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#endif
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
template <typename T>
void CheckMultiLayerLock()
{
    T layerManager;
    size_t const N = 20;
    float const size = 10;
    box2f const box = box2f::FromMinDelta(float2(0), float2(size));
    size_t expectedLayer = 0;
    for(size_t i = 0; i < N; ++i)
    {
        size_t const layerCount = (i & 0x3) + 1;
        LayerRange const range = layerManager.Lock(box, layerCount);
        SG_ASSERT(range.size() == layerCount);
        SG_ASSERT(range[0] == expectedLayer);
        expectedLayer += layerCount;
    }
}
template <typename T, typename R>
void TestOnGrid()
{
    T layerManager;
    R layerManagerRef;
    size_t const N = 25;
    float const size = 10;
    float const border = 5;
    float2 const delta(size, size);
    for(size_t i = 0; i < N; ++i)
    {
        for(size_t j = 0; j < N; ++j)
        {
            size_t const layerCount = 1;
            float2 const p = float2(float(i),float(j))*(size+border)+border;
            box2f const b = box2f::FromMinDelta(p, delta);
            LayerRange const range = layerManager.Lock(b, layerCount);
            LayerRange const rangeRef = layerManagerRef.Lock(b, layerCount);
            SG_ASSERT(range.size() == rangeRef.size());
            SG_ASSERT(range[0] == rangeRef[0]);
        }
    }
#if SG_ENABLE_ASSERT
    layerManager.CheckIntegrity();
#endif
}
template <typename T, typename R>
void TestOnStack()
{
    T layerManager;
    R layerManagerRef;
    size_t const N = 200;
    float const size = 10;
    float2 const delta(size, size);
    for(size_t i = 0; i < N; ++i)
    {
        size_t const layerCount = 1;
        float2 const p = float2(0);
        box2f const b = box2f::FromMinDelta(p, delta);
        LayerRange const range = layerManager.Lock(b, layerCount);
        LayerRange const rangeRef = layerManagerRef.Lock(b, layerCount);
#if SG_ENABLE_ASSERT
        layerManager.CheckIntegrity();
#endif
        SG_ASSERT(range.size() == rangeRef.size());
        SG_ASSERT(range[0] == rangeRef[0]);
    }
#if SG_ENABLE_ASSERT
    layerManager.CheckIntegrity();
#endif
}
template <typename T, typename R>
void TestOnRandom()
{
    T layerManager;
    R layerManagerRef;
    size_t const N = 3000;
    float const size = 500;
    float const minSize = 2;
    float const maxSize = 20;
    size_t const maxLayerCount = 1;
    box2f const box = box2f::FromMinDelta(float2(0,0), float2(size, size));
    unsigned int const seed(0xCAFEFADE);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> randx(roundi(box.min.x()), roundi(box.max.x()));
    std::uniform_int_distribution<int> randy(roundi(box.min.y()), roundi(box.max.y()));
    std::uniform_int_distribution<size_t> randLayerCount(1, maxLayerCount);
    for(size_t i = 0; i < 3000; ++i)
    {
        size_t const layerCount = randLayerCount(gen);
        float2 p0(float(randx(gen)), float(randy(gen)));
        float2 p1(float(randx(gen)), float(randy(gen)));
        float2 p = 0.5f * (p0 + p1);
        using namespace componentwise;
        float2 delta = max(min(abs(p0-p1), float2(maxSize, maxSize)), float2(minSize, minSize));
        box2f b = box2f::FromCenterDelta(p, delta);
        LayerRange const range = layerManager.Lock(b, layerCount);
        LayerRange const rangeRef = layerManagerRef.Lock(b, layerCount);
#if SG_ENABLE_ASSERT
        if(0 == i%100)
            layerManager.CheckIntegrity();
#endif
        SG_ASSERT(range.size() == rangeRef.size());
        SG_ASSERT(range[0] == rangeRef[0]);
    }
#if SG_ENABLE_ASSERT
    layerManager.CheckIntegrity();
#endif
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg, ui), LayerManager, (UserInterface, slow))
{
    {
        CheckMultiLayerLock<LayerManager_OneLayerPerRequest>();
        CheckMultiLayerLock<LayerManager_AllBoxes>();
        CheckMultiLayerLock<LayerManager_OneBoxPerLayer>();
        CheckMultiLayerLock<LayerManager_NBoxesPerLayer>();
        CheckMultiLayerLock<LayerManager_QuadTreeLike>();
        CheckMultiLayerLock<LayerManager_RTreePerLayer>();
        CheckMultiLayerLock<LayerManager_RTree>();
        CheckMultiLayerLock<LayerManager>();

        TestOnGrid<LayerManager_QuadTreeLike, LayerManager_AllBoxes>();
        TestOnGrid<LayerManager_RTreePerLayer, LayerManager_AllBoxes>();
        TestOnGrid<LayerManager_RTree, LayerManager_AllBoxes>();

        TestOnStack<LayerManager_RTree, LayerManager_AllBoxes>();

        TestOnRandom<LayerManager_QuadTreeLike, LayerManager_AllBoxes>();
        TestOnRandom<LayerManager_RTreePerLayer, LayerManager_AllBoxes>();
        TestOnRandom<LayerManager_RTree, LayerManager_AllBoxes>();
    }
}
#endif
//=============================================================================
}
}

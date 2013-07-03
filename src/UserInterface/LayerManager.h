#ifndef UserInterface_LayerManager_H
#define UserInterface_LayerManager_H

#include <Core/Config.h>
#include <Core/IntTypes.h>
#include <Core/MaxSizeVector.h>
#include <Core/SmartPtr.h>
#include <Math/Box.h>
#include <array>
#include <vector>


#define UI_LAYER_MANAGER_ONE_LAYER_PER_REQUEST 0    // one layer per request (not optimal number of layer).
#define UI_LAYER_MANAGER_ALL_BOXES 1                // all boxes.
#define UI_LAYER_MANAGER_ONE_BOX_PER_LAYER 2        // one box per layer (not optimal number of layer).
#define UI_LAYER_MANAGER_N_BOXES_PER_LAYER 3        // N boxes per layer (not optimal number of layer, but better than one box).
#define UI_LAYER_MANAGER_QUAD_TREE_LIKE 4           // quadtree-like structure with no balancing using input boxes to split space.
#define UI_LAYER_MANAGER_RTREE_PER_LAYER 5          // R-Tree-like structure per layer
#define UI_LAYER_MANAGER_RTREE 6                    // R-Tree-like structure

//#define UI_LAYER_MANAGER_VERSION UI_LAYER_MANAGER_ONE_BOX_PER_LAYER
#define UI_LAYER_MANAGER_VERSION UI_LAYER_MANAGER_N_BOXES_PER_LAYER
//#define UI_LAYER_MANAGER_VERSION UI_LAYER_MANAGER_ONE_LAYER_PER_REQUEST
//#define UI_LAYER_MANAGER_VERSION UI_LAYER_MANAGER_ALL_BOXES
//#define UI_LAYER_MANAGER_VERSION UI_LAYER_MANAGER_RTREE_PER_LAYER
//#define UI_LAYER_MANAGER_VERSION UI_LAYER_MANAGER_RTREE

#if SG_ENABLE_UNIT_TESTS
#define ENABLE_LAYER_MANAGER_COMPARISON
#endif

namespace sg {
namespace ui {
//=============================================================================
class LayerRange
{
public:
    size_t size() const { return m_end - m_begin; }
    size_t operator[] (size_t i) const { size_t r = m_begin + i; SG_ASSERT(r < m_end); return r; }
private:
    friend class LayerManager_OneLayerPerRequest;
    friend class LayerManager_AllBoxes;
    friend class LayerManager_OneBoxPerLayer;
    friend class LayerManager_NBoxesPerLayer;
    friend class LayerManager_QuadTreeLike;
    friend class LayerManager_RTreePerLayer;
    friend class LayerManager_RTree;
    LayerRange(u16 iBegin, u16 iEnd) : m_begin(iBegin), m_end(iEnd) {}
private:
    u16 m_begin;
    u16 m_end;
};
//=============================================================================
#if (UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ONE_LAYER_PER_REQUEST) || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_OneLayerPerRequest : public SafeCountable
{
public:
    LayerManager_OneLayerPerRequest();
    ~LayerManager_OneLayerPerRequest();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
private:
    size_t m_index;
};
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ALL_BOXES || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_AllBoxes : public SafeCountable
{
public:
    LayerManager_AllBoxes();
    ~LayerManager_AllBoxes();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
private:
    std::vector<std::vector<box2f> > m_layers;
};
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ONE_BOX_PER_LAYER || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_OneBoxPerLayer : public SafeCountable
{
public:
    LayerManager_OneBoxPerLayer();
    ~LayerManager_OneBoxPerLayer();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
private:
    std::vector<box2f> m_layers;
};
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_N_BOXES_PER_LAYER || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_NBoxesPerLayer : public SafeCountable
{
public:
    LayerManager_NBoxesPerLayer();
    ~LayerManager_NBoxesPerLayer();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
private:
    static size_t const N = 3;
    class BoxVector : public MaxSizeVector<box2f, N>
    {
    public:
        BoxVector() {}
        BoxVector(BoxVector const& iOther) { for(auto const& it : iOther) emplace_back(it); }
        BoxVector operator= (BoxVector const& iOther) { for(auto const& it : iOther) emplace_back(it); }
    };
    std::vector<BoxVector> m_layers;
};
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_QUAD_TREE_LIKE || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_QuadTreeLike : public SafeCountable
{
public:
    LayerManager_QuadTreeLike();
    ~LayerManager_QuadTreeLike();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
#if SG_ENABLE_ASSERT
    void CheckIntegrity() const { /* TODO? */ }
#endif
private:
    struct Node
    {
        box2f box;
        size_t children[8];

        Node(box2f const& iBox)
            : box(iBox)
        {
            for(size_t i = 0; i < SG_ARRAYSIZE(children); ++i)
            {
                children[i] = all_ones;
            }
        }
    };
private:
    std::vector<Node> m_nodes;
    std::vector<size_t> m_layers;
};
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_RTREE_PER_LAYER || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_RTreePerLayer : public SafeCountable
{
public:
    LayerManager_RTreePerLayer();
    ~LayerManager_RTreePerLayer();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
#if SG_ENABLE_ASSERT
    void CheckIntegrity() const;
#endif
private:
    template <size_t M>
    struct RTreeNode {
        MaxSizeVector<std::pair<box2f, size_t>, M > boxes;

        RTreeNode() {}
        RTreeNode(RTreeNode const& iOther)
        {
            boxes.clear();
            for(auto const& it : iOther.boxes)
                boxes.emplace_back(it);
        }
        RTreeNode & operator=(RTreeNode const& iOther) = delete;
    };
    static size_t const M = 64;
    static size_t const m = 1;
    typedef RTreeNode<M> Node;
    static size_t const MAX_DEPTH = 8;
    typedef MaxSizeVector<size_t, MAX_DEPTH> Path;
private:
    /*SG_FORCE_INLINE*/ bool Find(box2f const& iBox, size_t iLayerIndex, Path& oInsertPoint);
    /*SG_FORCE_INLINE*/ void Insert(box2f const& iBox, size_t iLayerIndex, Path const& iInsertPoint);
    /*SG_FORCE_INLINE*/ void SplitNode(size_t iNodeIndex, box2f const& iBox, size_t iChildNodeIndex, box2f& oOldNodeBox, box2f& oNewNodeBox, size_t& oNewNodeIndex);
private:
    std::vector<Node> m_nodes;
    std::vector<size_t> m_layers;
#if SG_ENABLE_ASSERT
    size_t m_boxCount;
#endif
};
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_RTREE || defined(ENABLE_LAYER_MANAGER_COMPARISON)
class LayerManager_RTree : public SafeCountable
{
public:
    LayerManager_RTree();
    ~LayerManager_RTree();

    LayerRange Lock(box2f const& iBox, size_t iLayerCount = 1);
    void Clear();
#if SG_ENABLE_ASSERT
    void CheckIntegrity() const;
#endif
private:
    struct BoxAndMaxLayer
    {
        box2f box;
        size_t maxLayer;

        explicit BoxAndMaxLayer(uninitialized_t) : box(uninitialized) {}
        BoxAndMaxLayer(box2f const& iBox, size_t iMaxLayer) : box(iBox), maxLayer(iMaxLayer) {}
    };
    struct Branch : public BoxAndMaxLayer
    {
        size_t index;

        Branch(box2f const& iBox, size_t iMaxLayer, size_t iIndex) : BoxAndMaxLayer(iBox, iMaxLayer), index(iIndex) {}
        Branch(BoxAndMaxLayer const& iBoxAndMaxLayer, size_t iIndex) : BoxAndMaxLayer(iBoxAndMaxLayer), index(iIndex) {}
    };
    template <size_t M>
    struct RTreeNode
    {
        MaxSizeVector<Branch, M> children;

        RTreeNode() {}
        RTreeNode(RTreeNode const& iOther)
        {
            children.clear();
            for(auto const& it : iOther.children)
                children.emplace_back(it);
        }
        RTreeNode & operator=(RTreeNode const& iOther) = delete;
    };
    static size_t const M = 32;
    static size_t const m = M * 2 / 7;
    typedef RTreeNode<M> Node;
    static size_t const MAX_DEPTH = 8;
    typedef MaxSizeVector<size_t, MAX_DEPTH> Path;
private:
    /*SG_FORCE_INLINE*/ size_t FindReturnFreeLayer(box2f const& iBox, Path& oInsertPoint);
    /*SG_FORCE_INLINE*/ void Insert(BoxAndMaxLayer const& iBoxAndLayer, Path const& iInsertPoint);
    /*SG_FORCE_INLINE*/ void SplitNode(size_t iNodeIndex, BoxAndMaxLayer const& iBoxAndLayer, size_t iChildNodeIndex, BoxAndMaxLayer& oOldNodeBoxAndLayer, BoxAndMaxLayer& oNewNodeBoxAndLayer, size_t& oNewNodeIndex);
    /*SG_FORCE_INLINE*/ void BalanceNodes(Node& ioNodeLess, box2f& ioBBLess, Node& ioNodeMore, box2f& ioBBMore);
private:
    std::vector<Node> m_nodes;
    size_t m_root;
#if SG_ENABLE_ASSERT
    size_t m_boxCount;
#endif
};
#endif
//=============================================================================
#if   UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ONE_LAYER_PER_REQUEST
class LayerManager : public LayerManager_OneLayerPerRequest {};
#elif UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ALL_BOXES
class LayerManager : public LayerManager_AllBoxes {};
#elif UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_ONE_BOX_PER_LAYER
class LayerManager : public LayerManager_OneBoxPerLayer {};
#elif UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_N_BOXES_PER_LAYER
class LayerManager : public LayerManager_NBoxesPerLayer {};
#elif UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_QUAD_TREE_LIKE
class LayerManager : public LayerManager_QuadTreeLike {};
#elif UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_RTREE_PER_LAYER
class LayerManager : public LayerManager_RTreePerLayer {};
#elif UI_LAYER_MANAGER_VERSION == UI_LAYER_MANAGER_RTREE
class LayerManager : public LayerManager_RTree {};
#else
#error "error"
#endif
//=============================================================================
}
}

#endif

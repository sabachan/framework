#ifndef Rendering_RenderBatchDico_H
#define Rendering_RenderBatchDico_H

#include "TransientRenderBatch.h"

#include "Material.h"
#include <unordered_map>

namespace sg {
namespace rendering {
//=============================================================================
class Material;
class RenderBatchSet;
class RenderDevice;
//=============================================================================
// A RenderBatchDico is used to share render batches that share same properties
// like materials or type.
class RenderBatchDico
{
public:
    RenderBatchDico(RenderBatchSet* iRenderBatchSetOwner);
    ~RenderBatchDico();

    TransientRenderBatch* GetOrCreateTransientRenderBatch(
        RenderBatchSet* iRenderBatchSetOwner,
        Material const& iMaterial,
        TransientRenderBatch::Properties const& iProperties);
    // TODO: add other render batch types
    void Clear(RenderBatchSet* iRenderBatchSetOwner);
private:
    struct MaterialKey
    {
        safeptr<Material const> material;
        u8 indexSize;

        MaterialKey(Material const* iMaterial, u8 iIndexSize)
            : material(iMaterial)
            , indexSize(iIndexSize)
        {}
    };
    struct HashAndCompareMaterialKey
    {
        size_t operator() (MaterialKey const& a) const { return a.material->Hash() ^ a.indexSize; }
        bool operator() (MaterialKey const& a, MaterialKey const& b) const { return *a.material == *b.material && a.indexSize == b.indexSize; }
    };
    template <typename RenderBatch_Type>
    struct Dico
    {
        std::unordered_multimap<uintptr_t, size_t> indexFromPtr;
        std::unordered_map<MaterialKey, size_t, HashAndCompareMaterialKey, HashAndCompareMaterialKey> indexFromMat;
        std::vector<refptr<RenderBatch_Type>> renderBatches;
    };
    Dico<TransientRenderBatch> m_transientDico;

    SG_CODE_FOR_ASSERT(safeptr<RenderBatchSet const> m_renderBatchSetForCheck);
    SG_CODE_FOR_ASSERT(safeptr<RenderDevice const> m_renderDeviceForCheck);
};
//=============================================================================
}
}

#endif

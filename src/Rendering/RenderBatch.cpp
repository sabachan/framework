#include "stdafx.h"

#include "RenderBatch.h"

#include "RenderBatchSet.h"

namespace sg {
namespace rendering {
//=============================================================================
#if SG_ENABLE_ASSERT
IRenderBatch::IRenderBatch()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IRenderBatch::~IRenderBatch()
{
    SG_ASSERT(nullptr == m_setWhereRegistered);
}
#endif
//=============================================================================
}
}

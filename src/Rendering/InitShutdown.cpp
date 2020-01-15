#include "stdafx.h"

#include "InitShutdown.h"

#include "RenderStateDico.h"
#include "ShaderCache.h"
#include "ShaderConstantDatabase.h"
#include "ShaderResourceDatabase.h"
#include "TextureCache.h"

namespace sg {
namespace rendering {
//=============================================================================
void Init()
{
    RenderStateName::Init();
    ShaderConstantName::Init();
    ShaderResourceName::Init();
    renderstatedico::Init();
    shadercache::Init();
    texturecache::Init();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    texturecache::Shutdown();
    shadercache::Shutdown();
    renderstatedico::Shutdown();
    ShaderConstantName::Shutdown();
    ShaderResourceName::Shutdown();
    RenderStateName::Shutdown();
}
//=============================================================================
}
}

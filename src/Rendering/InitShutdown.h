#ifndef Rendering_InitShutdown_H
#define Rendering_InitShutdown_H

namespace sg {
namespace rendering {
//=============================================================================
// These methods offer a simpler way to init/shutdown all the components of
// Rendering that need such init/shutdown.
// However, single components init/shutdown can be done instead.
void Init();
void Shutdown();
//=============================================================================
}
}

#endif

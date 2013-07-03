#pragma once
#ifndef UserInterface_stdafx_H
#define UserInterface_stdafx_H

#include <Core/stdafx.h>
#include <FileFormats/stdafx.h>
#include <Image/stdafx.h>
#include <Math/stdafx.h>
#include <ObjectScript/stdafx.h>
#include <Reflection/stdafx.h>
#include <RenderEngine/stdafx.h>
#include <Rendering/stdafx.h>

#if SG_USE_STDAFX
//=============================================================================
#if SG_ENABLE_PROJECT_INCLUDES_IN_STDAFX
#include "AnimFactor.h"
#include "ClippingContainer.h"
#include "Component.h"
#include "Container.h"
#include "Context.h"
#include "FitMode.h"
#include "FrameProperty.h"
#include "LayerManager.h"
#include "Length.h"
#include "ListLayout.h"
#include "Magnifier.h"
#include "Movable.h"
#include "OffscreenContainer.h"
#include "SensitiveArea.h"
#include "Text.h"
#include "TextFormatScript.h"
#include "TextModifier.h"
#include "TextRenderer.h"
#include "TextStyle.h"
#include "TextureDrawer.h"
#include "Typeface.h"
#include "TypefaceFromBitMapFont.h"
#include "UniformDrawer.h"
#endif
//=============================================================================
#endif

#endif

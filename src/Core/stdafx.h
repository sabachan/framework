#pragma once
#ifndef Core_stdafx_H
#define Core_stdafx_H

#if !defined(COMPILATION_CONFIG_IS_DEBUG)
#define SG_USE_STDAFX 1
#define SG_ENABLE_PROJECT_INCLUDES_IN_STDAFX 1
#else
#define SG_USE_STDAFX 0
#endif


#if SG_USE_STDAFX
//=============================================================================
#include <algorithm>
#include <array>
#include <codecvt>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iomanip>
#include <iterator>
#include <list>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <vector>

#include "Platform.h"
#if SG_PLATFORM_IS_WIN
#include "WindowsH.h"
#endif
//=============================================================================
#if SG_ENABLE_PROJECT_INCLUDES_IN_STDAFX
#include "ArrayView.h"
#include "Assert.h"
#include "BitSet.h"
#include "Cast.h"
#include "Config.h"
#include "DynamicBitSet.h"
#include "FastSymbol.h"
#include "FilePath.h"
#include "FileSystem.h"
#include "FixedPoint.h"
#include "For.h"
#include "IntrusiveList.h"
#include "IntTypes.h"
#include "Log.h"
#include "MaxSizeVector.h"
#include "Observer.h"
#include "PerfLog.h"
#include "Platform.h"
#include "Preprocessor.h"
#include "Singleton.h"
#include "SmartPtr.h"
#include "StringFormat.h"
#include "StringUtils.h"
#include "TemplateUtils.h"
#include "TimeFromWallClock.h"
#include "TimeServer.h"
#include "Utils.h"
#include "VectorOfScopedPtr.h"
#endif
//=============================================================================
#endif

#endif

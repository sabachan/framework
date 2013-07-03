#ifndef Core_Config_H
#define Core_Config_H

#if !defined(COMPILATION_CONFIG_IS_DEBUG) && !defined(COMPILATION_CONFIG_IS_FAST_DEBUG)&& !defined(COMPILATION_CONFIG_IS_PERF) && !defined(COMPILATION_CONFIG_IS_RELEASE)
#error "One config must be defined among {COMPILATION_CONFIG_IS_DEBUG, COMPILATION_CONFIG_IS_FAST_DEBUG, COMPILATION_CONFIG_IS_PERF, COMPILATION_CONFIG_IS_RELEASE}"
#endif

#if defined(COMPILATION_CONFIG_IS_DEBUG)
#define SG_CODE_IS_OPTIMIZED 0
#else
#define SG_CODE_IS_OPTIMIZED 1
#endif

#if defined(COMPILATION_CONFIG_IS_DEBUG) || defined(COMPILATION_CONFIG_IS_FAST_DEBUG)
#define SG_ENABLE_ASSERT 1
#else
#define SG_ENABLE_ASSERT 0
#endif

#if defined(COMPILATION_CONFIG_IS_DEBUG) || defined(COMPILATION_CONFIG_IS_FAST_DEBUG) || defined(COMPILATION_CONFIG_IS_PERF)
#define SG_ENABLE_UNIT_TESTS 1
#define SG_ENABLE_TOOLS 1
#define SG_ENABLE_PERF_LOG 1
#define SG_ENABLE_LOG 1
#else
#define SG_ENABLE_UNIT_TESTS 0
#define SG_ENABLE_TOOLS 0
#define SG_ENABLE_PERF_LOG 0
#define SG_ENABLE_LOG 0
#endif

#define SG_CONTAINERS_EXPOSE_STD_NAMES 1

#endif

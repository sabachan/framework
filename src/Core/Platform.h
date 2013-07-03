#ifndef Core_Platform_H
#define Core_Platform_H

#define SG_PLATFORM_WIN  0x04db7614

#define SG_COMPILER_GCC  0x56ca178c
#define SG_COMPILER_MSVC 0x7997f0df

#define SG_ARCH_X86      0x642462d6
#define SG_ARCH_X64      0xb3080c5c

#if defined(_WIN32) || defined(_WIN64)
#define SG_PLATFORM SG_PLATFORM_WIN
#endif

#ifdef _MSC_VER
#define SG_COMPILER SG_COMPILER_MSVC
#endif

#if defined(_M_IX86)
#define SG_ARCH SG_ARCH_X86
#elif defined(_M_X64)
#define SG_ARCH SG_ARCH_X64
#endif

#if !defined(SG_PLATFORM)
#error "Platform has not been identified"
#endif
#if !defined(SG_COMPILER)
#error "Compiler has not been identified"
#endif
#if !defined(SG_ARCH)
#error "Architecture has not been identified"
#endif

#define SG_PLATFORM_IS_WIN (SG_PLATFORM == SG_PLATFORM_WIN)

#define SG_COMPILER_IS_MSVC (SG_COMPILER == SG_COMPILER_MSVC)
#define SG_COMPILER_IS_GCC  (SG_COMPILER == SG_COMPILER_GCC)

#define SG_ARCH_IS_X86      (SG_ARCH == SG_ARCH_X86)
#define SG_ARCH_IS_X64      (SG_ARCH == SG_ARCH_X64)

#endif

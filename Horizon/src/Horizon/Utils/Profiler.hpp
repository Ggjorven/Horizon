#pragma once

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <new>
#include <cstdlib>

// Note: Profiling leaks memory (on windows), so don't keep on during any real tests. // TODO: Check if it's still that way
#define HZ_ENABLE_PROFILING 1
#define HZ_MEM_PROFILING 0

#if !defined(HZ_DIST) && HZ_ENABLE_PROFILING

#define HZ_MARK_FRAME() FrameMark
#define HZ_PROFILE_SCOPE(name) ZoneScopedN(name)

#if HZ_MEM_PROFILING
void* operator new(size_t size);
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;
#endif

#else

#define HZ_MARK_FRAME()
#define HZ_PROFILE_SCOPE(name)

#endif

#pragma once

#include <stdint.h>

constexpr uint32_t InitialClientWidth = 1280;
constexpr uint32_t InitialClientHeight = 720;

constexpr int NumBuffers = 2;

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif
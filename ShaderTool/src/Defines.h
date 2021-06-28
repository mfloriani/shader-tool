#pragma once

#include <stdint.h>

//Common resolutions of 16:9 aspect ratio :
//3840×2160(Ultra - HD 4K)
//2560×1440(2K)
//1920×1080(1080p Full - HD)
//1600×900.
//1366×768.
//1280×720(720p HD)
//1024×576.

constexpr uint32_t INIT_CLIENT_WIDTH = 1366;
constexpr uint32_t INIT_CLIENT_HEIGHT = 768;

constexpr int NUM_BACK_BUFFERS = 3;
constexpr int NUM_FRAMES = 3;

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif
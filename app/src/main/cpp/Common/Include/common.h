#ifndef COMMON_H
#define COMMON_H

#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <android_fopen.h>

#define LogMessage(LOG_TAG, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define USE_VAO 1
#define USE_BLIT 1
#define USE_TIMER 0

enum BENCH_VERTEX_USAGE
{
    BENCH_VERTEX_POSITION,          // 0
    BENCH_VERTEX_NORMAL,            // 1
    BENCH_TEXCOORD,                 // 2
};

#include "esUtil.h"

#endif

#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
//  #define DEBUG_LEVEL	5

/* 从编译器参数或环境变量获取调试级别 */
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 4  // 默认关闭
#endif

/* 调试级别定义 */
#define DEBUG_NONE    0
#define DEBUG_ERROR   1
#define DEBUG_WARN    2
#define DEBUG_INFO    3
#define DEBUG_DEBUG   4
#define DEBUG_TRACE   5

/* 条件调试宏 */
#if DEBUG_LEVEL >= DEBUG_ERROR
#define LOG_ERROR(fmt, ...) \
    printf("[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_WARN
#define LOG_WARN(fmt, ...) \
    printf("[WARN]  %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_INFO
#define LOG_INFO(fmt, ...) \
    printf("[INFO]  %s:%s():%d " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_DEBUG
#define LOG_DEBUG(fmt, ...) \
    printf("[DEBUG] %s:%s():%d " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_TRACE
#define LOG_TRACE(fmt, ...) \
    printf("[TRACE] %s:%s():%d " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_TRACE(fmt, ...) ((void)0)
#endif

/* 条件编译的调试代码块 */
#ifdef ENABLE_DEBUG_CODE
#define DEBUG_CODE(code) code
#else
#define DEBUG_CODE(code) ((void)0)
#endif

#endif /* __DEBUG_H */
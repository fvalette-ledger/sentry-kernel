#ifndef LIBTEST_LOG_H
#define LIBTEST_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <uapi/uapi.h>

#define USER_AUTOTEST "[AT]"
#define USER_AUTOTEST_EXEC    "[       ]"
#define USER_AUTOTEST_START   "[RUN    ]"
#define USER_AUTOTEST_END     "[END    ]"
#define USER_AUTOTEST_FAIL    "[KO     ]"
#define USER_AUTOTEST_SUCCESS "[SUCCESS]"

#define pr_autotest_fmt(func, fmt) "%s: " fmt "\n", func

/**
 * @def autotest messages
 */
#define pr_autotest(type, func, fmt, ...) \
	printf(USER_AUTOTEST type " " pr_autotest_fmt(func, fmt), ##__VA_ARGS__)

#define TEST_START() do { \
    pr_autotest(USER_AUTOTEST_START, __func__, "start"); \
} while (0)

#define TEST_END() do { \
    pr_autotest(USER_AUTOTEST_END, __func__, "end"); \
} while (0)


static inline void failure_u32(const char *func, int line, const char*failure, uint32_t a, uint32_t b) {
    pr_autotest(USER_AUTOTEST_FAIL, func, ": %lu %s %lu", a, failure, b);
}

static inline void success_u32(const char *func, int line, const char*success, uint32_t a, uint32_t b) {
    pr_autotest(USER_AUTOTEST_SUCCESS, func, ": %lu %s %lu", a, success, b);
}

static inline void failure_int(const char *func, int line, const char*failure, int a, int b) {
    pr_autotest(USER_AUTOTEST_FAIL, func, ": %d %s %d", a, failure, b);
}

static inline void success_int(const char *func, int line, const char*success, int a, int b) {
    pr_autotest(USER_AUTOTEST_SUCCESS, func, ": %d %s %d", a, success, b);
}


#define success(f, l, msg, T,U) _Generic((T),   \
    uint32_t: _Generic((U),                     \
        uint32_t: success_u32,                  \
        Status: success_u32                     \
    )(f,l,msg,T,U),                             \
    Status: _Generic((U),                       \
        uint32_t: success_u32,                  \
        Status: success_u32                     \
    )(f,l,msg,T,U)                              \
)

#define failure(f, l, msg, T,U) _Generic((T),   \
    uint32_t: _Generic((U),                     \
        uint32_t: failure_u32,                  \
        Status: failure_u32                     \
    )(f,l,msg,T,U),                             \
    Status: _Generic((U),                       \
        uint32_t: failure_u32,                  \
        Status: failure_u32                     \
    )(f,l,msg,T,U)                              \
)

#ifdef __cplusplus
}
#endif

#endif

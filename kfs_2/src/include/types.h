/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   types.h - Standard kernel types                                          */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant                                    */
/*   - All types explicitly sized                                             */
/*   - No ambiguous type definitions                                          */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPES_H
#define TYPES_H

/*
** ==========================================================================
** Fixed-width integer types (explicit sizes for kernel development)
** ==========================================================================
*/

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

/*
** ==========================================================================
** Size and pointer types
** ==========================================================================
*/

typedef uint32_t            size_t;
typedef int32_t             ssize_t;
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;

/*
** ==========================================================================
** Boolean type
** ==========================================================================
*/

typedef uint8_t             bool_t;

#define TRUE                ((bool_t)1)
#define FALSE               ((bool_t)0)

/*
** ==========================================================================
** NULL pointer
** ==========================================================================
*/

#define NULL                ((void *)0)

/*
** ==========================================================================
** Compiler attributes for NASA compliance
** ==========================================================================
*/

#define PACKED              __attribute__((packed))
#define ALIGNED(x)          __attribute__((aligned(x)))
#define NORETURN            __attribute__((noreturn))
#define UNUSED              __attribute__((unused))
#define ALWAYS_INLINE       __attribute__((always_inline)) inline

/*
** ==========================================================================
** Static assertions (compile-time checks - NASA Rule #5 adaptation)
** ==========================================================================
*/

#define STATIC_ASSERT(cond, msg) \
    typedef char static_assertion_##msg[(cond) ? 1 : -1]

/* Verify type sizes at compile time */
STATIC_ASSERT(sizeof(uint8_t) == 1, uint8_t_must_be_1_byte);
STATIC_ASSERT(sizeof(uint16_t) == 2, uint16_t_must_be_2_bytes);
STATIC_ASSERT(sizeof(uint32_t) == 4, uint32_t_must_be_4_bytes);
STATIC_ASSERT(sizeof(uint64_t) == 8, uint64_t_must_be_8_bytes);

#endif /* TYPES_H */

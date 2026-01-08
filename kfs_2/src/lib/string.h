/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   string.h - Basic String Functions Interface                              */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant                                    */
/*   Replacement for standard library functions                               */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRING_H
#define STRING_H

#include "../include/types.h"

/*
** ==========================================================================
** Memory Functions
** ==========================================================================
*/

void    *k_memset(void *dest, int c, size_t n);
void    *k_memcpy(void *dest, const void *src, size_t n);
void    *k_memmove(void *dest, const void *src, size_t n);
int     k_memcmp(const void *s1, const void *s2, size_t n);

/*
** ==========================================================================
** String Functions
** ==========================================================================
*/

size_t  k_strlen(const char *str);
int     k_strcmp(const char *s1, const char *s2);
int     k_strncmp(const char *s1, const char *s2, size_t n);
char    *k_strcpy(char *dest, const char *src);
char    *k_strncpy(char *dest, const char *src, size_t n);

/*
** ==========================================================================
** Number to String Conversion (for printk)
** ==========================================================================
*/

void    k_itoa(int32_t value, char *buffer, int base);
void    k_utoa(uint32_t value, char *buffer, int base);

#endif /* STRING_H */

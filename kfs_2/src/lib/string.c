/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   string.c - Basic String Functions Implementation                         */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - All loops bounded                                                      */
/*   - No recursion                                                           */
/*   - Functions <= 60 lines                                                  */
/*                                                                            */
/* ************************************************************************** */

#include "string.h"

/*
** ==========================================================================
** Memory Functions
** ==========================================================================
*/

/*
** Fill memory with a constant byte
** NASA Rule #2: Loop bounded by parameter n
*/
void *k_memset(void *dest, int c, size_t n)
{
    uint8_t *ptr;
    size_t  i;

    if (dest == NULL)
    {
        return (NULL);
    }
    ptr = (uint8_t *)dest;
    i = 0;
    while (i < n)
    {
        ptr[i] = (uint8_t)c;
        i++;
    }
    return (dest);
}

/*
** Copy memory area (non-overlapping)
** NASA Rule #2: Loop bounded by parameter n
*/
void *k_memcpy(void *dest, const void *src, size_t n)
{
    uint8_t         *d;
    const uint8_t   *s;
    size_t          i;

    if (dest == NULL || src == NULL)
    {
        return (NULL);
    }
    d = (uint8_t *)dest;
    s = (const uint8_t *)src;
    i = 0;
    while (i < n)
    {
        d[i] = s[i];
        i++;
    }
    return (dest);
}

/*
** Copy memory area (handles overlapping)
** NASA Rule #2: Loop bounded by parameter n
*/
void *k_memmove(void *dest, const void *src, size_t n)
{
    uint8_t         *d;
    const uint8_t   *s;
    size_t          i;

    if (dest == NULL || src == NULL)
    {
        return (NULL);
    }
    d = (uint8_t *)dest;
    s = (const uint8_t *)src;
    if (d < s)
    {
        i = 0;
        while (i < n)
        {
            d[i] = s[i];
            i++;
        }
    }
    else if (d > s)
    {
        i = n;
        while (i > 0)
        {
            i--;
            d[i] = s[i];
        }
    }
    return (dest);
}

/*
** Compare two memory areas
** NASA Rule #2: Loop bounded by parameter n
** Returns: 0 if equal, <0 if s1<s2, >0 if s1>s2
*/
int k_memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t   *p1;
    const uint8_t   *p2;
    size_t          i;

    if (s1 == NULL || s2 == NULL)
    {
        return (0);
    }
    p1 = (const uint8_t *)s1;
    p2 = (const uint8_t *)s2;
    i = 0;
    while (i < n)
    {
        if (p1[i] != p2[i])
        {
            return (p1[i] - p2[i]);
        }
        i++;
    }
    return (0);
}

/*
** ==========================================================================
** String Functions
** ==========================================================================
*/

/*
** Calculate string length
** NASA Rule #2: Loop bounded by null terminator
*/
size_t k_strlen(const char *str)
{
    size_t len;

    if (str == NULL)
    {
        return (0);
    }
    len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return (len);
}

/*
** Compare two strings
** Returns: 0 if equal, <0 if s1<s2, >0 if s1>s2
*/
int k_strcmp(const char *s1, const char *s2)
{
    size_t i;

    if (s1 == NULL || s2 == NULL)
    {
        return (0);
    }
    i = 0;
    while (s1[i] != '\0' && s2[i] != '\0')
    {
        if (s1[i] != s2[i])
        {
            return ((uint8_t)s1[i] - (uint8_t)s2[i]);
        }
        i++;
    }
    return ((uint8_t)s1[i] - (uint8_t)s2[i]);
}

/*
** Compare two strings up to n characters
** NASA Rule #2: Loop bounded by parameter n
*/
int k_strncmp(const char *s1, const char *s2, size_t n)
{
    size_t i;

    if (s1 == NULL || s2 == NULL || n == 0)
    {
        return (0);
    }
    i = 0;
    while (i < n && s1[i] != '\0' && s2[i] != '\0')
    {
        if (s1[i] != s2[i])
        {
            return ((uint8_t)s1[i] - (uint8_t)s2[i]);
        }
        i++;
    }
    if (i < n)
    {
        return ((uint8_t)s1[i] - (uint8_t)s2[i]);
    }
    return (0);
}

/*
** Copy string
*/
char *k_strcpy(char *dest, const char *src)
{
    size_t i;

    if (dest == NULL || src == NULL)
    {
        return (NULL);
    }
    i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return (dest);
}

/*
** Copy string up to n characters
** NASA Rule #2: Loop bounded by parameter n
*/
char *k_strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    if (dest == NULL || src == NULL)
    {
        return (NULL);
    }
    i = 0;
    while (i < n && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    while (i < n)
    {
        dest[i] = '\0';
        i++;
    }
    return (dest);
}

/*
** ==========================================================================
** Number to String Conversion
** ==========================================================================
*/

/*
** Convert signed integer to string
** Base: 2 to 16
** NASA Rule #2: Loop bounded by maximum digits (32 for base 2)
*/
void k_itoa(int32_t value, char *buffer, int base)
{
    static const char   digits[] = "0123456789ABCDEF";
    char                temp[33];
    int                 i;
    int                 j;
    int                 is_negative;
    uint32_t            uvalue;

    if (buffer == NULL || base < 2 || base > 16)
    {
        return;
    }
    is_negative = 0;
    if (value < 0 && base == 10)
    {
        is_negative = 1;
        uvalue = (uint32_t)(-(value + 1)) + 1;
    }
    else
    {
        uvalue = (uint32_t)value;
    }
    i = 0;
    if (uvalue == 0)
    {
        temp[i] = '0';
        i++;
    }
    while (uvalue > 0 && i < 32)
    {
        temp[i] = digits[uvalue % (uint32_t)base];
        uvalue = uvalue / (uint32_t)base;
        i++;
    }
    j = 0;
    if (is_negative)
    {
        buffer[j] = '-';
        j++;
    }
    while (i > 0)
    {
        i--;
        buffer[j] = temp[i];
        j++;
    }
    buffer[j] = '\0';
}

/*
** Convert unsigned integer to string
** NASA Rule #2: Loop bounded by maximum digits (32 for base 2)
*/
void k_utoa(uint32_t value, char *buffer, int base)
{
    static const char   digits[] = "0123456789ABCDEF";
    char                temp[33];
    int                 i;
    int                 j;

    if (buffer == NULL || base < 2 || base > 16)
    {
        return;
    }
    i = 0;
    if (value == 0)
    {
        temp[i] = '0';
        i++;
    }
    while (value > 0 && i < 32)
    {
        temp[i] = digits[value % (uint32_t)base];
        value = value / (uint32_t)base;
        i++;
    }
    j = 0;
    while (i > 0)
    {
        i--;
        buffer[j] = temp[i];
        j++;
    }
    buffer[j] = '\0';
}

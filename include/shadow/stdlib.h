#ifndef __CU2CCE_SHADOW_STDLIB_H__
#define __CU2CCE_SHADOW_STDLIB_H__
/*
 * cu2cce 遮蔽头（shadow header）——stdlib.h 替代版。
 * 背景与机制见 shadow/stdio.h。size_t 用 unsigned long（x86_64 Linux 等价）。
 */
/*
 * glibc declares most stdlib entry points with __THROW (noexcept in C++).
 * libstdc++'s <cstdlib> resolves this file via -I, then pulls in the real
 * /usr/include/stdlib.h through #include_next, so both declarations are seen
 * in one translation unit. Matching glibc's exception specification is
 * therefore required, or the redeclaration is rejected ("exception
 * specification ... does not match previous declaration"). system() is left
 * unguarded because glibc does not mark it __THROW.
 */
#if defined(__cplusplus)
#define CU2CCE_STDLIB_THROW noexcept
extern "C" {
#else
#define CU2CCE_STDLIB_THROW
#endif

typedef unsigned long size_t;

void *malloc(unsigned long) CU2CCE_STDLIB_THROW;
void *calloc(unsigned long, unsigned long) CU2CCE_STDLIB_THROW;
void *realloc(void *, unsigned long) CU2CCE_STDLIB_THROW;
void free(void *) CU2CCE_STDLIB_THROW;
void exit(int) CU2CCE_STDLIB_THROW;
void abort(void) CU2CCE_STDLIB_THROW;
int atoi(const char *) CU2CCE_STDLIB_THROW;
long atol(const char *) CU2CCE_STDLIB_THROW;
double atof(const char *) CU2CCE_STDLIB_THROW;
int abs(int) CU2CCE_STDLIB_THROW;
long labs(long) CU2CCE_STDLIB_THROW;
char *getenv(const char *) CU2CCE_STDLIB_THROW;
int system(const char *);

#if defined(__cplusplus)
}
#undef CU2CCE_STDLIB_THROW
#endif
#endif /* __CU2CCE_SHADOW_STDLIB_H__ */

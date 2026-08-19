#ifndef __CU2CCE_SHADOW_STDLIB_H__
#define __CU2CCE_SHADOW_STDLIB_H__
/*
 * cu2cce 遮蔽头（shadow header）——stdlib.h 替代版。
 * 背景与机制见 shadow/stdio.h。size_t 用 unsigned long（x86_64 Linux 等价）。
 */
#if defined(__cplusplus)
extern "C" {
#endif

typedef unsigned long size_t;

void *malloc(unsigned long);
void *calloc(unsigned long, unsigned long);
void *realloc(void *, unsigned long);
void free(void *);
void exit(int);
void abort(void);
int atoi(const char *);
long atol(const char *);
double atof(const char *);
int abs(int);
long labs(long);
char *getenv(const char *);
int system(const char *);

#if defined(__cplusplus)
}
#endif
#endif /* __CU2CCE_SHADOW_STDLIB_H__ */

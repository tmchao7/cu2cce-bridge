#ifndef __CU2CCE_SHADOW_STRING_H__
#define __CU2CCE_SHADOW_STRING_H__
/*
 * cu2cce 遮蔽头（shadow header）——string.h 替代版。
 * 背景与机制见 shadow/stdio.h。
 */
#if defined(__cplusplus)
extern "C" {
#endif

typedef unsigned long size_t;

void *memset(void *, int, unsigned long);
void *memcpy(void *, const void *, unsigned long);
void *memmove(void *, const void *, unsigned long);
int memcmp(const void *, const void *, unsigned long);
unsigned long strlen(const char *);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, unsigned long);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, unsigned long);
char *strcat(char *, const char *);
char *strstr(const char *, const char *);
char *strchr(const char *, int);
char *strrchr(const char *, int);
char *strdup(const char *);

#if defined(__cplusplus)
}
#endif
#endif /* __CU2CCE_SHADOW_STRING_H__ */

#ifndef __CU2CCE_SHADOW_STDIO_H__
#define __CU2CCE_SHADOW_STDIO_H__
/*
 * cu2cce shadow stdio.h.
 *
 * ccec + the target system glibc can reject the full system stdio headers.
 * This declaration-only header keeps ordinary CUDA sources source-compatible,
 * including C++ <cstdio>, while the final symbols still come from libc.
 */
/*
 * This header fully replaces the system stdio.h in shadow-libc mode: libstdc++
 * <cstdio> uses a plain `#include <stdio.h>` (no #include_next), so it resolves
 * here and the real glibc header is never parsed. The macros below mirror the
 * glibc constants that C++ stdlib code expects (fstream relies on BUFSIZ,
 * iostream on EOF, etc.).
 */
#ifndef BUFSIZ
#define BUFSIZ 8192
#endif
#ifndef EOF
#define EOF (-1)
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef _IOFBF
#define _IOFBF 0
#endif
#ifndef _IOLBF
#define _IOLBF 1
#endif
#ifndef _IONBF
#define _IONBF 2
#endif
#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif
#ifndef FOPEN_MAX
#define FOPEN_MAX 16
#endif
#ifndef L_tmpnam
#define L_tmpnam 20
#endif
#ifndef TMP_MAX
#define TMP_MAX 238328
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef __CU2CCE_SIZE_T_DEFINED
#define __CU2CCE_SIZE_T_DEFINED
typedef unsigned long size_t;
#endif
#ifndef __CU2CCE_VA_LIST_DEFINED
#define __CU2CCE_VA_LIST_DEFINED
typedef __builtin_va_list va_list;
#endif

typedef struct _IO_FILE FILE;
typedef long fpos_t;

int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int sprintf(char *, const char *, ...);
int snprintf(char *, size_t, const char *, ...);
int vprintf(const char *, __builtin_va_list);
int vfprintf(FILE *, const char *, __builtin_va_list);
int vsprintf(char *, const char *, __builtin_va_list);
int vsnprintf(char *, size_t, const char *, __builtin_va_list);
int puts(const char *);
int putchar(int);
int fputc(int, FILE *);
int putc(int, FILE *);
int fputs(const char *, FILE *);
int fgetc(FILE *);
int getc(FILE *);
int getchar(void);
int ungetc(int, FILE *);
int fflush(FILE *);
int fclose(FILE *);
void clearerr(FILE *);
int feof(FILE *);
int ferror(FILE *);
FILE *fopen(const char *, const char *);
FILE *freopen(const char *, const char *, FILE *);
FILE *tmpfile(void);
int fseek(FILE *, long, int);
int fgetpos(FILE *, fpos_t *);
int fsetpos(FILE *, const fpos_t *);
long ftell(FILE *);
size_t fread(void *, size_t, size_t, FILE *);
size_t fwrite(const void *, size_t, size_t, FILE *);
int fscanf(FILE *, const char *, ...);
int scanf(const char *, ...);
int sscanf(const char *, const char *, ...);
int vfscanf(FILE *, const char *, __builtin_va_list);
int vscanf(const char *, __builtin_va_list);
int vsscanf(const char *, const char *, __builtin_va_list);
char *fgets(char *, int, FILE *);
void perror(const char *);
int remove(const char *);
int rename(const char *, const char *);
void rewind(FILE *);
void setbuf(FILE *, char *);
int setvbuf(FILE *, char *, int, size_t);
char *tmpnam(char *);

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#if defined(__cplusplus)
}

// libstdc++ exposes these scanf variants through its internal namespace in
// <cstdio>. The declarations still use libc's C linkage.
namespace __gnu_cxx {
using ::vfscanf;
using ::vscanf;
using ::vsscanf;
}
#endif
#endif /* __CU2CCE_SHADOW_STDIO_H__ */

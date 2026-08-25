#ifndef __CU2CCE_SHADOW_STDIO_H__
#define __CU2CCE_SHADOW_STDIO_H__
/*
 * cu2cce shadow stdio.h.
 *
 * ccec + the target system glibc can reject the full system stdio headers.
 * This declaration-only header keeps ordinary CUDA sources source-compatible,
 * including C++ <cstdio>, while the final symbols still come from libc.
 */
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
#endif
#endif /* __CU2CCE_SHADOW_STDIO_H__ */

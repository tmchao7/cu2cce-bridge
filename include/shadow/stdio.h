#ifndef __CU2CCE_SHADOW_STDIO_H__
#define __CU2CCE_SHADOW_STDIO_H__
/*
 * cu2cce 遮蔽头（shadow header）——stdio.h 替代版。
 *
 * 背景：ccec（clang 15）+ glibc 2.39（Ubuntu 24.04）的系统头不兼容
 * （真机实测 _Float32/_Float64/__FILE/ferror 连锁错误）。用户 CUDA 代码
 * #include <stdio.h> 是合法代码、不能改（项目原则），因此由 cucc 把本目录
 * 注入到 include 搜索路径最前面，让 <stdio.h> 优先命中本遮蔽头（不碰 glibc）。
 *
 * 只提供常用函数的最小声明，链接期由 libc 提供符号；需要更多函数时在此追加。
 * 与系统头不冲突：本头不 include 任何系统头。
 */
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _IO_FILE FILE;

int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int sprintf(char *, const char *, ...);
int snprintf(char *, unsigned long, const char *, ...);
int puts(const char *);
int putchar(int);
int fflush(FILE *);
int fclose(FILE *);
FILE *fopen(const char *, const char *);
size_t fread(void *, unsigned long, unsigned long, FILE *);
size_t fwrite(const void *, unsigned long, unsigned long, FILE *);
int fseek(FILE *, long, int);
long ftell(FILE *);
int feof(FILE *);
int ferror(FILE *);
char *fgets(char *, int, FILE *);
int scanf(const char *, ...);
int sscanf(const char *, const char *, ...);

#if defined(__cplusplus)
}
#endif
#endif /* __CU2CCE_SHADOW_STDIO_H__ */

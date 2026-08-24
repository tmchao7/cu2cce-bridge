#ifndef __CU2CCE_SHADOW_MATH_H__
#define __CU2CCE_SHADOW_MATH_H__
/*
 * cu2cce 遮蔽头（shadow header）——math.h 替代版。
 * 背景与机制见 shadow/stdio.h。提供常用 float/double 数学函数声明；
 * 链接期需要 -lm（cucc 链接阶段已加）。昇腾 device 侧数学（SIMT 内置）不受影响。
 */
#if defined(__cplusplus)
extern "C" {
#endif

double sin(double);
double cos(double);
double tan(double);
double asin(double);
double acos(double);
double atan(double);
double atan2(double, double);
double exp(double);
double log(double);
double log10(double);
double pow(double, double);
double sqrt(double);
double cbrt(double);
double floor(double);
double ceil(double);
double round(double);
double fabs(double);
double fmod(double, double);
double fmin(double, double);
double fmax(double, double);

float sinf(float);
float cosf(float);
float tanf(float);
float asinf(float);
float acosf(float);
float atanf(float);
float atan2f(float, float);
float expf(float);
float logf(float);
float log10f(float);
float powf(float, float);
float sqrtf(float);
float cbrtf(float);
float floorf(float);
float ceilf(float);
float roundf(float);
float fabsf(float);
float fmodf(float, float);
float fminf(float, float);
float fmaxf(float, float);
float fmaf(float, float, float);

#if defined(__cplusplus)
}
#endif
#endif /* __CU2CCE_SHADOW_MATH_H__ */

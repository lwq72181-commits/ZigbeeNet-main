/*
 * File: fft.h
 *
 * MATLAB Coder version            : 4.3
 * C/C++ source code generated on  : 28-May-2024 09:17:04
 */

#ifndef FFT_H
#define FFT_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "Integral_types.h"

/* Function Declarations */
extern void b_fft(const emxArray_creal_T *x, int n, emxArray_creal_T *y);
extern void fft(const emxArray_real_T *x, int n, emxArray_creal_T *y);

#endif

/*
 * File trailer for fft.h
 *
 * [EOF]
 */

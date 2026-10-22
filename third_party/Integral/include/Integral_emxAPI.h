/*
 * File: Integral_emxAPI.h
 *
 * MATLAB Coder version            : 4.3
 * C/C++ source code generated on  : 28-May-2024 09:17:04
 */

#ifndef INTEGRAL_EMXAPI_H
#define INTEGRAL_EMXAPI_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "Integral_types.h"

/* Function Declarations */
extern emxArray_real_T *emxCreateND_real_T(int numDimensions, const int *size);
extern emxArray_real_T *emxCreateWrapperND_real_T(double *data, int
  numDimensions, const int *size);
extern emxArray_real_T *emxCreateWrapper_real_T(double *data, int rows, int cols);
extern emxArray_real_T *emxCreate_real_T(int rows, int cols);
extern void emxDestroyArray_real_T(emxArray_real_T *emxArray);
extern void emxInitArray_real_T(emxArray_real_T **pEmxArray, int numDimensions);

#endif

/*
 * File trailer for Integral_emxAPI.h
 *
 * [EOF]
 */

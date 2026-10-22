/*
 * File: Integral_emxutil.h
 *
 * MATLAB Coder version            : 4.3
 * C/C++ source code generated on  : 28-May-2024 09:17:04
 */

#ifndef INTEGRAL_EMXUTIL_H
#define INTEGRAL_EMXUTIL_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "Integral_types.h"

/* Function Declarations */
extern void emxEnsureCapacity_creal_T(emxArray_creal_T *emxArray, int oldNumel);
extern void emxEnsureCapacity_real_T(emxArray_real_T *emxArray, int oldNumel);
extern void emxFree_creal_T(emxArray_creal_T **pEmxArray);
extern void emxFree_real_T(emxArray_real_T **pEmxArray);
extern void emxInit_creal_T(emxArray_creal_T **pEmxArray, int numDimensions);
extern void emxInit_real_T(emxArray_real_T **pEmxArray, int numDimensions);

#endif

/*
 * File trailer for Integral_emxutil.h
 *
 * [EOF]
 */

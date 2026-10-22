#ifndef ADCHANDLE_H
#define ADCHANDLE_H
#include "Acquisitiontype.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "rtwtypes.h"
#include "Integral_types.h"
#include "Integral.h"
#include "Integral_emxAPI.h"
#include "Integral_terminate.h"
#include "rt_nonfinite.h"
#ifdef __cplusplus
}
#endif

class AdcHandle
{
public:
    AdcHandle();
    int initADC();
    int setAdParam(int freq);
    int getAdDataIndex(QList<FREIndexValue> *data);
    int getAdDataWave(FREWaveValue *waveData);
    int getAdDataLongWave(char *data);
    double calculateRMS(double *data, int n);
    emxArray_real_T *argInit_1xUnbounded_real_T();
protected:
    int m_data[4096];
    emxArray_real_T *m_FFTData;
    emxArray_real_T *m_ADData;
};

#endif // ADCHANDLE_H

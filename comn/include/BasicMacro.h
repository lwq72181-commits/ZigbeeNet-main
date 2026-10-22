/*
 * TBasicMacro.h
 *
 *  Created on: 2015年5月26日
 *      Author: chuanjiang.zh@qq.com
 */

#ifndef INCLUDE_BASICMACRO_H_
#define INCLUDE_BASICMACRO_H_


#ifndef     SAFE_DELETE
#define     SAFE_DELETE(p)          if((p) != NULL) {delete (p); (p) = NULL;}
#endif

#ifndef     SAFE_DELETE_ARRAY
#define     SAFE_DELETE_ARRAY(p)    if((p) != NULL) {delete[] (p);(p) = NULL;}
#endif

#ifndef     ARRAY_SIZE
#define     ARRAY_SIZE(x)           (sizeof(x)/sizeof((x)[0]))
#endif

#ifndef     CheckPointer
#define     CheckPointer(p,ret)     {if( (p) == NULL) return (ret);}
#endif

#ifndef     CheckCondition
#define     CheckCondition(p,ret)       {if(!(p)) return (ret);}
#endif


#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((uint16_t)(((uint8_t)(((uint32_t)(a)) & 0xff)) | ((uint16_t)((uint8_t)(((uint32_t)(b)) & 0xff))) << 8))
#endif //MAKEWORD

#ifndef MAKEDWORD
#define MAKEDWORD(a, b)     ((uint32_t)(((uint16_t)(((uint32_t)(a)) & 0xffff)) | ((uint32_t)((uint16_t)(((uint32_t)(b)) & 0xffff))) << 16))
#endif //MAKEDWORD

#ifndef LOWORD
#define LOWORD(l)           ((uint16_t)(((uint32_t)(l)) & 0xffff))
#endif //LOWORD

#ifndef HIWORD
#define HIWORD(l)           ((uint16_t)((((uint32_t)(l)) >> 16) & 0xffff))
#endif //HIWORD

#ifndef LOBYTE
#define LOBYTE(w)           ((uint8_t)(((uint32_t)(w)) & 0xff))
#endif //LOBYTE

#ifndef HIBYTE
#define HIBYTE(w)           ((uint8_t)((((uint32_t)(w)) >> 8) & 0xff))
#endif //HIBYTE



#endif /* INCLUDE_BASICMACRO_H_ */

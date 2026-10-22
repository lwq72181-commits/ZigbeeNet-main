#ifndef __DT_HEAP_H
#define __DT_HEAP_H
#ifdef __cplusplus
extern "C" {
#endif
#include "list.h"

#define DT_MEM_STATS_ENABLE /* enable memory status statistics*/

#ifdef DT_MEM_STATS_ENABLE

/* 内存申请结果 */
typedef enum _heap_memory_state_def{
    HEAP_MEMORY_ERROR = 0x00,
    HEAP_MEMORY_SUCCESS,
    HEAP_MEMORY_MAX
}heap_memory_state_t;

/* 内存对齐     */
#define MALLOC_ALIGN_SIZE 4
#define MALLOC_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))

typedef struct dt_hemp_record_list_def {
    struct list_head list; /* 链表节点 */
    void *ptr;             /* 申请地址 */
    unsigned int size;     /* 申请大小 */
    unsigned int line;     /* 申请行数 */
    char name[64];         /* 申请函数名 */
}dt_hemp_record_list_t;

/* public interface  */
void *_dt_malloc(unsigned int size, const char *func, int line);
#define dt_malloc(size) _dt_malloc(size,__func__,__LINE__)
void *_dt_calloc(unsigned int count, unsigned int size, const char *func, int line);
#define dt_calloc(count, size) _dt_calloc(count, size,__func__,__LINE__)
void dt_free(void *rmem);


typedef struct _dt_heap_manager_def{
    unsigned int used_mem;  /* 已经使用内存 */
    unsigned int max_mem;   /* 最大使用内存 */
    dt_hemp_record_list_t info; /* 信息记录 */
    char is_init;               /* 初始化标记 */
}dt_heap_manager_t;
void dt_heap_module_init(void);
void dt_print_malloc_size(void);

static inline void dt_auto_free(void **ptr)
{
    if (*ptr) {
        dt_free(*ptr);
        *ptr = NULL;
    }
}
#define dt_malloc_type  __attribute__ ((cleanup(dt_auto_free)))

#endif /* DT_MEM_STATS_ENABLE */
#ifdef __cplusplus
}
#endif
#endif /* __DT_HEAP_H */


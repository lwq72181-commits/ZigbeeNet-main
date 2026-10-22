#include "dt_heap.h"
#include <pthread.h>
#include "common.h"

#ifdef DT_MEM_STATS_ENABLE
static dt_heap_manager_t heap_mg;
static pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void dt_print_malloc_size(void)
{
    pthread_mutex_lock(&malloc_mutex);
    DT_LOG("****************** malloc info printf start ******************\r\n");
    DT_LOG("used memory : %d\r\n", heap_mg.used_mem);
    DT_LOG("maximum allocated memory: %d\r\n", heap_mg.max_mem);
    DT_LOG("malloc mem cnt: %d\r\n", list_len(&heap_mg.info.list));

    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    dt_hemp_record_list_t *plist = NULL;
    list_for_each_safe(pos, n, &heap_mg.info.list) {
        plist = container_of(pos, dt_hemp_record_list_t, list);
        if (plist->ptr) {
            DT_DBG("func name %s --- line %d --- size %d\r\n",
                plist->name, plist->line, plist->size);
        }
    }
    DT_LOG("****************** malloc info printf end ******************\r\n");
    pthread_mutex_unlock(&malloc_mutex);
}

void *_dt_malloc(unsigned int size, const char *func, int line)
{
    void *ptr = NULL;
    if (!heap_mg.is_init)
    {
        DT_DBG("_dt_malloc  is_init failed\r\n");
        return NULL;
    }
    dt_hemp_record_list_t *plist = NULL;
    pthread_mutex_lock(&malloc_mutex);
    plist = (dt_hemp_record_list_t*)malloc(sizeof(dt_hemp_record_list_t));
    size = MALLOC_ALIGN(size, MALLOC_ALIGN_SIZE);
    ptr = malloc(size);
    if ((NULL != plist) && (NULL != ptr)) {
        INIT_LIST_HEAD(&plist->list);
        plist->ptr = ptr;
        plist->size = size;
        plist->line = line;
        snprintf(plist->name, sizeof(plist->name), "%s", func);
        list_add_tail(&plist->list, &heap_mg.info.list);
        heap_mg.used_mem += size;
        if (heap_mg.used_mem > heap_mg.max_mem)
            heap_mg.max_mem = heap_mg.used_mem;
    }
    pthread_mutex_unlock(&malloc_mutex);
    return ptr;
}

void *_dt_calloc(unsigned int count, unsigned int size, const char *func, int line)
{
    void *p = NULL;

    /* allocate 'count' objects of size 'size' */
    p = _dt_malloc(count * size, func, line);
    /* zero the memory */
    if (p)
        memset(p, 0, count * size);
    return p;
}

void dt_free(void *rmem)
{
    if (NULL == rmem || !heap_mg.is_init) {
        return;
    }
    pthread_mutex_lock(&malloc_mutex);
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    dt_hemp_record_list_t *plist = NULL;
    list_for_each_safe(pos, n, &heap_mg.info.list) {
        plist = container_of(pos, dt_hemp_record_list_t, list);
        if (plist->ptr == rmem) {
            heap_mg.used_mem -= plist->size;
            list_del_init(&plist->list);
            if (plist) {
                free(plist);
                plist = NULL;
            }
            break;
        }
    }
    free(rmem);
    pthread_mutex_unlock(&malloc_mutex);
}

void dt_heap_module_init(void)
{
    INIT_LIST_HEAD(&heap_mg.info.list);
    heap_mg.is_init = 1;
}

#endif /* DT_MEM_STATS_ENABLE */
#if 0
void dt_heap_user_test(void)
{
    dt_heap_module_init();
    unsigned char *ptr = dt_malloc(1);
    if (NULL == ptr) {
        DT_ERR("malloc failed \r\n");
        return;
    }
    unsigned char *ptr2 = dt_malloc(1);
    if (NULL == ptr2) {
        DT_ERR("malloc failed \r\n");
        return ;
    }
    DT_DBG("malloc ok \r\n");
    dt_free(ptr);
    DT_DBG("free ok \r\n");
    print_malloc_size();
}
#endif

#ifndef __ROZH_VPOPEN_H_
#define __ROZH_VPOPEN_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/wait.h>

FILE *vpopen(const char* cmdstring, const char *type);
int vpclose(FILE *fp);
#ifdef __cplusplus
}
#endif
#endif


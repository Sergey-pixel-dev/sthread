#ifndef _STHREAD_
#define _STHREAD_
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "unistd.h"
#include <stdlib.h>
#include <string.h> // для memcpy

#include "stdint.h"
typedef struct sthread sthread;
typedef struct scontext scontext;
typedef enum
{
    RUN,
    WAIT,
    NEW, // первый запуск
    DONE
} sthread_state;
void delete_current_thread(void);
void sthread_create(void (*pfunc)(void *), void *func_arg);
void sthread_exit();
extern void *context_swap(scontext *);

#endif
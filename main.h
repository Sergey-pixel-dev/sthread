#ifndef _STHREAD_
#define _STHREAD_
typedef struct sthread sthread;
typedef struct scontext scontext;
void sthread_free(sthread *thread);
void sthread_create(void (*pfunc)(void *), void *func_arg);
extern void *context_swap(scontext *);

#endif
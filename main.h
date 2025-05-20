#ifndef _STHREAD_
#define _STHREAD_
typedef struct sthread sthread;
void sthread_free(sthread *thread);
void sthread_create(void (*pfunc)(void *), void *func_arg);
extern void *switcher(void (*pfunc)(void *), void *stack_top, void *arg);

#endif
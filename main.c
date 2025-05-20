
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "unistd.h"
#include <stdlib.h>
#include <string.h> // для memcpy
#include "main.h"
#include "stdint.h"
#define STACK_SIZE (1024 * 1024) // 1 MiB
#define MAX_STACK_SAVE (1024 * 64)

uint32_t id = 0;
struct scontext
{
    uint64_t gregs[23]; // все «общие» регистры
    uint64_t rip;       // инструкция
    uint64_t eflags;    // флаги
    // uint8_t fxsave[512] __attribute__((aligned(16))); // FPU/SSE состояние (XSAVE frame)
    void *stack_ptr; // RSP
    // size_t stack_size;                             // для полного снимка стека
    // void *stack_copy;                              // указатель на копию содержимого стека
};
struct sthread
{
    void *stack;
    uint32_t stack_size;
    uint32_t id;
    void (*pfunc)(void *);
    void *func_arg;
    scontext context;
    sthread *next;
};

sthread *head = NULL; // ПРИОРИТЕТНАЯ ОЧЕРЕДЬ НУЖНА
sthread *tail = NULL;
sthread *cur = NULL;
char HasStarted = 0;
// Функция, которую нужно вызывать по таймеру
void foo(void *arg) //
{
    printf("Hello, world!\n");
    while (1) // без возврата чтоб была
    {
    }
}

void timer_handler(int sig, siginfo_t *info, void *ucontext)
{

    if (head == NULL)
        return;
    if (cur == NULL) // при первом запуске
    {
        cur = head;
    }
    // HasStarted == 0 ни разу не запускали sheduler -> не зачем сохранять пред контекст
    // иначе сохраняем
    if (HasStarted)
    {
        ucontext_t *uc = (ucontext_t *)ucontext;
        memcpy(cur->context.gregs,
               uc->uc_mcontext.gregs,
               sizeof(cur->context.gregs));
        cur->context.rip = uc->uc_mcontext.gregs[REG_RIP];
        cur->context.eflags = uc->uc_mcontext.gregs[REG_EFL];
        HasStarted = 1;
    }
    cur = cur->next;
    context_swap(&cur->context);

    /* sthread *temp = cur->next;
    sthread_free(cur);
    cur = temp;
    head = cur; */
    // переход дальше (список закольцован)
}

void sthread_create(void (*pfunc)(void *), void *func_arg)
{
    sthread *thread = malloc(sizeof(sthread));

    thread->stack = (char *)malloc(STACK_SIZE);
    thread->stack_size = STACK_SIZE;
    thread->id = id++;
    thread->pfunc = pfunc;
    thread->func_arg = func_arg;
    thread->next = NULL;

    thread->context.rip = 0;
    thread->context.stack_ptr = thread->stack + STACK_SIZE;
    thread->context.gregs[REG_RSP] = (uint64_t)(uintptr_t)thread->context.stack_ptr;
    thread->context.gregs[REG_RIP] = (uint64_t)(uintptr_t)thread->pfunc;
    if (head == NULL)
    {
        head = tail = thread;
    }
    else
    {

        tail->next = thread;
        tail = thread;
        tail->next = head;
    }
}

void sthread_free(sthread *thread)
{
    free(thread->stack);
    free(thread);
}

int main()
{
    struct sigaction sa = {0};
    struct itimerval timer;

    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART; // для получение указателя на контекс
    sigaction(SIGALRM, &sa, NULL);

    // Настройка таймера
    timer.it_value.tv_sec = 1; // Первое срабатывание
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1; // Интервал
    timer.it_interval.tv_usec = 0;
    printf("Creating sthread...\n");
    int a, b, c;
    a = 0;
    b = 1;
    c = 2;
    sthread_create(foo, &a);
    sthread_create(foo, &b);
    sthread_create(foo, &c);
    printf("Sthread is created\n");
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer failed");
        return 1;
    }
    while (1)
    {
    }
    /* int z = 0;
    while (1)
    {
        if (head == NULL && z == 0)
        {
            sthread_create(foo, NULL);
            z = 1;
        }
    } */

    return 0;
}

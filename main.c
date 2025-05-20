
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "unistd.h"
#include <stdlib.h>
#include "main.h"
#define STACK_SIZE (1024 * 1024) // 1 MiB
typedef unsigned int uint;
uint id = 0;
struct sthread
{
    void *stack;
    uint stack_size;
    uint id;
    void (*pfunc)(void *);
    void *func_arg;
    sthread *next;
};
sthread *head = NULL; // ПРИОРИТЕТНАЯ ОЧЕРЕДЬ НУЖНА
sthread *tail = NULL;
sthread *cur = NULL;
// Функция, которую нужно вызывать по таймеру
void foo(void *arg) //
{
    int a = 12;
    a += 8;
    printf("N: %d", *(int *)arg + a);
    fflush(stdout);
}
void timer_handler(int signum)
{
    if (head == NULL)
        return;
    if (cur == NULL)
        cur = head;
    void *a = switcher(cur->pfunc, cur->stack, &cur->id);
    printf("f %p", (char *)a);
    sthread *temp = cur->next;
    sthread_free(cur);
    cur = temp;
    head = cur;
}

void sthread_create(void (*pfunc)(void *), void *func_arg)
{
    sthread *thread = malloc(sizeof(sthread));

    thread->stack = (char *)malloc(STACK_SIZE) + STACK_SIZE;
    thread->stack_size = STACK_SIZE;
    thread->id = id++;
    thread->pfunc = pfunc;
    thread->func_arg = func_arg;
    thread->next = NULL;
    if (head == NULL)
    {
        head = tail = thread;
    }
    else
    {
        tail->next = thread;
        tail = thread;
    }
} // Бесконечный цикл ожидания

void sthread_free(sthread *thread)
{
    free(thread->stack - thread->stack_size);
    free(thread);
}

int main()
{
    struct sigaction sa;
    struct itimerval timer;

    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("sigaction failed");
        return 1;
    }

    // Настройка таймера (1 миллисекунда)
    timer.it_value.tv_sec = 1;     // Первое срабатывание
    timer.it_value.tv_usec = 0;    // Через 1 мс
    timer.it_interval.tv_sec = 1;  // Интервал
    timer.it_interval.tv_usec = 0; // Каждую 1 мс
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
    int z = 0;
    while (1)
    {
        if (head == NULL && z == 0)
        {
            sthread_create(foo, NULL);
            z = 1;
        }
    }

    return 0;
}

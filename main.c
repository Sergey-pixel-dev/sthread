
#define _GNU_SOURCE
#include "main.h"
#define STACK_SIZE (1024 * 1024) // 1 MiB
#define MAX_STACK_SAVE (1024 * 64)

uint64_t id = 0;
struct scontext
{
    uint64_t gregs[23]; // все «общие» регистры
    uint64_t rip;       // инструкция
    uint64_t eflags;    // флаги
    void *stack_ptr;    // RSP
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
    sthread_state state;
};

sthread *head = NULL; // ПРИОРИТЕТНАЯ ОЧЕРЕДЬ НУЖНА
sthread *tail = NULL;
sthread *cur = NULL;
char HasStarted = 0;
void foo(void *arg) //
{
    printf("Hello, world, SThread: %d!\n", *(int *)arg);
    while (1)
    {
        /* code */
    }
}

static inline void save_context(scontext *dst, mcontext_t *src)
{
    memcpy(dst->gregs,
           src->gregs,
           sizeof(dst->gregs));

    dst->rip = src->gregs[REG_RIP];
    dst->eflags = src->gregs[REG_EFL];
    dst->stack_ptr = (void *)src->gregs[REG_RSP];
}

static inline void load_context(scontext *src, ucontext_t *dst_uc)
{
    mcontext_t *dst = &dst_uc->uc_mcontext;
    for (int i = 0; i <= REG_RAX; i++) // копируем не все регистры из массива,
        dst->gregs[i] = src->gregs[i]; // так как там есть служебные, и их копирование
    //(и дальнейшее к ним ядром обращение) приводит к segm fault

    dst->gregs[REG_RSP] = src->gregs[REG_RSP];
    dst->gregs[REG_RIP] = src->gregs[REG_RIP];
    dst->gregs[REG_EFL] = src->gregs[REG_EFL];
}
static inline void load_context_with_parametr(scontext *src, ucontext_t *dst_uc)
{
    mcontext_t *dst = &dst_uc->uc_mcontext;
    for (int i = 0; i <= REG_RAX; i++)
        dst->gregs[i] = src->gregs[i];
    dst->gregs[REG_RSP] = src->gregs[REG_RSP];
    dst->gregs[REG_RIP] = src->gregs[REG_RIP];
    dst->gregs[REG_EFL] = src->gregs[REG_EFL];
    dst->gregs[REG_RDI] = (uint64_t)(uintptr_t)cur->func_arg;
}
void timer_handler(int sig, siginfo_t *info, void *ucontext)
{
    if (!head)
        return;
    if (!cur) // будет при первом запуске
    {
        cur = head;
    }
    ucontext_t *uc = (ucontext_t *)ucontext;
    if (HasStarted)
    {
        save_context(&cur->context, &uc->uc_mcontext);
    }
    switch (cur->state)
    {
    case NEW:
        load_context_with_parametr(&cur->next->context, uc);
        cur->state = RUN;
        HasStarted = 1;
        break;
    case RUN:
        load_context(&cur->next->context, uc);
        break;
    case DONE:
        // delete_current_thread();
        /* if (!cur)
            exit(0); */
        break;
    default:
        break;
    }

    cur = cur->next;
}

void sthread_create(void (*pfunc)(void *), void *func_arg)
{
    sthread *thread = calloc(1, sizeof(*thread));

    thread->stack = (char *)malloc(STACK_SIZE);
    thread->stack_size = STACK_SIZE;
    thread->id = id++;
    thread->pfunc = pfunc;
    thread->func_arg = func_arg;
    thread->next = NULL;
    thread->state = NEW;

    thread->context.stack_ptr = thread->stack + STACK_SIZE;
    uint64_t *sp = (uint64_t *)thread->context.stack_ptr;
    *--sp = thread->id;
    *--sp = (uint64_t)(uintptr_t)sthread_exit;
    thread->context.stack_ptr = sp;
    thread->context.gregs[REG_RSP] = (uint64_t)(uintptr_t)sp;

    thread->context.gregs[REG_RIP] = (uint64_t)(uintptr_t)thread->pfunc;
    // thread->context.gregs[REG_EFL] = 0x202;
    if (head == NULL)
    {
        head = tail = thread;
        thread->next = thread;
    }
    else
    {
        tail->next = thread;
        tail = thread;
        tail->next = head;
    }
}

void sthread_exit()
{
    uintptr_t sp;
    __asm__ volatile("movq %%rsp, %0" : "=r"(sp));
    int tid = *(int *)sp;
    printf("Close thread %d\n", tid);

    cur->state = DONE;
    raise(SIGALRM);
    for (;;)
        __asm__ volatile("pause");
}

void delete_current_thread(void)
{
    if (!cur)
        return;
    sthread *to_del = cur;

    if (to_del->next == to_del)
    {
        free(to_del->stack);
        free(to_del);
        head = tail = cur = NULL;
        return;
    }
    sthread *prev = head;
    while (prev->next != to_del)
        prev = prev->next;
    prev->next = to_del->next;
    if (to_del == head)
        head = to_del->next;
    if (to_del == tail)
        tail = prev;

    cur = to_del->next;
    free(to_del->stack);
    free(to_del);
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
    timer.it_interval.tv_sec = 2; // Интервал
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
    setitimer(ITIMER_REAL, &timer, NULL);

    while (1)
    {
    }
    return 0;
}

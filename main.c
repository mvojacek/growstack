#define _GNU_SOURCE
#define __USE_GNU

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#define DEBUG 1

#if DEBUG
#define PRINT_STACKREG(TAG) \
    do {                                                \
        register volatile uintptr_t rsp = 0, rbp = 0;   \
        asm volatile (                                  \
        "mov %[vrsp], rsp \n\t"                         \
        "mov %[vrbp], rbp \n\t"                         \
        :[vrsp] "=r"(rsp),[vrbp] "=r"(rbp)              \
        );                                              \
        printf(" >%s: rsp=%lx rbp=%lx\n", TAG, rsp, rbp);   \
    } while (0)
#else
#define PRINT_STACKREG(...)
#endif

int run_with_stack(void *stackend, void (*func)(void *), void *arg) {
    //static volatile uintptr_t save_rsp, save_rbp;

    typedef uintptr_t save_t[2];

    static volatile save_t *save;
    static volatile size_t save_cnt, save_cap;

    save_cnt++;

#if DEBUG
    printf("adding save entry, to %ld\n", save_cnt);
#endif

    if (save_cnt > save_cap) {
        void *new_mem;
        size_t new_cap;
        if (save == NULL) {
            new_mem = malloc(sizeof(save[0]) * (new_cap = save_cnt));
#if DEBUG
            printf("malloc'd sav: %p of 0x%lx\n", new_mem, sizeof(save[0]) * (new_cap));
#endif
        } else {
            new_mem = realloc((void *) save, sizeof(save[0]) * (new_cap = 2 * save_cap));
#if DEBUG
            printf("realloc'd sav: %p of 0x%lx\n", new_mem, sizeof(save[0]) * (new_cap));
#endif
        }
        if (new_mem == NULL) {
            save_cnt--;
            return 1; // errno was set
        }
        save = new_mem;
        save_cap = new_cap;
    }

    PRINT_STACKREG("run-before");
    asm volatile (
    "sal rbx, 4 \n\t"
    "sub rbx, 16 \n\t"

    "mov r10, %[save] \n\t"
    "add r10, rbx \n\t"

    "mov [r10], rsp \n\t"
    "mov [r10+8], rbp \n\t"

    "mov rsp, %[stackend] \n\t"
    "mov rbp, rsp \n\t"

    "call %[func] \n\t"

    "add rbx, %[save] \n\t"

    "mov rsp, [rbx] \n\t"
    "mov rbp, [rbx+8] \n\t"
    ::[cnt] "b"(save_cnt),[save] "m"(save),[stackend] "rm"(stackend),[func] "r"(func), "D"(arg)
    : "r10"
    );
    PRINT_STACKREG("run-after");

    save_cnt--;

    return 0;
}

bool newstack(size_t quads, uintptr_t **stack, uintptr_t **stackend, size_t *bytes) {
    *bytes = quads * sizeof(uintptr_t);
    *stack = malloc(*bytes);
    if (*stack == NULL) {
        return false;
    }
    *stackend = *stack + quads - 1;
#if DEBUG
    printf("allocated stack: quads=%ld bytes=%ld stack=%p stackend=%p\n", quads, *bytes, *stack, *stackend);
#endif
    return true;
}

#define NEWSTACK(quads, quadsvar, stackvar, stackendvar, bytesvar) \
size_t quadsvar = (quads), bytesvar; \
uintptr_t *stackvar, *stackendvar; \
newstack(quadsvar, &stackvar, &stackendvar, &bytesvar)

#define FREESTACK(stackvar) \
free((void*) (stackvar))

void recursive_func(uint64_t iters) {
    volatile uint64_t arr[64];
    asm volatile(""::"m" (arr));
    if (iters <= 0) {
        printf("%s\n", "Recursion finished!");
        return;
    }
    recursive_func(iters - 1);
}

void dostuff(void *arg) {
    PRINT_STACKREG("dostuff");
    printf("dostuff arg: %s\n", (char *) arg);
    recursive_func(1000000);
}

void reentrant_stacktest(void *arg) {
    uintptr_t iter = (uintptr_t) arg;

#if DEBUG || 1
    printf("%s %ld\n", "reentr: ", iter);
#endif

    if (iter == 0)
        return;
    iter--;

    NEWSTACK(10000, quads, stack, stackend, bytes);

    run_with_stack(stackend, reentrant_stacktest, (void *) iter);

    FREESTACK(stack);
}

void reentrant_infinite(void *arg) {
    NEWSTACK(500, quads, stack, stackend, bytes);
    run_with_stack(stackend, reentrant_infinite, arg);
}

int main(int argc, char **argv) {

    //reentrant_infinite(NULL);

    NEWSTACK(2l * 1024 * 1024 * 1024 / 8, quads, stack, stackend, bytes);

    printf("%s\n", "Running recursive function on allocated stack:");
    PRINT_STACKREG("main-before");
    run_with_stack(stackend, dostuff, "dostuff arg string");
    PRINT_STACKREG("main-after");

    FREESTACK(stack);


    reentrant_stacktest((void *) 10);

    /*
    printf("%s\n", "Running recursive function on system stack:");
    PRINT_STACKREG("main-before");
    dostuff("dostuff arg string");
    PRINT_STACKREG("main-after");
    */
}

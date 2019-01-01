#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
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
    } while (0);
#else
#define PRINT_STACKREG(...)
#endif

void run_with_stack(void *stackend, void (*func)(void *), void *arg) {
    PRINT_STACKREG("run-before")
    static volatile uintptr_t save_rsp, save_rbp;
    asm volatile (
    "mov %[s_rsp], rsp \n\t"
    "mov %[s_rbp], rbp \n\t"

    "mov rsp, %[stackend] \n\t"
    "mov rbp, rsp \n\t"

    "call %[func] \n\t"

    "mov rsp, %[s_rsp] \n\t"
    "mov rbp, %[s_rbp] \n\t"
    :[s_rsp] "+m"(save_rsp),[s_rbp] "+m"(save_rbp)
    :[stackend] "rm"(stackend),[func] "r"(func), "D"(arg)
    );
    PRINT_STACKREG("run-after");
}

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

int main(int argc, char **argv) {

    const size_t stackunit = sizeof(uintptr_t);
    const size_t stackquads = 2l * 1024 * 1024 * 1024 / stackunit;
    const size_t stacksize = stackquads * stackunit;
    uintptr_t *stack = malloc(stacksize);
    if (stack == NULL) {
        perror("malloc failed");
        return 3;
    }
    void *stackend = stack + stackquads - 1;

    printf("allocated stack: uintptr=%ld quads=%lx size=%ld stack=%p stackend=%p\n", sizeof(uintptr_t), stackquads,
           stacksize, stack, stackend);

    printf("%s\n", "Running recursive function on allocated stack:");
    PRINT_STACKREG("main-before")
    run_with_stack(stackend, dostuff, "dostuff arg string");
    PRINT_STACKREG("main-after")

    free(stack);

    printf("%s\n", "Running recursive function on system stack:");
    PRINT_STACKREG("main-before")
    dostuff("dostuff arg string");
    PRINT_STACKREG("main-after")
}

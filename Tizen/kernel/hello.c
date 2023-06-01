#include <linux/kernel.h>
#include <linux/syscalls.h>

static long num_of_sys_call = 0;

SYSCALL_DEFINE0(hello) {
    printk("Hello, this is syscall by Yang Sukhun! (%ld)\n", num_of_sys_call++);
    return 0;
}
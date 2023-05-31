#include <linux/mm.h>
#include <linux/module.h>
#include <linux/nmi.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>
#include <asm/mmu_context.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/completion.h>
#include <linux/kernel_stat.h>
#include <linux/debug_locks.h>
#include <linux/perf_event.h>
#include <linux/security.h>
#include <linux/notifier.h>
#include <linux/profile.h>
#include <linux/freezer.h>
#include <linux/vmalloc.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/pid_namespace.h>
#include <linux/smp.h>
#include <linux/threads.h>
#include <linux/timer.h>
#include <linux/rcupdate.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/percpu.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sysctl.h>
#include <linux/syscalls.h>
#include <linux/times.h>
#include <linux/tsacct_kern.h>
#include <linux/kprobes.h>
#include <linux/delayacct.h>
#include <linux/unistd.h>
#include <linux/pagemap.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/debugfs.h>
#include <linux/ctype.h>
#include <linux/ftrace.h>
#include <linux/slab.h>
#include <linux/init_task.h>
#include <linux/binfmts.h>
#include <linux/context_tracking.h>
#include <asm/switch_to.h>
#include <asm/tlb.h>
#include <asm/irq_regs.h>
#include <asm/paravirt.h>
#include "sched.h"
#include "../workqueue_internal.h"
#include "../smpboot.h"
#include <trace/events/sched.h>


int do_sched_setweight(pid_t pid, int weight) {
	if (weight <= 0 || weight > 20) {
        return -EINVAL;
    }

	struct task_struct* target;
	struct rq *rq;
	struct rq_flags flag;

	if (pid == 0) {
 	    target = current;
    }
    else {
        target = find_task_by_vpid(pid);
    }
    if (target != NULL && target->policy == SCHED_WRR) {
		if (current_uid().val != 0) {
			if (target -> wrr.weight < weight) {
				return -EPERM;
			}
			struct cred* cred = current_cred();
			struct cred* tcred;
			rcu_read_lock();
			tcred = __task_cred(target);
			if (!uid_eq(cred->euid, tcred->euid) && !uid_eq(cred->euid, tcred->uid)) {
				rcu_read_unlock();
				return -EPERM;
			}
			rcu_read_unlock();
		}

		task_rq_lock(target, &flag);
		rq = task_rq(target);
		rq->wrr.sum_weight -= target->wrr.weight;
		rq->wrr.sum_weight += weight;
		target->wrr.weight = weight;
		if(pid != rq->curr->pid) {
			target->wrr.time_slice = __WRR_TIMESLICE * (weight);
		}
		task_rq_unlock(rq, target, &flag);
	}
	return -EINVAL;
}

long do_sched_getweight(pid_t pid) {
    struct task_struct* target = NULL;
    if (pid == 0) {
        target = current;
    }
    else {
        target = find_task_by_vpid(pid);
    }
    if (target != NULL && target->policy == SCHED_WRR) {
        return target->wrr.weight;
    }
    return -EINVAL;
}

SYSCALL_DEFINE2(sched_setweight, pid_t, pid, int, weight)
{
	return do_sched_setweight(pid, weight);
}

SYSCALL_DEFINE1(sched_getweight, pid_t, pid)
{
	return do_sched_getweight(pid);
}

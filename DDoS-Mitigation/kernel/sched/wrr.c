#include "sched.h"

#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/list.h>
#include <linux/types.h>




void trigger_load_balance_wrr(struct rq *rq)  {
    int number;
	int val = cpumask_first(cpu_active_mask);
    
	for_each_possible_cpu(number) {
		if (cpumask_test_cpu(number, cpu_active_mask)) {
			val = number;
		}
	}

    struct wrr_rq *wrr_rq = &cpu_rq(val)->wrr;

	if(val == smp_processor_id()) {
		(wrr_rq -> counter)++;
		if (wrr_rq -> counter >= __WRR_BALANCE_TICKS) {
			wrr_rq -> counter = 0;

            int max_weight = -1, min_weight = 2000000000, max_task_weight = -1;
            int weight, number, i;
            int val = cpumask_first(cpu_active_mask);
            unsigned long flags;

            for_each_possible_cpu(number) {
                if (cpumask_test_cpu(number, cpu_active_mask)) {
                    val = number;
                }
	        }

            struct rq *rq_master = cpu_rq(val);
            struct rq *rq_max = rq_master, *rq_min = rq_master;
            struct task_struct *task;
            struct list_head* head_node;
            struct task_struct *task_target = NULL;
            struct sched_wrr_entity *task_wrr = NULL;
            
	        rcu_read_lock();
            for_each_possible_cpu(i) {
                struct rq *rq = cpu_rq(i);

                int val = cpumask_first(cpu_active_mask);
                for_each_possible_cpu(number) {
                    if (cpumask_test_cpu(number, cpu_active_mask)) {
                        val = number;
                    }
                }


                if (val != i && (cpumask_test_cpu(i, cpu_active_mask))) {
                    long total_weight = (rq -> wrr).sum_weight * __WRR_TIMESLICE + (rq -> wrr).wrr_nr_running * __WRR_KERNEL_OVERHEAD_PER_TASK;

                    
                    
                    if(min_weight > total_weight) {
                        min_weight = total_weight;
                        rq_min = rq;
                    }
                    if(max_weight < total_weight) {
                        max_weight = total_weight;
                        rq_max = rq;
                    }
                }
            }
            rcu_read_unlock();

            if(rq_master->wrr.sum_weight) {
                max_weight = rq_master->wrr.sum_weight;
                rq_max = rq_master;
            }

            if((rq_max != rq_master && rq_max->wrr.wrr_nr_running < 2) || rq_min == rq_master || rq_min == rq_max) {
                return;
            }

            local_irq_save(flags);
            double_rq_lock(rq_max, rq_min);

            list_for_each(head_node, &(rq_max -> wrr.head)) {
                task_wrr = list_entry(head_node, struct sched_wrr_entity, que_node);
                task = container_of(task_wrr, struct task_struct, wrr);
                weight = (task_wrr -> weight) * __WRR_TIMESLICE;
                
                int val = cpumask_first(cpu_active_mask);
                for_each_possible_cpu(number) {
                    if (cpumask_test_cpu(number, cpu_active_mask)) {
                        val = number;
                    }
                }


                if ((task != rq_max->curr) && cpumask_test_cpu(cpu_of(rq_min), cpu_active_mask) && (cpu_of(rq_min) != val)  && 
                        (cpumask_test_cpu(cpu_of(rq_min), &(task -> cpus_allowed)) && weight > max_task_weight)) {
                    
                    if (rq_max == rq_master || (max_weight - weight - __WRR_KERNEL_OVERHEAD_PER_TASK) > (min_weight + weight + __WRR_KERNEL_OVERHEAD_PER_TASK)) {
                        task_target = task;
                        max_task_weight = weight;
                    }
                }
            }

            if (task_target != NULL) {
                deactivate_task(rq_max, task_target, 0);
                set_task_cpu(task_target, cpu_of(rq_min));
                activate_task(rq_min, task_target, 0);
                if(rq_max == rq_master) {
                    (task_target -> wrr).time_slice = __WRR_TIMESLICE * (task_target -> wrr.weight);
                }
            }
	        double_rq_unlock(rq_max, rq_min);
	        local_irq_restore(flags);

            printk(KERN_DEBUG "[WRR LOAD BALANCING] jiffies: %Ld\n"
                  "[WRR LOAD BALANCING] max_cpu: %d, total weight: %u\n"
                  "[WRR LOAD BALANCING] min_cpu: %d, total weight: %u\n"
                  "[WRR LOAD BALANCING] migrated task name: %s, task weight: %u\n", (long long)(jiffies), rq_max -> wrr.CPUID, rq_max -> wrr.sum_weight, rq_min -> wrr.CPUID, rq_min -> wrr.sum_weight, task_target -> wrr.pid, task_target -> wrr.weight);
		}
	}
}

void init_wrr_rq(struct wrr_rq *wrr_rq, int CPUID) {
    wrr_rq -> wrr_nr_running = 0;
	wrr_rq -> sum_weight = 0;
	wrr_rq -> counter = 0;
	wrr_rq -> CPUID = CPUID;
	INIT_LIST_HEAD(&(wrr_rq -> head));
}

__init void init_sched_wrr_class(void) {
    return;
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags) {
	struct wrr_rq *wrr_rq = &(rq -> wrr);
	struct sched_wrr_entity *wrr = &(p -> wrr), *first_wrr;

	list_add_tail(&(wrr -> que_node), &(wrr_rq -> head));

    int number;
	int val = cpumask_first(cpu_active_mask);
	for_each_possible_cpu(number) {
		if (cpumask_test_cpu(number, cpu_active_mask)) {
			val = number;
		}
	}

	if (cpu_of(rq) != val) {
		wrr -> pid = (int) (p -> pid);
		wrr_rq -> sum_weight += wrr -> weight;
        (wrr_rq -> wrr_nr_running)++;
		add_nr_running(rq, 1);
		first_wrr = list_first_entry_or_null(&(wrr_rq -> head), struct sched_wrr_entity, que_node);

        if(first_wrr != NULL && first_wrr != wrr) {
            wrr -> previous_start_time = first_wrr -> previous_start_time;
        } else {
            wrr -> previous_start_time = jiffies;
        }
        wrr -> time_slice = __WRR_TIMESLICE * (wrr -> weight);
		wrr -> time_interval = 0;
	} else {
		wrr -> pid = (int) (p -> pid);
        wrr -> time_slice = 0;
		(wrr_rq -> wrr_nr_running)++;
		wrr_rq -> sum_weight += wrr -> weight;
	}
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags) {
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr = &p->wrr;

    int number;
	int val = cpumask_first(cpu_active_mask);
	for_each_possible_cpu(number) {
		if (cpumask_test_cpu(number, cpu_active_mask)) {
			val = number;
		}
	}

    int dec_weight = wrr -> weight;
	if (cpu_of(rq) == val) {
        list_del(&(wrr -> que_node));
		(wrr_rq -> wrr_nr_running)--;
		wrr_rq -> sum_weight -= dec_weight;
	} else {
		list_del(&(wrr->que_node));
        wrr_rq -> sum_weight -= dec_weight;
		(wrr_rq -> wrr_nr_running)--;
		sub_nr_running(rq, 1);
	}

}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags) {
	return;
}

static void yield_task_wrr(struct rq *rq) {
	struct task_struct *curr = (rq -> curr);
    struct sched_wrr_entity *wrr = &(curr -> wrr);
	struct wrr_rq *wrr_rq = &(rq -> wrr);

	list_move_tail(&(wrr -> que_node), &(wrr_rq -> head));
}

static bool yield_to_task_wrr(struct rq *rq, struct task_struct *p, bool preempt) {
    return false;
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf) {
	struct wrr_rq *wrr_rq = &(rq -> wrr);
	struct sched_wrr_entity *next_wrr = list_first_entry_or_null(&(wrr_rq -> head), struct sched_wrr_entity, que_node);
	struct task_struct *task;

    if(!next_wrr) {
        return NULL;
    }

    int number;
	int val = cpumask_first(cpu_active_mask);
	for_each_possible_cpu(number) {
		if (cpumask_test_cpu(number, cpu_active_mask)) {
			val = number;
		}
	}

	if(val == cpu_of(rq)) {
		return NULL;
	}

	next_wrr -> time_slice = __WRR_TIMESLICE * (next_wrr -> weight);

	long come_back_Time = jiffies;
	task = container_of(next_wrr, struct task_struct, wrr);
	task -> wrr.time_interval = come_back_Time - (task -> wrr).previous_start_time;
 	(task -> wrr).previous_start_time = jiffies;

	return task;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev) {
    return;
}


static int select_task_rq_wrr(struct task_struct *p, int task_cpu, int sd_flag, int flags) {
	int i;
	int min_weight = 2000000000;

	int target_cpu = cpumask_first(cpu_active_mask);
	for_each_possible_cpu(i) {
		if (cpumask_test_cpu(i, cpu_active_mask)) {
			target_cpu = i;
		}
	}

	rcu_read_lock();
	for_each_possible_cpu(i) {
		struct rq *rq = cpu_rq(i);
		struct wrr_rq *wrr_rq = &(rq -> wrr);
		long weight = (wrr_rq -> sum_weight) *__WRR_TIMESLICE + (wrr_rq -> wrr_nr_running) * __WRR_KERNEL_OVERHEAD_PER_TASK;


        int val = cpumask_first(cpu_active_mask);
        int number;
        for_each_possible_cpu(number) {
            if (cpumask_test_cpu(number, cpu_active_mask)) {
                val = number;
            }
        }

		if ((val != i) && (cpumask_test_cpu(i, cpu_active_mask)) && (cpumask_test_cpu(i, &(p -> cpus_allowed))) &&  weight < min_weight) {
			target_cpu = i;
			min_weight = weight;
		}
	}
	rcu_read_unlock();
	return target_cpu;
}

static void migrate_task_rq_wrr(struct task_struct *p) {
    return; 
}

static void task_woken_wrr(struct rq *this_rq, struct task_struct *task) {
    return;
}

static void set_cpus_allowed_wrr(struct task_struct *p, const struct cpumask *newmask) {
    return;
}

static void rq_online_wrr(struct rq *rq) {
    return;
}

static void rq_offline_wrr(struct rq *rq) {
    return;
}


static void set_curr_task_wrr(struct rq *rq) {
    return;
}

static void task_tick_wrr(struct rq *rq, struct task_struct *curr, int queued) {
    struct sched_wrr_entity *wrr = &(curr -> wrr);
	struct wrr_rq *wrr_rq = &(rq -> wrr);
	
    if(!(wrr -> time_slice)) {
        int number;
        int val = cpumask_first(cpu_active_mask);
        for_each_possible_cpu(number) {
            if (cpumask_test_cpu(number, cpu_active_mask)) {
                val = number;
            }
        }

		if (cpu_of(rq) == val) {
            wrr -> time_slice = 0;
			(cpu_rq(val) -> wrr).counter = __WRR_BALANCE_TICKS;
		} else {
			wrr -> time_slice = __WRR_TIMESLICE * (wrr -> weight);
		}

        if(wrr_rq -> wrr_nr_running <= 1) {
            ;
        } else {
            list_move_tail(&(wrr -> que_node), &(wrr_rq -> head));
        }

		resched_curr(rq);
    } else {
        (wrr -> time_slice)--;
    }
}

static void task_fork_wrr(struct task_struct *p) {
	(p -> wrr).weight = (current -> wrr).weight;
}

static void task_dead_wrr(struct task_struct *p) {
    return;
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio) {
    return;
}

static void switched_from_wrr(struct rq *rq, struct task_struct *p) {
    return;
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p) {
    return;
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task) {
    return 0;
}

static void update_curr_wrr(struct rq *rq) {
    return;
}


const struct sched_class wrr_sched_class = 
{
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,
	.yield_to_task		= yield_to_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,
//	.migrate_task_rq	= migrate_task_rq_wrr,

    .task_woken = task_woken_wrr,
    .set_cpus_allowed	= set_cpus_allowed_wrr,

	.rq_online		= rq_online_wrr,
	.rq_offline		= rq_offline_wrr,
#endif
	.set_curr_task          = set_curr_task_wrr,
	.task_tick		= task_tick_wrr,
	.task_fork		= task_fork_wrr,
    .task_dead		= task_dead_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_from		= switched_from_wrr,
	.switched_to		= switched_to_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.update_curr		= update_curr_wrr,
};

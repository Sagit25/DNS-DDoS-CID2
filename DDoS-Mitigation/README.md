[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-8d59dc4de5201274e310e4c54b9627a8934c3b88527886e3b421487c677d23eb.svg)](https://classroom.github.com/a/B0xXPUw_)

# Project 3 : Hello, Scheduler!
## (Team 17 : 양석훈, 인재원, 전진용)

We implement a weighted round-robin (WRR) scheduler,
with two system calls number 294~295.\
(check kernel/sched/weight.c)

### syscall#294(sched_setweight)
example below set weight 15 to task which pid is 20

	syscall(294, 20, 15)


### syscall#295(sched_getweight) 
example below get weight to task which pid is 30

	int id = syscall(295, 30)

## Test

You can test the syscall we implemented with the codes in test/ directory.\
We implement three testcases

1) infloop.c: infinite loop scheduling case
2) fork.c: improve fork() testcase
3) prime.c: calculate prime factorization, used in demo

Just 'make' in test folder and re-boot QEMU, then you can test our scenario.

Run './infloop', './fork', './prime'.

## Plot

![그림1](https://github.com/swsnu/project-3-hello-scheduler-team-17/assets/69968627/c13e5059-c7fd-4189-82f6-726e9b0c1afa)

x-axis: weight, y-axis: sec \
We use WRR scheduling, so result is like above image

## Breif explanation about implementations

Basic structure of WRR scheduler (wrr.c) is come from RT scheduler (rt.c). \
Because, basic behavior is almost same. \
Then, we changed initialize and priority code. (I think this is almost same for everyone so skip detail) \

We use some locks in linux kernel to solve concurrency issues.

1) read_rcu_lock() / read_rcu_unlock(): find sum of weight for each cpu, for concurrency
2) task_rq_lock() / task_rq_unlock(): for setweight for task
3) double_rq_lock() / double_rq_unlock(): for transfer task between min cpu and max cpu

### Demo & Slide
Demo: https://youtu.be/kyd9jv5Tg18

Slide: https://docs.google.com/presentation/d/1YUZm-eSYCeP64mz4KHmVgjA2Clfja550lc0MsQBfvo8/edit?usp=share_link

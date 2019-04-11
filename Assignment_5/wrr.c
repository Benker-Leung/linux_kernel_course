/* Location: kernel/sched/wrr.c */

/* include necessary header files */
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include "sched.h"

#define WRR_TIMESLICE (100 * HZ / 1000)


/* implementation of init_wrr_rq */
void init_wrr_rq(struct wrr_rq *rr_rq)
{
	INIT_LIST_HEAD(&rr_rq->queue);
}


/* implementation of the weight round-robin scheduler functions (i.e. XXX_wrr functions */
static void
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	list_add_tail(&wrr_se->list, &rq->wrr.queue);

	printk(KERN_INFO"[SCHED_WRR] ENQUEUE: p->pid=%d, p->policy=%d "
		"curr->pid=%d, curr->policy=%d, flags=%d\n",
		p->pid, p->policy, rq->curr->pid, rq->curr->policy, flags);

}

static void
dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	list_del(&wrr_se->list);

	printk(KERN_INFO"[SCHED_WRR] DEQUEUE: p->pid=%d, p->policy=%d "
		"curr->pid=%d, curr->policy=%d, flags=%d\n",
		p->pid, p->policy, rq->curr->pid, rq->curr->policy, flags);

}

static void yield_task_wrr(struct rq *rq)
{
	struct sched_wrr_entity *wrr_se = &rq->curr->wrr;
	struct wrr_rq *wrr_rq = &rq->wrr;

	// yield the current task, put it to the end of the queue
	list_move_tail(&wrr_se->list, &wrr_rq->queue);

	printk(KERN_INFO"[SCHED_WRR] YIELD: Process-%d\n", rq->curr->pid);
}

static void
check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	return;
}

static struct task_struct *
pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct sched_wrr_entity *wrr_se = NULL;
	struct task_struct *p = NULL;
	struct wrr_rq *wrr_rq = &rq->wrr;
	if (list_empty(&wrr_rq->queue)) {
		return NULL;
	}
	put_prev_task(rq, prev);
	wrr_se = list_entry(wrr_rq->queue.next,
				struct sched_wrr_entity,
				list);
	p = container_of(wrr_se, struct task_struct, wrr);
	return p;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{

}

static void set_curr_task_wrr(struct rq *rq)
{

}

/* This function need to be modified */
static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	if (p->policy != SCHED_WRR)
		return ;

	printk(KERN_INFO"[SCHED_WRR] Task Tick: Process-%d = %d\n", p->pid, p->wrr.time_slice);

	if(--p->wrr.time_slice)
		return;

	p->wrr.time_slice = WRR_TIMESLICE * p->wrr.wrr_weight;
	list_move_tail(&p->wrr.list, &rq->wrr.queue);
	set_tsk_need_resched(p);

}

/* This function need to be modified */
unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
	return p->wrr.time_slice;
}

static void
prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
	return;
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{

}

static void update_curr_wrr(struct rq *rq)
{

}


/* WRR scheduler class definition */
const struct sched_class wrr_sched_class = {
	.next = &fair_sched_class,
	.enqueue_task = enqueue_task_wrr,
	.dequeue_task = dequeue_task_wrr,
	.yield_task = yield_task_wrr,
	.check_preempt_curr = check_preempt_curr_wrr,
	.pick_next_task = pick_next_task_wrr,
	.put_prev_task = put_prev_task_wrr,
	.set_curr_task = set_curr_task_wrr,
	.task_tick = task_tick_wrr,
	.get_rr_interval = get_rr_interval_wrr,
	.prio_changed = prio_changed_wrr,
	.switched_to = switched_to_wrr,
	.update_curr = update_curr_wrr,
};

SYSCALL_DEFINE2(set_wrr_weight, pid_t, pid, int, weight)
{
	struct task_struct *p;
	p = find_task_by_vpid(pid);
	if (!p) {
		printk(KERN_INFO "pid_t [%d] not exist\n", pid);
		return -3;
	}
	p->wrr.wrr_weight = weight;
	p->wrr.time_slice = weight * WRR_TIMESLICE;
	return 0;
}

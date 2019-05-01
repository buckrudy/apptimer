#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>

#include "list.h"
#include "apptimer.h"

#define DEBUG_PP(...)	printf(__VA_ARGS__)
//#define DEBUG_PP(...)

struct apptimer {
	struct list_head node;
	APPTIMER_ID_T tid;
	APPTIMER_EXPIRE_T expire;
	APPTIMER_EXPIRE_T _expire;
	int repeat;
	void (*proc)(APPTIMER_ID_T tid, void *user_data);
	void *user_data;
};

static struct list_head _s_timer_list;
static APPTIMER_ID_T _s_timer_id = 0;
static pthread_mutex_t _s_timer_lock;
static pthread_mutexattr_t _s_timer_mutex_attr;
//static pthread_cond_t _s_timer_condition = PTHREAD_COND_INITIALIZER;
static pthread_t _s_timer_tid;
static int _s_timer_running = 0;

static void *_timer_thread(void *arg)
{
	struct apptimer *t, *tmp;
	struct timeval tv;
	APPTIMER_ID_T tid;
	int repeat;

	pthread_detach(_s_timer_tid);

	do {
		tv.tv_sec = 0;
		tv.tv_usec = APPTIMER_GRANULARITY; //100 milliseconds

		select(0, NULL, NULL, NULL, &tv);

		pthread_mutex_lock(&_s_timer_lock);
		if (!list_empty(&_s_timer_list)) {
			list_for_each_entry_safe(t, tmp, &_s_timer_list, node) {
				t->expire -= APPTIMER_GRANULARITY;
				if (t->expire == 0) {
					t->expire = t->_expire;
					tid = t->tid;
					repeat = t->repeat;
					
					if (t->proc)
						t->proc(tid, t->user_data);

					/* 如果定时器在proc中删除自己，这里就会段错误，所以不能这样实现
					if (t->repeat == APP_TIMER_NOREPEAT) {
						list_del(&t->node);
						free(t);
					}
					*/
					
					//这样就不会出现上面的问题
					if (repeat == APPTIMER_NOREPEAT)
						apptimer_deltimer(tid);
				}
			}
		}
		pthread_mutex_unlock(&_s_timer_lock);

		if (_s_timer_running == 0) {
			break;
		}

	} while (1);

	pthread_mutex_lock(&_s_timer_lock);
	list_for_each_entry_safe(t, tmp, &_s_timer_list, node) {
		list_del(&t->node);
		free(t);
	}
	pthread_mutex_unlock(&_s_timer_lock);

	pthread_mutex_destroy(&_s_timer_lock);
	pthread_mutexattr_destroy(&_s_timer_mutex_attr);
	return NULL;
}

int apptimer_init(void)
{
	if (_s_timer_running == 1) //already init
		return 1;

	_s_timer_running = 1;
	_s_timer_id = 0;
	INIT_LIST_HEAD(&_s_timer_list);
	pthread_mutexattr_init(&_s_timer_mutex_attr);
	pthread_mutexattr_settype(&_s_timer_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_s_timer_lock, &_s_timer_mutex_attr);
	pthread_create(&_s_timer_tid, NULL, _timer_thread, NULL);
	return 0;
}

int apptimer_exit(void)
{
	_s_timer_running = 0;
	return 0;
}

int apptimer_addtimer(APPTIMER_ID_T *tid, APPTIMER_EXPIRE_T expire,
		int repeat, APPTIMER_FUNC_T func, void *user_data)
{
	struct apptimer *t;

	if (repeat != APPTIMER_NOREPEAT && repeat != APPTIMER_REPEAT)
		return 1;
	
	t = calloc(1, sizeof(*t));
	if (!t)
		return 2;


	t->expire = (expire / APPTIMER_GRANULARITY) * APPTIMER_GRANULARITY;
	if (t->expire == 0) {	//不可为0
		free(t);
		return 3;
	}

	t->_expire = t->expire;
	t->repeat = repeat;
	t->proc = func;
	t->user_data = user_data;


	pthread_mutex_lock(&_s_timer_lock);

	_s_timer_id ++;
	t->tid = _s_timer_id;
	if (tid) *tid = _s_timer_id;

	list_add_tail(&t->node, &_s_timer_list);

	pthread_mutex_unlock(&_s_timer_lock);
	return 0;
}

int apptimer_deltimer(APPTIMER_ID_T tid)
{
	struct apptimer *t, *tmp;
	pthread_mutex_lock(&_s_timer_lock);
	list_for_each_entry_safe(t, tmp, &_s_timer_list, node) {
		if (t->tid == tid) {
			list_del(&t->node);
			free(t);
		}
	}
	pthread_mutex_unlock(&_s_timer_lock);
	return 0;
}

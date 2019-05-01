#include <stdio.h>
#include <unistd.h>
#include "apptimer.h"

#define PP(...) printf(__VA_ARGS__)

void xxx(APPTIMER_ID_T tid, void *user_data)
{
	PP("tid = %d, user_data = %s\n", tid, (char *)user_data);
}

void xx(APPTIMER_ID_T tid, void *user_data)
{
	PP("tid = %d, user_data = %s\n", tid, (char *)user_data);
	apptimer_addtimer(NULL, 20 * APPTIMER_GRANULARITY, APPTIMER_REPEAT, xxx, "I'am xxx");
	apptimer_deltimer(tid);
}

void x(APPTIMER_ID_T tid, void *user_data)
{
	PP("tid = %d, user_data = %s\n", tid, (char*)user_data);
}


int main(void)
{
	int ret;
	ret = apptimer_init();
	if (ret != 0) {
		PP("apptimer_init() error\n");
		return 1;
	}

	ret = apptimer_addtimer(NULL, 10 * APPTIMER_GRANULARITY, APPTIMER_REPEAT, xx, "googol");
	if (ret != 0) {
		PP("apptimer_addtimer() error\n");
	}

	ret = apptimer_addtimer(NULL, 50 * APPTIMER_GRANULARITY, APPTIMER_REPEAT, x, "hello");

	do {
		sleep(10);
	} while (1);

	return 0;
}

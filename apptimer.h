#ifndef __APPTIMTER_H__
#define __APPTIMTER_H__
#ifdef __cplusplus
extern "C" {
#endif

//100 milliseconds
#define APPTIMER_GRANULARITY	(100*1000U)
#define APPTIMER_REPEAT	1
#define APPTIMER_NOREPEAT	0

typedef unsigned int APPTIMER_ID_T;
typedef unsigned int APPTIMER_EXPIRE_T;
typedef void (APPTIMER_FUNC_T)(APPTIMER_ID_T tid, void *user_data);

//return zero success, other failed
int apptimer_init(void);
//return zero success, other failed
int apptimer_exit(void);
//return zero success, other failed
int apptimer_addtimer(APPTIMER_ID_T *tid, APPTIMER_EXPIRE_T expire,
		int repeat, APPTIMER_FUNC_T func, void *user_data);
//return zero success, other failed
int apptimer_deltimer(APPTIMER_ID_T tid);

#ifdef __cplusplus
}
#endif
#endif

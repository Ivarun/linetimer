/*
 * iun_time.h: header-only time-related functions.
 */

#ifndef IUN_TIME_INCLUDED
#define IUN_TIME_INCLUDED

#include <time.h>
#include <string.h>
#include <errno.h>

/*
 * Simple timer function returning a timespec using the best available timer.
 * Exit on error if err is NULL, else return the error code from clock_gettime
 * in err.
 */
struct timespec iun_timer(int *err_arg)
{
	int err;
	struct timespec ts;
	clockid_t clock_id;

	#ifdef CLOCK_MONOTONIC
	clock_id = CLOCK_MONOTONIC;
	#else
	#warning "CLOCK_MONOTONIC not supported, using CLOCK_REALTIME instead"
	clock_id = CLOCK_REALTIME;
	#endif

	err = clock_gettime(clock_id, &ts);
	if (err && NULL == err_arg)
		iun_err("clock_gettime failed with: %s\n", strerror(errno));
	else if (NULL != err_arg)
		*err_arg = err;
	return ts;
}

/*
 * Sleep until t_ref would be a multiple of dt. If err is NULL exit with an
 * error message on errors, else return the error code from nanosleep in err.
 */
void iun_sleepmult(struct timespec t_ref, struct timespec dt, int *err_arg)
{
	struct timespec slpt = {.tv_sec = 0, .tv_nsec = 0};
	if (0 != dt.tv_sec)
		slpt.tv_sec = dt.tv_sec - t_ref.tv_sec % dt.tv_sec;
	if (0 != dt.tv_nsec)
		slpt.tv_nsec = dt.tv_nsec - t_ref.tv_nsec % dt.tv_nsec;
	if (slpt.tv_nsec < 0) {
		slpt.tv_nsec += 1000000000L;
		slpt.tv_sec -= 1;
	}
	int err = nanosleep(&slpt, NULL);
	if (err && NULL == err_arg)
		iun_err("nanosleep failed with: %s\n", strerror(errno));
	else if (NULL != err_arg)
		*err_arg = err;
}

/*
 * Return the time from start to stop.
 */
struct timespec iun_dt(struct timespec start, struct timespec stop)
{
	struct timespec dt;
	dt.tv_sec  = stop.tv_sec  - start.tv_sec;
	dt.tv_nsec = stop.tv_nsec - start.tv_nsec;
	if (dt.tv_nsec < 0) {
		dt.tv_nsec += 1000000000L;
		dt.tv_sec -= 1;
	}
	return dt;
}

#endif // IUN_TIME_INCLUDED


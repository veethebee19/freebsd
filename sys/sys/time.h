/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)time.h	8.5 (Berkeley) 5/4/95
 * $FreeBSD$
 */

#ifndef _SYS_TIME_H_
#define	_SYS_TIME_H_

#include <sys/_timeval.h>
#include <sys/types.h>
#include <sys/timespec.h>

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};
#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */

#if __BSD_VISIBLE
struct bintime {
	time_t	sec;
	uint64_t frac;
};

static __inline void
bintime_addx(struct bintime *_bt, uint64_t _x)
{
	uint64_t _u;

	_u = _bt->frac;
	_bt->frac += _x;
	if (_u > _bt->frac)
		_bt->sec++;
}

static __inline void
bintime_add(struct bintime *_bt, const struct bintime *_bt2)
{
	uint64_t _u;

	_u = _bt->frac;
	_bt->frac += _bt2->frac;
	if (_u > _bt->frac)
		_bt->sec++;
	_bt->sec += _bt2->sec;
}

static __inline void
bintime_sub(struct bintime *_bt, const struct bintime *_bt2)
{
	uint64_t _u;

	_u = _bt->frac;
	_bt->frac -= _bt2->frac;
	if (_u < _bt->frac)
		_bt->sec--;
	_bt->sec -= _bt2->sec;
}

static __inline void
bintime_mul(struct bintime *_bt, u_int _x)
{
	uint64_t _p1, _p2;

	_p1 = (_bt->frac & 0xffffffffull) * _x;
	_p2 = (_bt->frac >> 32) * _x + (_p1 >> 32);
	_bt->sec *= _x;
	_bt->sec += (_p2 >> 32);
	_bt->frac = (_p2 << 32) | (_p1 & 0xffffffffull);
}

static __inline void
bintime_shift(struct bintime *_bt, int _exp)
{

	if (_exp > 0) {
		_bt->sec <<= _exp;
		_bt->sec |= _bt->frac >> (64 - _exp);
		_bt->frac <<= _exp;
	} else if (_exp < 0) {
		_bt->frac >>= -_exp;
		_bt->frac |= (uint64_t)_bt->sec << (64 + _exp);
		_bt->sec >>= -_exp;
	}
}

#define	bintime_clear(a)	((a)->sec = (a)->frac = 0)
#define	bintime_isset(a)	((a)->sec || (a)->frac)
#define	bintime_cmp(a, b, cmp)						\
	(((a)->sec == (b)->sec) ?					\
	    ((a)->frac cmp (b)->frac) :					\
	    ((a)->sec cmp (b)->sec))

#define	SBT_1S	((sbintime_t)1 << 32)
#define	SBT_1M	(SBT_1S * 60)
#define	SBT_1MS	(SBT_1S / 1000)
#define	SBT_1US	(SBT_1S / 1000000)
#define	SBT_1NS	(SBT_1S / 1000000000) /* beware rounding, see nstosbt() */
#define	SBT_MAX	0x7fffffffffffffffLL

static __inline int
sbintime_getsec(sbintime_t _sbt)
{

	return (_sbt >> 32);
}

static __inline sbintime_t
bttosbt(const struct bintime _bt)
{

	return (((sbintime_t)_bt.sec << 32) + (_bt.frac >> 32));
}

static __inline struct bintime
sbttobt(sbintime_t _sbt)
{
	struct bintime _bt;

	_bt.sec = _sbt >> 32;
	_bt.frac = _sbt << 32;
	return (_bt);
}

/*
 * Decimal<->sbt conversions.  Multiplying or dividing by SBT_1NS results in
 * large roundoff errors which sbttons() and nstosbt() avoid.  Millisecond and
 * microsecond functions are also provided for completeness.
 */
static __inline int64_t
sbttons(sbintime_t _sbt)
{

	return ((1000000000 * _sbt) >> 32);
}

static __inline sbintime_t
nstosbt(int64_t _ns)
{

	return ((_ns * (((uint64_t)1 << 63) / 500000000)) >> 32);
}

static __inline int64_t
sbttous(sbintime_t _sbt)
{

	return ((1000000 * _sbt) >> 32);
}

static __inline sbintime_t
ustosbt(int64_t _us)
{

	return ((_us * (((uint64_t)1 << 63) / 500000)) >> 32);
}

static __inline int64_t
sbttoms(sbintime_t _sbt)
{

	return ((1000 * _sbt) >> 32);
}

static __inline sbintime_t
mstosbt(int64_t _ms)
{

	return ((_ms * (((uint64_t)1 << 63) / 500)) >> 32);
}

/*-
 * Background information:
 *
 * When converting between timestamps on parallel timescales of differing
 * resolutions it is historical and scientific practice to round down rather
 * than doing 4/5 rounding.
 *
 *   The date changes at midnight, not at noon.
 *
 *   Even at 15:59:59.999999999 it's not four'o'clock.
 *
 *   time_second ticks after N.999999999 not after N.4999999999
 */

static __inline void
bintime2timespec(const struct bintime *_bt, struct timespec *_ts)
{

	_ts->tv_sec = _bt->sec;
	_ts->tv_nsec = ((uint64_t)1000000000 *
	    (uint32_t)(_bt->frac >> 32)) >> 32;
}

static __inline void
timespec2bintime(const struct timespec *_ts, struct bintime *_bt)
{

	_bt->sec = _ts->tv_sec;
	/* 18446744073 = int(2^64 / 1000000000) */
	_bt->frac = _ts->tv_nsec * (uint64_t)18446744073LL;
}

static __inline void
bintime2timeval(const struct bintime *_bt, struct timeval *_tv)
{

	_tv->tv_sec = _bt->sec;
	_tv->tv_usec = ((uint64_t)1000000 * (uint32_t)(_bt->frac >> 32)) >> 32;
}

static __inline void
timeval2bintime(const struct timeval *_tv, struct bintime *_bt)
{

	_bt->sec = _tv->tv_sec;
	/* 18446744073709 = int(2^64 / 1000000) */
	_bt->frac = _tv->tv_usec * (uint64_t)18446744073709LL;
}

static __inline struct timespec
sbttots(sbintime_t _sbt)
{
	struct timespec _ts;

	_ts.tv_sec = _sbt >> 32;
	_ts.tv_nsec = sbttons((uint32_t)_sbt);
	return (_ts);
}

static __inline sbintime_t
tstosbt(struct timespec _ts)
{

	return (((sbintime_t)_ts.tv_sec << 32) + nstosbt(_ts.tv_nsec));
}

static __inline struct timeval
sbttotv(sbintime_t _sbt)
{
	struct timeval _tv;

	_tv.tv_sec = _sbt >> 32;
	_tv.tv_usec = sbttous((uint32_t)_sbt);
	return (_tv);
}

static __inline sbintime_t
tvtosbt(struct timeval _tv)
{

	return (((sbintime_t)_tv.tv_sec << 32) + ustosbt(_tv.tv_usec));
}
#endif /* __BSD_VISIBLE */

#if defined(_KERNEL) || defined(_KERNEL_UT)

/* Operations on timespecs */
#define	timespecclear(tvp)	((tvp)->tv_sec = (tvp)->tv_nsec = 0)
#define	timespecisset(tvp)	((tvp)->tv_sec || (tvp)->tv_nsec)
#define	timespeccmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))
#define	timespecadd(vvp, uvp)						\
	do {								\
		(vvp)->tv_sec += (uvp)->tv_sec;				\
		(vvp)->tv_nsec += (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec >= 1000000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_nsec -= 1000000000;			\
		}							\
	} while (0)
#define	timespecsub(vvp, uvp)						\
	do {								\
		(vvp)->tv_sec -= (uvp)->tv_sec;				\
		(vvp)->tv_nsec -= (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_nsec += 1000000000;			\
		}							\
	} while (0)

/* Operations on timevals. */

#define	timevalclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define	timevalisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timevalcmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_usec cmp (uvp)->tv_usec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))

/* timevaladd and timevalsub are not inlined */

#endif /* _KERNEL */

#ifndef _KERNEL			/* NetBSD/OpenBSD compatible interfaces */

#define	timerclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_usec cmp (uvp)->tv_usec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))
#define	timeradd(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;	\
		if ((vvp)->tv_usec >= 1000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_usec -= 1000000;			\
		}							\
	} while (0)
#define	timersub(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	\
		if ((vvp)->tv_usec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_usec += 1000000;			\
		}							\
	} while (0)
#endif

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

struct itimerval {
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};

/*
 * Getkerninfo clock information structure
 */
struct clockinfo {
	int	hz;		/* clock frequency */
	int	tick;		/* micro-seconds per hz tick */
	int	spare;
	int	stathz;		/* statistics clock frequency */
	int	profhz;		/* profiling clock frequency */
};

/* These macros are also in time.h. */
#ifndef CLOCK_REALTIME
#define	CLOCK_REALTIME	0
#define	CLOCK_VIRTUAL	1
#define	CLOCK_PROF	2
#define	CLOCK_MONOTONIC	4
#define	CLOCK_UPTIME	5		/* FreeBSD-specific. */
#define	CLOCK_UPTIME_PRECISE	7	/* FreeBSD-specific. */
#define	CLOCK_UPTIME_FAST	8	/* FreeBSD-specific. */
#define	CLOCK_REALTIME_PRECISE	9	/* FreeBSD-specific. */
#define	CLOCK_REALTIME_FAST	10	/* FreeBSD-specific. */
#define	CLOCK_MONOTONIC_PRECISE	11	/* FreeBSD-specific. */
#define	CLOCK_MONOTONIC_FAST	12	/* FreeBSD-specific. */
#define	CLOCK_SECOND	13		/* FreeBSD-specific. */
#define	CLOCK_THREAD_CPUTIME_ID	14
#define	CLOCK_PROCESS_CPUTIME_ID	15
#endif

#ifndef TIMER_ABSTIME
#define	TIMER_RELTIME	0x0	/* relative timer */
#define	TIMER_ABSTIME	0x1	/* absolute timer */
#endif

#if __BSD_VISIBLE
#define	CPUCLOCK_WHICH_PID	0
#define	CPUCLOCK_WHICH_TID	1
#endif

#if defined(_KERNEL) || defined(_KERNEL_UT)

/*
 * Kernel to clock driver interface.
 */
void	inittodr(time_t base);
void	resettodr(void);

extern volatile time_t	time_second;
extern volatile time_t	time_uptime;
extern struct bintime tc_tick_bt;
extern sbintime_t tc_tick_sbt;
extern struct bintime tick_bt;
extern sbintime_t tick_sbt;
extern int tc_precexp;
extern int tc_timepercentage;
extern struct bintime bt_timethreshold;
extern struct bintime bt_tickthreshold;
extern sbintime_t sbt_timethreshold;
extern sbintime_t sbt_tickthreshold;

extern volatile int rtc_generation;

/*
 * Functions for looking at our clock: [get]{bin,nano,micro}[up]time()
 *
 * Functions without the "get" prefix returns the best timestamp
 * we can produce in the given format.
 *
 * "bin"   == struct bintime  == seconds + 64 bit fraction of seconds.
 * "nano"  == struct timespec == seconds + nanoseconds.
 * "micro" == struct timeval  == seconds + microseconds.
 *
 * Functions containing "up" returns time relative to boot and
 * should be used for calculating time intervals.
 *
 * Functions without "up" returns UTC time.
 *
 * Functions with the "get" prefix returns a less precise result
 * much faster than the functions without "get" prefix and should
 * be used where a precision of 1/hz seconds is acceptable or where
 * performance is priority. (NB: "precision", _not_ "resolution" !)
 */

void	binuptime(struct bintime *bt);
void	nanouptime(struct timespec *tsp);
void	microuptime(struct timeval *tvp);

static __inline sbintime_t
sbinuptime(void)
{
	struct bintime _bt;

	binuptime(&_bt);
	return (bttosbt(_bt));
}

void	bintime(struct bintime *bt);
void	nanotime(struct timespec *tsp);
void	microtime(struct timeval *tvp);

void	getbinuptime(struct bintime *bt);
void	getnanouptime(struct timespec *tsp);
void	getmicrouptime(struct timeval *tvp);

static __inline sbintime_t
getsbinuptime(void)
{
	struct bintime _bt;

	getbinuptime(&_bt);
	return (bttosbt(_bt));
}

void	getbintime(struct bintime *bt);
void	getnanotime(struct timespec *tsp);
void	getmicrotime(struct timeval *tvp);

void	getboottime(struct timeval *boottime);
void	getboottimebin(struct bintime *boottimebin);

/* Other functions */
int	itimerdecr(struct itimerval *itp, int usec);
int	itimerfix(struct timeval *tv);
int	ppsratecheck(struct timeval *, int *, int);
int	ratecheck(struct timeval *, const struct timeval *);
void	timevaladd(struct timeval *t1, const struct timeval *t2);
void	timevalsub(struct timeval *t1, const struct timeval *t2);
int	tvtohz(struct timeval *tv);

#define	TC_DEFAULTPERC		5

#define	BT2FREQ(bt)                                                     \
	(((uint64_t)0x8000000000000000 + ((bt)->frac >> 2)) /           \
	    ((bt)->frac >> 1))

#define	SBT2FREQ(sbt)	((SBT_1S + ((sbt) >> 1)) / (sbt))

#define	FREQ2BT(freq, bt)                                               \
{									\
	(bt)->sec = 0;                                                  \
	(bt)->frac = ((uint64_t)0x8000000000000000  / (freq)) << 1;     \
}

#define	TIMESEL(sbt, sbt2)						\
	(((sbt2) >= sbt_timethreshold) ?				\
	    ((*(sbt) = getsbinuptime()), 1) : ((*(sbt) = sbinuptime()), 0))

#else /* !_KERNEL */
#include <time.h>

#include <sys/cdefs.h>
#include <sys/select.h>

__BEGIN_DECLS
int	setitimer(int, const struct itimerval *, struct itimerval *);
int	utimes(const char *, const struct timeval *);

#if __BSD_VISIBLE
int	adjtime(const struct timeval *, struct timeval *);
int	clock_getcpuclockid2(id_t, int, clockid_t *);
int	futimes(int, const struct timeval *);
int	futimesat(int, const char *, const struct timeval [2]);
int	lutimes(const char *, const struct timeval *);
int	settimeofday(const struct timeval *, const struct timezone *);
#endif

#if __XSI_VISIBLE
int	getitimer(int, struct itimerval *);
int	gettimeofday(struct timeval *, struct timezone *);
#endif

__END_DECLS

#endif /* !_KERNEL */

#endif /* !_SYS_TIME_H_ */

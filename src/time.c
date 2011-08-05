/*
    Copyright (C) 2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <time.h>
#include "dtk_time.h"

#if !HAVE_DECL_CLOCK_NANOSLEEP
#include "../lib/clock_nanosleep.h"
#endif
#if !HAVE_DECL_CLOCK_GETTIME
#include "../lib/clock_gettime.h"
#endif


#define DTK_CLOCKID	CLOCK_REALTIME


API_EXPORTED
void dtk_gettime(struct dtk_timespec* dtk_ts)
{
	struct timespec ts;

	if (clock_gettime(DTK_CLOCKID, &ts))
		return;

	dtk_ts->sec = ts.tv_sec;
	dtk_ts->nsec = ts.tv_nsec;
	return;
}


API_EXPORTED
int dtk_nanosleep(int abs, const struct dtk_timespec* dtk_ts,
                           struct dtk_timespec* dtk_rem)
{
	int flags = abs ? TIMER_ABSTIME : 0;
	struct timespec rem, ts = {
		.tv_sec = dtk_ts->sec,
		.tv_nsec = dtk_ts->nsec
	};

	if (clock_nanosleep(DTK_CLOCKID, flags, &ts, &rem))
		return -1;

	if (dtk_rem) {
		dtk_rem->sec = rem.tv_sec;
		dtk_rem->nsec = rem.tv_nsec;
	}

	return 0;
}


API_EXPORTED
int dtk_addtime(struct dtk_timespec* ts, long sec, long nsec)
{
	if ((nsec >= 1000000000) || (nsec <= -1000000000)) {
		errno = EINVAL;
		return -1;
	}

	ts->sec += sec;
	ts->nsec += nsec;
	if (ts->nsec >= 1000000000) {
		ts->nsec -= 1000000000;
		ts->sec += 1;
	} else if (ts->nsec < 0) {
		ts->nsec += 1000000000;
		ts->sec -= 1;
	}

	return 0;
}


API_EXPORTED
long dtk_difftime_s(const struct dtk_timespec* ts,
                    const struct dtk_timespec* orig)
{
	long diff = (ts->sec - orig->sec);
	diff += (ts->nsec - orig->nsec)/1000000000;
	return diff;
}


API_EXPORTED
long dtk_difftime_ms(const struct dtk_timespec* ts,
                     const struct dtk_timespec* orig)
{
	long diff = (ts->sec - orig->sec)*1000;
	diff += (ts->nsec - orig->nsec)/1000000;
	return diff;
}


API_EXPORTED
long dtk_difftime_us(const struct dtk_timespec* ts,
                     const struct dtk_timespec* orig)
{
	long diff = (ts->sec - orig->sec)*1000000;
	diff += (ts->nsec - orig->nsec)/1000;
	return diff;
}


API_EXPORTED
long dtk_difftime_ns(const struct dtk_timespec* ts,
                     const struct dtk_timespec* orig)
{
	long diff = (ts->sec - orig->sec)*1000000000;
	diff += (ts->nsec - orig->nsec);
	return diff;
}


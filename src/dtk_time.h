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
#ifndef DTK_TIME_H
#define DTK_TIME_H

#ifdef __cplusplus
extern "C" {
#endif 

struct dtk_timespec {
	long sec;
	long nsec;
};

void dtk_gettime(struct dtk_timespec* ts);
int dtk_nanosleep(int abs, const struct dtk_timespec* ts,
                           struct dtk_timespec* rem);
int dtk_addtime(struct dtk_timespec* ts, long sec, long nsec);
long dtk_difftime_s(const struct dtk_timespec* ts,
                    const struct dtk_timespec* orig);
long dtk_difftime_ms(const struct dtk_timespec* ts,
                     const struct dtk_timespec* orig);
long dtk_difftime_us(const struct dtk_timespec* ts,
                     const struct dtk_timespec* orig);
long dtk_difftime_ns(const struct dtk_timespec* ts,
                     const struct dtk_timespec* orig);

#ifdef __cplusplus
}
#endif

#endif


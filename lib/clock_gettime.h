/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Laboratory CNBI (Chair in Non-Invasive Brain-Machine Interface)
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
#ifndef CLOCK_GETTIME_H
#define CLOCK_GETTIME_H

#include <time.h>

#if !HAVE_DECL_CLOCK_GETTIME
# include "timespec.h"

LOCAL_FN
int clock_gettime(clockid_t clk_id, struct timespec *tp);
# endif //!HAVE_DECL_CLOCK_GETTIME


#endif /* CLOCK_GETTIME_H */


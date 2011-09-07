/*
    Copyright (C) 2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Laboratory CNBI (Chair in Non-Invasive Brain-Machine Interface)
    Pierluca Borsò <pierluca.borso@epfl.ch>

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

#ifndef DTK_VIDEO_H
#define DTK_VIDEO_H

#include <drawtk.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	DTKV_PLAYING	0x01
#define DTKV_EOS	0x02


enum dtk_video_cmd {
	DTKV_CMD_PLAY = 0,
	DTKV_CMD_PAUSE,
	DTKV_CMD_SEEK,
};

#define DTK_AUTOSTART	0x01
#define DTK_NOBLOCKING	0x02

dtk_htex dtk_load_video_tcp(int flags, const char *server, int port);
dtk_htex dtk_load_video_udp(int flags, int port);
dtk_htex dtk_load_video_file(int flags, const char *file);
dtk_htex dtk_load_video_test(int flags);
dtk_htex dtk_load_video_gst(int flags, const char* desc);
int dtk_video_exec(dtk_htex video, int command, const void* arg);
int dtk_video_getstate(dtk_htex video);

#ifdef __cplusplus
}
#endif
#endif

/*
    Copyright (C) 2009-2010  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <drawtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dtk_colors.h>
#include <dtk_event.h>
#include <string.h>


static char imgfilename[256], fontfilename[256];
static char text[] = "This is a test string!!!";

dtk_hwnd wnd;
dtk_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, str;
dtk_hshape comp;

dtk_hfont font;

#define red	dtk_red
#define green	dtk_green
#define white	dtk_white
#define blue	dtk_blue

static void setup_shapes(void)
{
	font  = dtk_load_font(fontfilename);
	dtk_hshape shplist[] = {
		rec1 = dtk_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red),
		rec2 = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green),
		cro = dtk_create_cross(NULL, 0.0, 0.0, 0.2, red),
		img = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white,dtk_load_image(imgfilename, 4)),
		tri = dtk_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red),
		tri2 = dtk_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue),
		cir = dtk_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, dtk_orange_light, 60),
		arr = dtk_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, red),
		str = dtk_create_string(NULL, text ,0.1,-0.0,-0.9, DTK_RIGHT, white, font),
	};

	comp = dtk_create_composite_shape(NULL, 
	                            sizeof(shplist)/sizeof(shplist[0]),
				    shplist, 1);
}

void redraw(dtk_hwnd wnd)
{
	dtk_clear_screen(wnd);
	dtk_draw_shape(comp);
	dtk_update_screen(wnd);
}

int event_handler(dtk_hwnd wnd, int type, const union dtk_event* evt)
{
	int retcode = 1;
	unsigned int h = 0,w = 0;

	switch (type) {
	case DTK_EVT_QUIT:
		retcode = 0;
		break;

	case DTK_EVT_REDRAW:
		redraw(wnd);
		dtk_window_getsize(wnd, &h, &w);
		printf("window size: h=%u  w=%u\n", h, w);
		retcode = 1;
		break;
	
	case DTK_EVT_KEYBOARD:
		if (evt->key.sym == DTKK_ESCAPE)
			retcode = 0;
		else
			retcode = 1;
		break;

	default:
		retcode = 1;
	}

	return retcode;
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	sprintf(imgfilename, "%s/navy.png", getenv("srcdir"));
	if (argc < 2)
		sprintf(fontfilename, "%s/test.ttf", getenv("srcdir"));
	else
		strcpy(fontfilename, argv[1]);

	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	setup_shapes();
	dtk_set_event_handler(wnd, event_handler);

	redraw(wnd);
	while (dtk_process_events(wnd));

	dtk_destroy_font(font);
	dtk_destroy_shape(comp);
	dtk_close(wnd);

	return 0;
}

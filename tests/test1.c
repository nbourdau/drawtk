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
#if HAVE_CONFIG_H
# include <config.h>
#endif 

#include <drawtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <dtk_colors.h>
#include <dtk_time.h>
#include <math.h>

static char imgfilename[256], fontfilename[256];
static char text[] = "This is a test string!!!";

#define NUMVERT	4
#define NUMIND	4
float vertpos[2*NUMVERT];
float vertcolor[4*NUMVERT];
unsigned int indices[NUMIND];

dtk_hwnd wnd;
dtk_htex tex, tex2;
dtk_hfont font;
dtk_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, img2, str, cshp;
dtk_hshape comp;

#define red	dtk_red
#define green	dtk_green
#define white	dtk_white
#define blue	dtk_blue

void setup_complex(void)
{
	unsigned int i;

	vertpos[0] = -0.5f;
	vertpos[1] = -1.0f;
	vertpos[2] = -0.5f;
	vertpos[3] = -0.5f;
	vertpos[4] = 0.0f;
	vertpos[5] = -1.0f;
	vertpos[6] = 0.0f;
	vertpos[7] = -0.5f;

	for (i=0; i<4*NUMVERT; i+=4) {
		vertcolor[i  ] = 0.0f;
		vertcolor[i+1] = 0.0f;
		vertcolor[i+2] = 0.0f;
		vertcolor[i+3] = 1.0f;
	}

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;
}

static void setup_shapes(void)
{
	unsigned int w, h;
	tex = dtk_load_image(imgfilename, 4);
	tex2 = dtk_load_image(imgfilename, 4);
	font  = dtk_load_font(fontfilename);
	dtk_texture_getsize(tex, &w, &h);
	printf("texture size: %ux%u\n", w, h);

	dtk_hshape shplist[] = {
		rec1 = dtk_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red),
		rec2 = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green),
		cro = dtk_create_cross(NULL, 0.0, 0.0, 0.2, red),
		img = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white, tex),
		tri = dtk_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red),
		tri2 = dtk_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue),
		str = dtk_create_string(NULL, text ,0.1,-0.0,-0.9, DTK_HMID, white, font),
		cir = dtk_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, dtk_orange_light, 60),
		arr = dtk_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, red),
		img2 = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white, tex2)
	};

	comp = dtk_create_composite_shape(NULL, 
	                           sizeof(shplist)/sizeof(shplist[0]),
				   shplist, 1);
	setup_complex();
	cshp = dtk_create_complex_shape(NULL, 
	                                NUMVERT, vertpos, vertcolor, NULL,
					NUMIND, indices,
					DTK_TRIANGLE_STRIP, NULL);
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	float angle;
	struct dtk_timespec delay = {1, 0};

	sprintf(imgfilename, "%s/navy.png", getenv("srcdir"));
	sprintf(fontfilename, "%s/test.ttf", getenv("srcdir"));

	wnd = dtk_create_window(640, 480, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	setup_shapes();

	dtk_clear_screen(wnd);
	dtk_draw_shape(comp);
	dtk_draw_shape(cshp);
	dtk_update_screen(wnd);
	dtk_nanosleep(0, &delay, NULL);
	dtk_clear_screen(wnd);
	
	dtk_move_shape(tri,0.1,0.1);
	dtk_move_shape(arr,-0.5,-0.5);
	dtk_move_shape(img2,-0.5,-0.5);
	dtk_draw_shape(comp);
	dtk_draw_shape(cshp);
	dtk_update_screen(wnd);

	delay.sec = 0;
	delay.nsec = 5000000; // 5ms
	for (angle=0.0f; angle<720.0f; angle+=1.0f) {
		dtk_clear_screen(wnd);
		dtk_rotate_shape(arr,angle);
		dtk_rotate_shape(img2,-angle);
		dtk_rotate_shape(comp,angle/2.0f);
		dtk_draw_shape(comp);

		vertpos[2] = -0.5f+0.2f*cos(4*3.14*angle/360.0f);
		vertpos[3] = -0.5f+0.2f*sin(4*3.14*angle/360.0f);
		vertpos[2] = -0.5f+0.2f*sin(4*3.14*angle/360.0f);
		vertpos[3] = -0.5f+0.2f*cos(4*3.14*angle/360.0f);
		vertcolor[4*0+1] = fabs(sin(4*3.14*angle/360.0f));
		vertcolor[4*1+0] = fabs(cos(4*3.14*angle/360.0f));
		vertcolor[4*2+2] = fabs(cos(4*3.14*angle/360.0f));
		vertcolor[4*3+3] = fabs(sin(4*3.14*angle/360.0f));
		dtk_draw_shape(cshp);
		dtk_update_screen(wnd);
		dtk_nanosleep(0, &delay, NULL);
	}


	dtk_destroy_shape(comp);
	dtk_destroy_shape(cshp);
	dtk_destroy_texture(tex);
	dtk_destroy_font(font);
	dtk_close(wnd);


	return 0;
}

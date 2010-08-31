#if HAVE_CONFIG_H
# include <config.h>
#endif 

#include <drawtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <palette.h>
#include <time.h>

#if !HAVE_DECL_CLOCK_NANOSLEEP
#include "../lib/clock_nanosleep.h"
#endif


static char imgfilename[256], fontfilename[256];
static char text[] = "This is a test string!!!";

dtk_hwnd wnd;
dtk_htex tex, tex2;
dtk_hfont font;
dtk_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, img2, str;
dtk_hshape comp;

#define red	pal_basic[red]
#define green	pal_basic[green]
#define white	pal_basic[white]
#define blue	pal_basic[blue]

static void setup_shapes(void)
{
	tex = dtk_load_image(imgfilename, 4);
	tex2 = dtk_load_image(imgfilename, 4);
	font  = dtk_load_font(fontfilename);

	dtk_hshape shplist[] = {
		rec1 = dtk_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red),
		rec2 = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green),
		cro = dtk_create_cross(NULL, 0.0, 0.0, 0.2, red),
		img = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white, tex),
		tri = dtk_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red),
		tri2 = dtk_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue),
		str = dtk_create_string(NULL, text ,0.1,-0.0,-0.9, DTK_HMID, white, font),
		cir = dtk_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, pal_tango[orange_light], 60),
		arr = dtk_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, red),
		img2 = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white, tex2)
	};

	comp = dtk_create_composite_shape(shplist, sizeof(shplist)/sizeof(shplist[0]));
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	float angle;
	struct timespec delay = {1, 0};

	sprintf(imgfilename, "%s/navy.png", getenv("srcdir"));
	sprintf(fontfilename, "%s/test.ttf", getenv("srcdir"));

	wnd = dtk_create_window(640, 480, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	setup_shapes();

	dtk_clear_screen(wnd);
	dtk_draw_shape(comp);
	dtk_update_screen(wnd);
	clock_nanosleep(CLOCK_REALTIME, 0, &delay, NULL);
	dtk_clear_screen(wnd);
	
	dtk_move_shape(tri,0.1,0.1);
	dtk_move_shape(arr,-0.5,-0.5);
	dtk_move_shape(img2,-0.5,-0.5);
	dtk_draw_shape(comp);
	dtk_update_screen(wnd);

	delay.tv_sec = 0;
	delay.tv_nsec = 5000000; // 5ms
	for (angle=0.0f; angle<360.0f; angle+=1.0f) {
		dtk_clear_screen(wnd);
		dtk_rotate_shape(arr,angle);
		dtk_rotate_shape(img2,-angle);
		dtk_rotate_shape(comp,angle/2.0f);
		dtk_draw_shape(comp);
		dtk_update_screen(wnd);
		clock_nanosleep(CLOCK_REALTIME, 0, &delay, NULL);
	}


	dtk_destroy_shape(comp);
	dtk_destroy_texture(tex);
	dtk_close(wnd);


	return 0;
}

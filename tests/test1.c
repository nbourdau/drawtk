#include <feedback.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <palette.h>


static char imgfilename[256], fontfilename[256];
static char text[] = "This is a test string!!!";

fb_hwnd wnd;
fb_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, str;
fb_hshape comp;

#define red	pal_basic[red]
#define green	pal_basic[green]
#define white	pal_basic[white]
#define blue	pal_basic[blue]

static void setup_shapes(void)
{
	fb_hshape shplist[] = {
		rec1 = fb_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red),
		rec2 = fb_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green),
		cro = fb_create_cross(NULL, 0.0, 0.0, 0.2, red),
		img = fb_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white,fb_load_image(imgfilename, 4)),
		tri = fb_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red),
		tri2 = fb_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue),
		str = fb_create_string(NULL, text ,0.1,-0.9,-0.9, white, fontfilename),
		cir = fb_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, pal_tango[orange_light], 60),
		arr = fb_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, red),
	};

	comp = fb_create_composite_shape(shplist, sizeof(shplist)/sizeof(shplist[0]));
}

int main(int argc, char* args[])
{
	
	sprintf(imgfilename, "%s/navy.png", getenv("srcdir"));
	sprintf(fontfilename, "%s/font.png", getenv("srcdir"));

	wnd = fb_create_window(0, 0, 0, 0, 16, "hello");
	fb_make_current_window(wnd);
	setup_shapes();

	fb_clear_screen(wnd);
	fb_draw_shape(comp);
	fb_update_screen(wnd);
	sleep(1);
	fb_clear_screen(wnd);
	
	fb_move_shape(tri,0.1,0.1);
	fb_move_shape(arr,-0.5,-0.5);
	fb_draw_shape(comp);
	fb_update_screen(wnd);
	sleep(1);

	fb_clear_screen(wnd);
	fb_rotate_shape(arr,45.0f);
	fb_draw_shape(comp);
	fb_update_screen(wnd);
	sleep(1);

	fb_clear_screen(wnd);
	fb_rotate_shape(arr,90.0f);
	fb_draw_shape(comp);
	fb_update_screen(wnd);
	sleep(1);
	fb_clear_screen(wnd);

	fb_destroy_shape(comp);
	fb_close(wnd);


	return 0;
}

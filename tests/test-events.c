#include <drawtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <palette.h>
#include <dtk_event.h>


static char imgfilename[256], fontfilename[256];
static char text[] = "This is a test string!!!";

dtk_hwnd wnd;
dtk_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, str;
dtk_hshape comp;

#define red	pal_basic[red]
#define green	pal_basic[green]
#define white	pal_basic[white]
#define blue	pal_basic[blue]

static void setup_shapes(void)
{
	dtk_hshape shplist[] = {
		rec1 = dtk_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red),
		rec2 = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green),
		cro = dtk_create_cross(NULL, 0.0, 0.0, 0.2, red),
		img = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white,dtk_load_image(imgfilename, 4)),
		tri = dtk_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red),
		tri2 = dtk_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue),
		str = dtk_create_string(NULL, text ,0.1,-0.9,-0.9, white, fontfilename),
		cir = dtk_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, pal_tango[orange_light], 60),
		arr = dtk_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, red),
	};

	comp = dtk_create_composite_shape(shplist, sizeof(shplist)/sizeof(shplist[0]));
}

void redraw(dtk_hwnd wnd)
{
	dtk_clear_screen(wnd);
	dtk_draw_shape(comp);
	dtk_update_screen(wnd);
}

int event_handler(dtk_hwnd wnd, unsigned int type, const void* data)
{
	int retcode = 1;
	const struct dtk_keyevent* keyevt = data;

	switch (type) {
	case DTK_EVT_QUIT:
		retcode = 0;
		break;

	case DTK_EVT_REDRAW:
		redraw(wnd);
		retcode = 1;
		break;
	
	case DTK_EVT_KEYBOARD:
		if (keyevt->sym == SDLK_ESCAPE)
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
	sprintf(fontfilename, "%s/font.png", getenv("srcdir"));

	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	setup_shapes();
	dtk_set_event_handler(wnd, event_handler);

	redraw(wnd);
	while (dtk_process_events(wnd));

	dtk_close(wnd);

	return 0;
}

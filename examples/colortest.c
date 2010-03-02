#include <feedback.h>
#include <stdio.h>
#include <unistd.h>
#include <palette.h>
#include <fb_event.h>
#include <SDL/SDL.h>



int event_handler(fb_hwnd wnd, unsigned int type, const void* data)
{
	int retcode = 1;
	struct fb_keyevent* keyevt = data;
	struct fb_mouseevent* mouseevt = data;

	switch (type) {
	case FB_EVT_QUIT:
		retcode = 0;
		break;

	case FB_EVT_KEYBOARD:
		if (keyevt->sym == SDLK_ESCAPE)
			retcode = 0;
		else
			retcode = 1;
		break;

	case FB_EVT_MOUSE:
		
		if (mouseevt->button == SDL_BUTTON_LEFT)
			retcode = 0;
		else
			retcode = 1;
		break;


	default:
		retcode = 1;
	}

	return retcode;
}



int main(int argc, char* args[])
{

	// Create window
	fb_hwnd wnd;
	wnd = fb_create_window(1024, 768, 0, 0, 16, "Color Palettes");
	fb_make_current_window(wnd);
	fb_clear_screen(wnd);
	
	fb_set_event_handler(wnd,event_handler);
	
	fb_hshape tangocolors[27];
	
	// Create all colored squares
	tangocolors[0] = fb_create_rectangle_hw(NULL, -0.3f, 0.8f, 0.1, 0.1, 1, pal_tango[butter_light]);
	tangocolors[1] = fb_create_rectangle_hw(NULL, 0.0f, 0.8f, 0.1, 0.1, 1, pal_tango[butter_med]);
	tangocolors[2] = fb_create_rectangle_hw(NULL, 0.3f, 0.8f, 0.1, 0.1, 1, pal_tango[butter_dark]);
	tangocolors[3] = fb_create_rectangle_hw(NULL, -0.3f, 0.6f, 0.1, 0.1, 1, pal_tango[orange_light]);
	tangocolors[4] = fb_create_rectangle_hw(NULL, 0.0f, 0.6f, 0.1, 0.1, 1, pal_tango[orange_med]);
	tangocolors[5] = fb_create_rectangle_hw(NULL, 0.3f, 0.6f, 0.1, 0.1, 1, pal_tango[orange_dark]);
	tangocolors[6] = fb_create_rectangle_hw(NULL, -0.3f, 0.4f, 0.1, 0.1, 1, pal_tango[chocolate_light]);
	tangocolors[7] = fb_create_rectangle_hw(NULL, 0.0f, 0.4f, 0.1, 0.1, 1, pal_tango[chocolate_med]);
	tangocolors[8] = fb_create_rectangle_hw(NULL, 0.3f, 0.4f, 0.1, 0.1, 1, pal_tango[chocolate_dark]);
	tangocolors[9] = fb_create_rectangle_hw(NULL, -0.3f, 0.2f, 0.1, 0.1, 1, pal_tango[chameleon_light]);
	tangocolors[10] = fb_create_rectangle_hw(NULL, 0.0f, 0.2f, 0.1, 0.1, 1, pal_tango[chameleon_med]);
	tangocolors[11] = fb_create_rectangle_hw(NULL, 0.3f, 0.2f, 0.1, 0.1, 1, pal_tango[chameleon_dark]);
	tangocolors[12] = fb_create_rectangle_hw(NULL, -0.3f, 0.0f, 0.1, 0.1, 1, pal_tango[skyblue_light]);
	tangocolors[13] = fb_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.1, 0.1, 1, pal_tango[skyblue_med]);
	tangocolors[14] = fb_create_rectangle_hw(NULL, 0.3f, 0.0f, 0.1, 0.1, 1, pal_tango[skyblue_dark]);
	tangocolors[15] = fb_create_rectangle_hw(NULL, -0.3f, -0.2f, 0.1, 0.1, 1, pal_tango[plum_light]);
	tangocolors[16] = fb_create_rectangle_hw(NULL, 0.0f, -0.2f, 0.1, 0.1, 1, pal_tango[plum_med]);
	tangocolors[17] = fb_create_rectangle_hw(NULL, 0.3f, -0.2f, 0.1, 0.1, 1, pal_tango[plum_dark]);
	tangocolors[18] = fb_create_rectangle_hw(NULL, -0.3f, -0.4f, 0.1, 0.1, 1, pal_tango[scarletred_light]);
	tangocolors[19] = fb_create_rectangle_hw(NULL, 0.0f, -0.4f, 0.1, 0.1, 1, pal_tango[scarletred_med]);
	tangocolors[20] = fb_create_rectangle_hw(NULL, 0.3f, -0.4f, 0.1, 0.1, 1, pal_tango[scarletred_dark]);
	tangocolors[21] = fb_create_rectangle_hw(NULL, -0.3f, -0.6f, 0.1, 0.1, 1, pal_tango[aluminium_light]);
	tangocolors[22] = fb_create_rectangle_hw(NULL, 0.0f, -0.6f, 0.1, 0.1, 1, pal_tango[aluminium_med]);
	tangocolors[23] = fb_create_rectangle_hw(NULL, 0.3f, -0.6f, 0.1, 0.1, 1, pal_tango[aluminium_dark]);
	tangocolors[24] = fb_create_rectangle_hw(NULL, -0.3f, -0.8f, 0.1, 0.1, 1, pal_tango[aluminium2_light]);
	tangocolors[25] = fb_create_rectangle_hw(NULL, 0.0f, -0.8f, 0.1, 0.1, 1, pal_tango[aluminium2_med]);
	tangocolors[26] = fb_create_rectangle_hw(NULL, 0.3f, -0.8f, 0.1, 0.1, 1, pal_tango[aluminium2_dark]);
        
	fb_hshape light = fb_create_string(NULL,"light",0.05f,-0.35f,0.93f,pal_tango[aluminium_light],"font.png");
	fb_hshape med = fb_create_string(NULL,"med",0.05f,-0.03f,0.93f,pal_tango[aluminium_light],"font.png");
	fb_hshape dark = fb_create_string(NULL,"dark",0.05f,0.25f,0.93f,pal_tango[aluminium_light],"font.png");


	fb_hshape butter = fb_create_string(NULL,"butter",0.05f,-0.8f,0.8f,pal_tango[butter_light],"font.png");
	fb_hshape orange = fb_create_string(NULL,"orange",0.05f,-0.8f,0.6f,pal_tango[orange_light],"font.png");
	fb_hshape chocolate = fb_create_string(NULL,"chocolate",0.05f,-0.8f,0.4f,pal_tango[chocolate_light],"font.png");
	fb_hshape chameleon = fb_create_string(NULL,"chameleon",0.05f,-0.8f,0.2f,pal_tango[chameleon_light],"font.png");
	fb_hshape skyblue = fb_create_string(NULL,"skyblue",0.05f,-0.8f,0.0f,pal_tango[skyblue_light],"font.png");
	fb_hshape plum = fb_create_string(NULL,"plum",0.05f,-0.8f,-0.2f,pal_tango[plum_light],"font.png");
	fb_hshape scarletred = fb_create_string(NULL,"scarletred",0.05f,-0.8f,-0.4f,pal_tango[scarletred_light],"font.png");
	fb_hshape aluminium = fb_create_string(NULL,"aluminium",0.05f,-0.8f,-0.6f,pal_tango[aluminium_light],"font.png");
	fb_hshape aluminium2 = fb_create_string(NULL,"aluminium2",0.05f,-0.8f,-0.8f,pal_tango[aluminium2_light],"font.png");

	fb_hshape exitMsg = fb_create_string(NULL,"Press ESC to exit...",0.07f,0.15f,-0.97f,pal_tango[aluminium2_light],"font.png");

	//Draw shapes
	for(int i=0;i<27;i++){
		fb_bind_shape_to_window(tangocolors[i], wnd);
		fb_draw_shape(tangocolors[i]);
	}
	fb_update_screen(wnd);	
	
	
	fb_bind_shape_to_window(light,wnd);
	fb_bind_shape_to_window(med,wnd);
	fb_bind_shape_to_window(dark,wnd);
	fb_bind_shape_to_window(butter,wnd);
	fb_bind_shape_to_window(orange,wnd);
	fb_bind_shape_to_window(chocolate,wnd);
	fb_bind_shape_to_window(chameleon,wnd);
	fb_bind_shape_to_window(skyblue,wnd);
	fb_bind_shape_to_window(plum,wnd);
	fb_bind_shape_to_window(scarletred,wnd);
	fb_bind_shape_to_window(aluminium,wnd);
	fb_bind_shape_to_window(aluminium2,wnd);
	fb_bind_shape_to_window(exitMsg,wnd);


	fb_draw_shape(light);
	fb_draw_shape(med);
	fb_draw_shape(dark);
	fb_draw_shape(butter);
	fb_draw_shape(orange);
	fb_draw_shape(chocolate);
	fb_draw_shape(chameleon);
	fb_draw_shape(skyblue);
	fb_draw_shape(plum);
	fb_draw_shape(scarletred);
	fb_draw_shape(aluminium);
	fb_draw_shape(aluminium2);
	fb_draw_shape(exitMsg);
	
	fb_update_screen(wnd);
	
	
	while (fb_process_events(wnd));
	
	// Destroy shapes
	for(int i=0;i<27;i++){
		fb_destroy_shape(tangocolors[i]);
	}
	
	fb_destroy_shape(light);
	fb_destroy_shape(med);
	fb_destroy_shape(dark);
	fb_destroy_shape(butter);
	fb_destroy_shape(orange);
	fb_destroy_shape(chocolate);
	fb_destroy_shape(chameleon);
	fb_destroy_shape(skyblue);
	fb_destroy_shape(plum);
	fb_destroy_shape(scarletred);
	fb_destroy_shape(aluminium);
	fb_destroy_shape(aluminium2);
	fb_destroy_shape(exitMsg);


	fb_close(wnd);


	return 0;
}




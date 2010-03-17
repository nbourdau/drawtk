#include <drawtk.h>
#include <stdio.h>
#include <unistd.h>
#include <palette.h>
#include <dtk_event.h>
#include <SDL/SDL.h>



int event_handler(dtk_hwnd wnd, unsigned int type, const void* data)
{
	int retcode = 1;
	struct dtk_keyevent* keyevt = data;
	struct dtk_mouseevent* mouseevt = data;

	switch (type) {
	case DTK_EVT_QUIT:
		retcode = 0;
		break;

	case DTK_EVT_KEYBOARD:
		if (keyevt->sym == SDLK_ESCAPE)
			retcode = 0;
		else
			retcode = 1;
		break;

	case DTK_EVT_MOUSE:
		
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
	dtk_hwnd wnd;
	wnd = dtk_create_window(1024, 768, 0, 0, 16, "Color Palettes");
	dtk_make_current_window(wnd);
	dtk_clear_screen(wnd);
	
	dtk_set_event_handler(wnd,event_handler);
	
	dtk_hshape tangocolors[27];
	
	// Create all colored squares
	tangocolors[0] = dtk_create_rectangle_hw(NULL, -0.3f, 0.8f, 0.1, 0.1, 1, pal_tango[butter_light]);
	tangocolors[1] = dtk_create_rectangle_hw(NULL, 0.0f, 0.8f, 0.1, 0.1, 1, pal_tango[butter_med]);
	tangocolors[2] = dtk_create_rectangle_hw(NULL, 0.3f, 0.8f, 0.1, 0.1, 1, pal_tango[butter_dark]);
	tangocolors[3] = dtk_create_rectangle_hw(NULL, -0.3f, 0.6f, 0.1, 0.1, 1, pal_tango[orange_light]);
	tangocolors[4] = dtk_create_rectangle_hw(NULL, 0.0f, 0.6f, 0.1, 0.1, 1, pal_tango[orange_med]);
	tangocolors[5] = dtk_create_rectangle_hw(NULL, 0.3f, 0.6f, 0.1, 0.1, 1, pal_tango[orange_dark]);
	tangocolors[6] = dtk_create_rectangle_hw(NULL, -0.3f, 0.4f, 0.1, 0.1, 1, pal_tango[chocolate_light]);
	tangocolors[7] = dtk_create_rectangle_hw(NULL, 0.0f, 0.4f, 0.1, 0.1, 1, pal_tango[chocolate_med]);
	tangocolors[8] = dtk_create_rectangle_hw(NULL, 0.3f, 0.4f, 0.1, 0.1, 1, pal_tango[chocolate_dark]);
	tangocolors[9] = dtk_create_rectangle_hw(NULL, -0.3f, 0.2f, 0.1, 0.1, 1, pal_tango[chameleon_light]);
	tangocolors[10] = dtk_create_rectangle_hw(NULL, 0.0f, 0.2f, 0.1, 0.1, 1, pal_tango[chameleon_med]);
	tangocolors[11] = dtk_create_rectangle_hw(NULL, 0.3f, 0.2f, 0.1, 0.1, 1, pal_tango[chameleon_dark]);
	tangocolors[12] = dtk_create_rectangle_hw(NULL, -0.3f, 0.0f, 0.1, 0.1, 1, pal_tango[skyblue_light]);
	tangocolors[13] = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.1, 0.1, 1, pal_tango[skyblue_med]);
	tangocolors[14] = dtk_create_rectangle_hw(NULL, 0.3f, 0.0f, 0.1, 0.1, 1, pal_tango[skyblue_dark]);
	tangocolors[15] = dtk_create_rectangle_hw(NULL, -0.3f, -0.2f, 0.1, 0.1, 1, pal_tango[plum_light]);
	tangocolors[16] = dtk_create_rectangle_hw(NULL, 0.0f, -0.2f, 0.1, 0.1, 1, pal_tango[plum_med]);
	tangocolors[17] = dtk_create_rectangle_hw(NULL, 0.3f, -0.2f, 0.1, 0.1, 1, pal_tango[plum_dark]);
	tangocolors[18] = dtk_create_rectangle_hw(NULL, -0.3f, -0.4f, 0.1, 0.1, 1, pal_tango[scarletred_light]);
	tangocolors[19] = dtk_create_rectangle_hw(NULL, 0.0f, -0.4f, 0.1, 0.1, 1, pal_tango[scarletred_med]);
	tangocolors[20] = dtk_create_rectangle_hw(NULL, 0.3f, -0.4f, 0.1, 0.1, 1, pal_tango[scarletred_dark]);
	tangocolors[21] = dtk_create_rectangle_hw(NULL, -0.3f, -0.6f, 0.1, 0.1, 1, pal_tango[aluminium_light]);
	tangocolors[22] = dtk_create_rectangle_hw(NULL, 0.0f, -0.6f, 0.1, 0.1, 1, pal_tango[aluminium_med]);
	tangocolors[23] = dtk_create_rectangle_hw(NULL, 0.3f, -0.6f, 0.1, 0.1, 1, pal_tango[aluminium_dark]);
	tangocolors[24] = dtk_create_rectangle_hw(NULL, -0.3f, -0.8f, 0.1, 0.1, 1, pal_tango[aluminium2_light]);
	tangocolors[25] = dtk_create_rectangle_hw(NULL, 0.0f, -0.8f, 0.1, 0.1, 1, pal_tango[aluminium2_med]);
	tangocolors[26] = dtk_create_rectangle_hw(NULL, 0.3f, -0.8f, 0.1, 0.1, 1, pal_tango[aluminium2_dark]);
        
	dtk_hshape light = dtk_create_string(NULL,"light",0.05f,-0.35f,0.93f,pal_tango[aluminium_light],"font.png");
	dtk_hshape med = dtk_create_string(NULL,"med",0.05f,-0.03f,0.93f,pal_tango[aluminium_light],"font.png");
	dtk_hshape dark = dtk_create_string(NULL,"dark",0.05f,0.25f,0.93f,pal_tango[aluminium_light],"font.png");


	dtk_hshape butter = dtk_create_string(NULL,"butter",0.05f,-0.8f,0.8f,pal_tango[butter_light],"font.png");
	dtk_hshape orange = dtk_create_string(NULL,"orange",0.05f,-0.8f,0.6f,pal_tango[orange_light],"font.png");
	dtk_hshape chocolate = dtk_create_string(NULL,"chocolate",0.05f,-0.8f,0.4f,pal_tango[chocolate_light],"font.png");
	dtk_hshape chameleon = dtk_create_string(NULL,"chameleon",0.05f,-0.8f,0.2f,pal_tango[chameleon_light],"font.png");
	dtk_hshape skyblue = dtk_create_string(NULL,"skyblue",0.05f,-0.8f,0.0f,pal_tango[skyblue_light],"font.png");
	dtk_hshape plum = dtk_create_string(NULL,"plum",0.05f,-0.8f,-0.2f,pal_tango[plum_light],"font.png");
	dtk_hshape scarletred = dtk_create_string(NULL,"scarletred",0.05f,-0.8f,-0.4f,pal_tango[scarletred_light],"font.png");
	dtk_hshape aluminium = dtk_create_string(NULL,"aluminium",0.05f,-0.8f,-0.6f,pal_tango[aluminium_light],"font.png");
	dtk_hshape aluminium2 = dtk_create_string(NULL,"aluminium2",0.05f,-0.8f,-0.8f,pal_tango[aluminium2_light],"font.png");

	dtk_hshape exitMsg = dtk_create_string(NULL,"Press ESC to exit...",0.07f,0.15f,-0.97f,pal_tango[aluminium2_light],"font.png");

	//Draw shapes
	for(int i=0;i<27;i++){
		dtk_bind_shape_to_window(tangocolors[i], wnd);
		dtk_draw_shape(tangocolors[i]);
	}
	dtk_update_screen(wnd);	
	
	
	dtk_bind_shape_to_window(light,wnd);
	dtk_bind_shape_to_window(med,wnd);
	dtk_bind_shape_to_window(dark,wnd);
	dtk_bind_shape_to_window(butter,wnd);
	dtk_bind_shape_to_window(orange,wnd);
	dtk_bind_shape_to_window(chocolate,wnd);
	dtk_bind_shape_to_window(chameleon,wnd);
	dtk_bind_shape_to_window(skyblue,wnd);
	dtk_bind_shape_to_window(plum,wnd);
	dtk_bind_shape_to_window(scarletred,wnd);
	dtk_bind_shape_to_window(aluminium,wnd);
	dtk_bind_shape_to_window(aluminium2,wnd);
	dtk_bind_shape_to_window(exitMsg,wnd);


	dtk_draw_shape(light);
	dtk_draw_shape(med);
	dtk_draw_shape(dark);
	dtk_draw_shape(butter);
	dtk_draw_shape(orange);
	dtk_draw_shape(chocolate);
	dtk_draw_shape(chameleon);
	dtk_draw_shape(skyblue);
	dtk_draw_shape(plum);
	dtk_draw_shape(scarletred);
	dtk_draw_shape(aluminium);
	dtk_draw_shape(aluminium2);
	dtk_draw_shape(exitMsg);
	
	dtk_update_screen(wnd);
	
	
	while (dtk_process_events(wnd));
	
	// Destroy shapes
	for(int i=0;i<27;i++){
		dtk_destroy_shape(tangocolors[i]);
	}
	
	dtk_destroy_shape(light);
	dtk_destroy_shape(med);
	dtk_destroy_shape(dark);
	dtk_destroy_shape(butter);
	dtk_destroy_shape(orange);
	dtk_destroy_shape(chocolate);
	dtk_destroy_shape(chameleon);
	dtk_destroy_shape(skyblue);
	dtk_destroy_shape(plum);
	dtk_destroy_shape(scarletred);
	dtk_destroy_shape(aluminium);
	dtk_destroy_shape(aluminium2);
	dtk_destroy_shape(exitMsg);


	dtk_close(wnd);


	return 0;
}




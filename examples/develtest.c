#include <drawtk.h>
#include <stdio.h>
#include <unistd.h>
#include <palette.h>

#include <SDL/SDL.h>

int main(int argc, char* args[])
{
	//float red[4] = {1.0f, 0.0f, 0.0f, 1.0f};
	//float green[4] = {0.0f, 1.0f, 0.0f, 1.0f};
	//float blue[4] = {0.0f, 0.0f, 1.0f, 1.0f};
	//float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};	
		
	
	/* !2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
	 * Oh noooo! Michele is creating shapes before having a window!
	 */

	/* Simis: Well, he is known to be a f@@@ing a%%hole! :P
	*/
	dtk_hshape ass = dtk_create_arrow(NULL, 0.0f, 0.0f, -0.5, 0.5, 1, pal_basic[yellow]);

	dtk_hwnd wnd;
	dtk_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, str;

	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	tri = dtk_create_triangle(NULL,0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, pal_tango[skyblue_light]);
	//cir = dtk_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, green, 60);
	//rec1 = dtk_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red);
	//rec2 = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green);
	arr = dtk_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, pal_tango[scarletred_dark]);
	//cro = dtk_create_cross(NULL, 0.0, 0.0, 0.2, red);
	//img = dtk_create_image(NULL, 0.0f,0.0f,1.0f,0.5f,white,"/home/sperdikis/Subversion/cnbi-shared/trunk/libfeedback/examples/P.png");
	//img = dtk_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white,args[1]);
	//img = dtk_create_rectangle(NULL, 0.0f, 0.0f, 1.0f, 0.5f, 1, white);
	//tri = dtk_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red);
	//tri2 = dtk_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue);
	//str = dtk_create_string(NULL, args[2],0.03,-0.9,-0.9,white,args[1]);

	dtk_clear_screen(wnd);
	

	dtk_draw_shape(tri);
	//dtk_draw_shape(cir);
	dtk_draw_shape(arr);
	/* 2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
	 * Yes Perdikis, you can kiss my ass now.
	 */
	/* 
	 * Look at this attitude... Shame on you!
	 */

	dtk_bind_shape_to_window(ass, wnd);
	dtk_draw_shape(ass);

	//dtk_draw_shape(rec1);
	//dtk_draw_shape(rec2);

	//dtk_draw_shape(cro);
	//dtk_draw_shape(tri);
	//dtk_draw_shape(tri2);
	//dtk_draw_shape(img);
	//dtk_draw_shape(str);

	dtk_update_screen(wnd);
	//sleep(1);

	// test for dtk_bgcolor
	// 2009-11-02  Michele Tavella <michele.tavella@epfl.ch>
	// Simis, change the interface! This warning has been
	// here for ages.
	//
	//dtk_bgcolor(pal_tango[chameleon_med]);

	dtk_clear_screen(wnd);
	dtk_move_shape(tri,0.1,0.1);
	dtk_move_shape(arr,-0.1,-0.1);
	dtk_draw_shape(tri);
	dtk_draw_shape(arr);

	dtk_update_screen(wnd);
	//sleep(2);
	//getchar();

	while (dtk_process_events(wnd));


	dtk_destroy_shape(tri);
	//dtk_destroy_shape(tri2);
	//dtk_destroy_shape(cir);
	dtk_destroy_shape(arr);
	//dtk_destroy_shape(rec1);
	//dtk_destroy_shape(rec2);
	//dtk_destroy_shape(cro);
	//dtk_destroy_shape(img);
	//dtk_destroy_shape(str);

	dtk_close(wnd);


	return 0;
}

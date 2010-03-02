#include <feedback.h>
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
	fb_hshape ass = fb_create_arrow(NULL, 0.0f, 0.0f, -0.5, 0.5, 1, pal_basic[yellow]);

	fb_hwnd wnd;
	fb_hshape tri, tri2, cir, arr, rec1, rec2,cro, img, str;

	wnd = fb_create_window(1024, 768, 0, 0, 16, "hello");
	fb_make_current_window(wnd);
	tri = fb_create_triangle(NULL,0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, pal_tango[skyblue_light]);
	//cir = fb_create_circle(NULL, -0.4f, -0.4f, 0.3f, 1, green, 60);
	//rec1 = fb_create_rectangle_2p(NULL, -1.0f, -1.0f, -0.3f,-0.2f, 1, red);
	//rec2 = fb_create_rectangle_hw(NULL, 0.0f, 0.0f, 0.5f, 0.6f, 1, green);
	arr = fb_create_arrow(NULL, 0.0f, 0.0f, 1.0, 0.5, 1, pal_tango[scarletred_dark]);
	//cro = fb_create_cross(NULL, 0.0, 0.0, 0.2, red);
	//img = fb_create_image(NULL, 0.0f,0.0f,1.0f,0.5f,white,"/home/sperdikis/Subversion/cnbi-shared/trunk/libfeedback/examples/P.png");
	//img = fb_create_image(NULL, 0.0f,0.0f,0.5f,0.5f,white,args[1]);
	//img = fb_create_rectangle(NULL, 0.0f, 0.0f, 1.0f, 0.5f, 1, white);
	//tri = fb_create_triangle(NULL, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1, red);
	//tri2 = fb_create_triangle(NULL, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1, blue);
	//str = fb_create_string(NULL, args[2],0.03,-0.9,-0.9,white,args[1]);

	fb_clear_screen(wnd);
	

	fb_draw_shape(tri);
	//fb_draw_shape(cir);
	fb_draw_shape(arr);
	/* 2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
	 * Yes Perdikis, you can kiss my ass now.
	 */
	/* 
	 * Look at this attitude... Shame on you!
	 */

	fb_bind_shape_to_window(ass, wnd);
	fb_draw_shape(ass);

	//fb_draw_shape(rec1);
	//fb_draw_shape(rec2);

	//fb_draw_shape(cro);
	//fb_draw_shape(tri);
	//fb_draw_shape(tri2);
	//fb_draw_shape(img);
	//fb_draw_shape(str);

	fb_update_screen(wnd);
	//sleep(1);

	// test for fb_bgcolor
	// 2009-11-02  Michele Tavella <michele.tavella@epfl.ch>
	// Simis, change the interface! This warning has been
	// here for ages.
	//
	//fb_bgcolor(pal_tango[chameleon_med]);

	fb_clear_screen(wnd);
	fb_move_shape(tri,0.1,0.1);
	fb_move_shape(arr,-0.1,-0.1);
	fb_draw_shape(tri);
	fb_draw_shape(arr);

	fb_update_screen(wnd);
	//sleep(2);
	//getchar();

	while (fb_process_events(wnd));


	fb_destroy_shape(tri);
	//fb_destroy_shape(tri2);
	//fb_destroy_shape(cir);
	fb_destroy_shape(arr);
	//fb_destroy_shape(rec1);
	//fb_destroy_shape(rec2);
	//fb_destroy_shape(cro);
	//fb_destroy_shape(img);
	//fb_destroy_shape(str);

	fb_close(wnd);


	return 0;
}

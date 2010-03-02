#ifndef PALETTE_H
#define PALETTE_H

#ifdef __cplusplus
extern "C" {
#endif

// Enumerations for palette handling
extern const float pal_basic[][4];
extern const float pal_tango[][4];


// Collection of colors for each palette
// See http://tango.freedesktop.org/Tango_Icon_Theme_Guidelines
// for the definition of the TANGO colors
enum libfb_tango{
	butter_light, butter_med, butter_dark,
	orange_light, orange_med, orange_dark,
	chocolate_light, chocolate_med, chocolate_dark,
	chameleon_light, chameleon_med, chameleon_dark,
	skyblue_light, skyblue_med, skyblue_dark,
	plum_light, plum_med, plum_dark,
	scarletred_light, scarletred_med, scarletred_dark,
	aluminium_light, aluminium_med, aluminium_dark,
	aluminium2_light, aluminium2_med, aluminium2_dark,
	NUM_TANGO_COLORS
};

enum libfb_basic
{
	white, black, yellow, orange, blue, green, red, magenta, cyan,
	NUM_BASIC_COLORS
};

#ifdef __cplusplus
}
#endif

#endif

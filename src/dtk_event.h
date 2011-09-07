/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Laboratory CNBI (Chair in Non-Invasive Brain-Machine Interface)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DTK_EVENT_H
#define DTK_EVENT_H

#include <drawtk.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DTK_EVT_REDRAW		0
#define DTK_EVT_QUIT		1
#define DTK_EVT_KEYBOARD	2
#define DTK_EVT_MOUSEBUTTON	3
#define DTK_EVT_MOUSEMOTION	4
#define NUM_DTK_EVT		5

#define DTK_KEY_PRESSED		1
#define DTK_KEY_RELEASED	0


struct dtk_keyevent
{
	unsigned int state;
	unsigned int sym;
	unsigned int mod;
};


struct dtk_mouseevent
{
	unsigned int button;
	unsigned int state;
	unsigned int x;
	unsigned int y;
};

union dtk_event
{
	struct dtk_keyevent key;
	struct dtk_mouseevent mouse;
};


enum dtk_keysym {
	DTKK_UNKNOWN		= 0,
	DTKK_FIRST		= 0,
	DTKK_BACKSPACE		= 8,
	DTKK_TAB		= 9,
	DTKK_CLEAR		= 12,
	DTKK_RETURN		= 13,
	DTKK_PAUSE		= 19,
	DTKK_ESCAPE		= 27,
	DTKK_SPACE		= 32,
	DTKK_EXCLAIM		= 33,
	DTKK_QUOTEDBL		= 34,
	DTKK_HASH		= 35,
	DTKK_DOLLAR		= 36,
	DTKK_AMPERSAND		= 38,
	DTKK_QUOTE		= 39,
	DTKK_LEFTPAREN		= 40,
	DTKK_RIGHTPAREN		= 41,
	DTKK_ASTERISK		= 42,
	DTKK_PLUS		= 43,
	DTKK_COMMA		= 44,
	DTKK_MINUS		= 45,
	DTKK_PERIOD		= 46,
	DTKK_SLASH		= 47,
	DTKK_0			= 48,
	DTKK_1			= 49,
	DTKK_2			= 50,
	DTKK_3			= 51,
	DTKK_4			= 52,
	DTKK_5			= 53,
	DTKK_6			= 54,
	DTKK_7			= 55,
	DTKK_8			= 56,
	DTKK_9			= 57,
	DTKK_COLON		= 58,
	DTKK_SEMICOLON		= 59,
	DTKK_LESS		= 60,
	DTKK_EQUALS		= 61,
	DTKK_GREATER		= 62,
	DTKK_QUESTION		= 63,
	DTKK_AT			= 64,

	DTKK_LEFTBRACKET	= 91,
	DTKK_BACKSLASH		= 92,
	DTKK_RIGHTBRACKET	= 93,
	DTKK_CARET		= 94,
	DTKK_UNDERSCORE		= 95,
	DTKK_BACKQUOTE		= 96,
	DTKK_a			= 97,
	DTKK_b			= 98,
	DTKK_c			= 99,
	DTKK_d			= 100,
	DTKK_e			= 101,
	DTKK_f			= 102,
	DTKK_g			= 103,
	DTKK_h			= 104,
	DTKK_i			= 105,
	DTKK_j			= 106,
	DTKK_k			= 107,
	DTKK_l			= 108,
	DTKK_m			= 109,
	DTKK_n			= 110,
	DTKK_o			= 111,
	DTKK_p			= 112,
	DTKK_q			= 113,
	DTKK_r			= 114,
	DTKK_s			= 115,
	DTKK_t			= 116,
	DTKK_u			= 117,
	DTKK_v			= 118,
	DTKK_w			= 119,
	DTKK_x			= 120,
	DTKK_y			= 121,
	DTKK_z			= 122,
	DTKK_DELETE		= 127,

	DTKK_WORLD_0		= 160,		/* 0xA0 */
	DTKK_WORLD_1		= 161,
	DTKK_WORLD_2		= 162,
	DTKK_WORLD_3		= 163,
	DTKK_WORLD_4		= 164,
	DTKK_WORLD_5		= 165,
	DTKK_WORLD_6		= 166,
	DTKK_WORLD_7		= 167,
	DTKK_WORLD_8		= 168,
	DTKK_WORLD_9		= 169,
	DTKK_WORLD_10		= 170,
	DTKK_WORLD_11		= 171,
	DTKK_WORLD_12		= 172,
	DTKK_WORLD_13		= 173,
	DTKK_WORLD_14		= 174,
	DTKK_WORLD_15		= 175,
	DTKK_WORLD_16		= 176,
	DTKK_WORLD_17		= 177,
	DTKK_WORLD_18		= 178,
	DTKK_WORLD_19		= 179,
	DTKK_WORLD_20		= 180,
	DTKK_WORLD_21		= 181,
	DTKK_WORLD_22		= 182,
	DTKK_WORLD_23		= 183,
	DTKK_WORLD_24		= 184,
	DTKK_WORLD_25		= 185,
	DTKK_WORLD_26		= 186,
	DTKK_WORLD_27		= 187,
	DTKK_WORLD_28		= 188,
	DTKK_WORLD_29		= 189,
	DTKK_WORLD_30		= 190,
	DTKK_WORLD_31		= 191,
	DTKK_WORLD_32		= 192,
	DTKK_WORLD_33		= 193,
	DTKK_WORLD_34		= 194,
	DTKK_WORLD_35		= 195,
	DTKK_WORLD_36		= 196,
	DTKK_WORLD_37		= 197,
	DTKK_WORLD_38		= 198,
	DTKK_WORLD_39		= 199,
	DTKK_WORLD_40		= 200,
	DTKK_WORLD_41		= 201,
	DTKK_WORLD_42		= 202,
	DTKK_WORLD_43		= 203,
	DTKK_WORLD_44		= 204,
	DTKK_WORLD_45		= 205,
	DTKK_WORLD_46		= 206,
	DTKK_WORLD_47		= 207,
	DTKK_WORLD_48		= 208,
	DTKK_WORLD_49		= 209,
	DTKK_WORLD_50		= 210,
	DTKK_WORLD_51		= 211,
	DTKK_WORLD_52		= 212,
	DTKK_WORLD_53		= 213,
	DTKK_WORLD_54		= 214,
	DTKK_WORLD_55		= 215,
	DTKK_WORLD_56		= 216,
	DTKK_WORLD_57		= 217,
	DTKK_WORLD_58		= 218,
	DTKK_WORLD_59		= 219,
	DTKK_WORLD_60		= 220,
	DTKK_WORLD_61		= 221,
	DTKK_WORLD_62		= 222,
	DTKK_WORLD_63		= 223,
	DTKK_WORLD_64		= 224,
	DTKK_WORLD_65		= 225,
	DTKK_WORLD_66		= 226,
	DTKK_WORLD_67		= 227,
	DTKK_WORLD_68		= 228,
	DTKK_WORLD_69		= 229,
	DTKK_WORLD_70		= 230,
	DTKK_WORLD_71		= 231,
	DTKK_WORLD_72		= 232,
	DTKK_WORLD_73		= 233,
	DTKK_WORLD_74		= 234,
	DTKK_WORLD_75		= 235,
	DTKK_WORLD_76		= 236,
	DTKK_WORLD_77		= 237,
	DTKK_WORLD_78		= 238,
	DTKK_WORLD_79		= 239,
	DTKK_WORLD_80		= 240,
	DTKK_WORLD_81		= 241,
	DTKK_WORLD_82		= 242,
	DTKK_WORLD_83		= 243,
	DTKK_WORLD_84		= 244,
	DTKK_WORLD_85		= 245,
	DTKK_WORLD_86		= 246,
	DTKK_WORLD_87		= 247,
	DTKK_WORLD_88		= 248,
	DTKK_WORLD_89		= 249,
	DTKK_WORLD_90		= 250,
	DTKK_WORLD_91		= 251,
	DTKK_WORLD_92		= 252,
	DTKK_WORLD_93		= 253,
	DTKK_WORLD_94		= 254,
	DTKK_WORLD_95		= 255,		/* 0xFF */

	DTKK_KP0		= 256,
	DTKK_KP1		= 257,
	DTKK_KP2		= 258,
	DTKK_KP3		= 259,
	DTKK_KP4		= 260,
	DTKK_KP5		= 261,
	DTKK_KP6		= 262,
	DTKK_KP7		= 263,
	DTKK_KP8		= 264,
	DTKK_KP9		= 265,
	DTKK_KP_PERIOD		= 266,
	DTKK_KP_DIVIDE		= 267,
	DTKK_KP_MULTIPLY	= 268,
	DTKK_KP_MINUS		= 269,
	DTKK_KP_PLUS		= 270,
	DTKK_KP_ENTER		= 271,
	DTKK_KP_EQUALS		= 272,

	DTKK_UP			= 273,
	DTKK_DOWN		= 274,
	DTKK_RIGHT		= 275,
	DTKK_LEFT		= 276,
	DTKK_INSERT		= 277,
	DTKK_HOME		= 278,
	DTKK_END		= 279,
	DTKK_PAGEUP		= 280,
	DTKK_PAGEDOWN		= 281,

	DTKK_F1			= 282,
	DTKK_F2			= 283,
	DTKK_F3			= 284,
	DTKK_F4			= 285,
	DTKK_F5			= 286,
	DTKK_F6			= 287,
	DTKK_F7			= 288,
	DTKK_F8			= 289,
	DTKK_F9			= 290,
	DTKK_F10		= 291,
	DTKK_F11		= 292,
	DTKK_F12		= 293,
	DTKK_F13		= 294,
	DTKK_F14		= 295,
	DTKK_F15		= 296,

	DTKK_NUMLOCK		= 300,
	DTKK_CAPSLOCK		= 301,
	DTKK_SCROLLOCK		= 302,
	DTKK_RSHIFT		= 303,
	DTKK_LSHIFT		= 304,
	DTKK_RCTRL		= 305,
	DTKK_LCTRL		= 306,
	DTKK_RALT		= 307,
	DTKK_LALT		= 308,
	DTKK_RMETA		= 309,
	DTKK_LMETA		= 310,
	DTKK_LSUPER		= 311,	/* Left "Windows" key */
	DTKK_RSUPER		= 312,	/* Right "Windows" key */
	DTKK_MODE		= 313,	/* "Alt Gr" key */
	DTKK_COMPOSE		= 314,	/* Multi-key compose key */

	DTKK_HELP		= 315,
	DTKK_PRINT		= 316,
	DTKK_SYSREQ		= 317,
	DTKK_BREAK		= 318,
	DTKK_MENU		= 319,
	DTKK_POWER		= 320,	/* Power Macintosh power key */
	DTKK_EURO		= 321,	/* Some european keyboards */
	DTKK_UNDO		= 322,	/* Atari keyboard has Undo */

	DTKK_LAST
};

/* Enumeration of valid key mods (possibly OR'd together) */
enum dtk_key_mod {
	DTKMOD_NONE  = 0x0000,
	DTKMOD_LSHIFT= 0x0001,
	DTKMOD_RSHIFT= 0x0002,
	DTKMOD_LCTRL = 0x0040,
	DTKMOD_RCTRL = 0x0080,
	DTKMOD_LALT  = 0x0100,
	DTKMOD_RALT  = 0x0200,
	DTKMOD_LMETA = 0x0400,
	DTKMOD_RMETA = 0x0800,
	DTKMOD_NUM   = 0x1000,
	DTKMOD_CAPS  = 0x2000,
	DTKMOD_MODE  = 0x4000
};

/* Event related declarations */
typedef int (*DTKEvtProc)(dtk_hwnd, int, const union dtk_event*);
void dtk_set_event_handler(dtk_hwnd wnd, DTKEvtProc handler);
int dtk_process_events(dtk_hwnd wnd);


#ifdef __cplusplus
}
#endif

#endif /* DTK_EVENT_H */

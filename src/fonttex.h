#ifndef FONTTEX_H
#define FONTTEX_H


#define NUMCHAR		(256-32)

struct character {
	float advance;
	float xmin, xmax, ymin, ymax;
	float txmin, txmax, tymin, tymax;
};

struct dtk_font {
	struct dtk_texture* tex;
	unsigned int pixwidth, pixheight;
	struct character ch[NUMCHAR];
};

LOCAL_FN
int dtk_char_pos(const struct dtk_font* restrict font, unsigned char c,
                 float* restrict vert, float* restrict texcoords,
		 unsigned int * restrict ind,
		 unsigned int currind, float * restrict org);

#endif // FONTTEX_H

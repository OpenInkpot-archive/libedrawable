#ifndef __COLORMOD
#define __COLORMOD 1

#include "common.h"
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include "image.h"

typedef struct _drawable_color_modifier    DrawableColorModifier;

struct _drawable_color_modifier
{
   DATA8     red_mapping[256];
   DATA8     green_mapping[256];
   DATA8     blue_mapping[256];
   DATA8     alpha_mapping[256];
   DATABIG   modification_count;
};

#define CMOD_APPLY_RGB(cm, r, g, b) \
(r) = (cm)->red_mapping[(int)(r)]; \
(g) = (cm)->green_mapping[(int)(g)]; \
(b) = (cm)->blue_mapping[(int)(b)];

#define CMOD_APPLY_RGBA(cm, r, g, b, a) \
(r) = (cm)->red_mapping[(int)(r)]; \
(g) = (cm)->green_mapping[(int)(g)]; \
(b) = (cm)->blue_mapping[(int)(b)]; \
(a) = (cm)->alpha_mapping[(int)(a)];

#define CMOD_APPLY_R(cm, r) \
(r) = (cm)->red_mapping[(int)(r)];
#define CMOD_APPLY_G(cm, g) \
(g) = (cm)->green_mapping[(int)(g)];
#define CMOD_APPLY_B(cm, b) \
(b) = (cm)->blue_mapping[(int)(b)];
#define CMOD_APPLY_A(cm, a) \
(a) = (cm)->alpha_mapping[(int)(a)];

#define R_CMOD(cm, r) \
(cm)->red_mapping[(int)(r)]
#define G_CMOD(cm, g) \
(cm)->green_mapping[(int)(g)]
#define B_CMOD(cm, b) \
(cm)->blue_mapping[(int)(b)]
#define A_CMOD(cm, a) \
(cm)->alpha_mapping[(int)(a)]

__hidden DrawableColorModifier * __drawable_CreateCmod(void);
__hidden void                 __drawable_FreeCmod(DrawableColorModifier *cm);
__hidden void                 __drawable_CmodChanged(DrawableColorModifier *cm);
__hidden void                 __drawable_CmodSetTables(DrawableColorModifier *cm, DATA8 *r, 
					   DATA8 *g, DATA8 *b, DATA8 *a);
__hidden void                 __drawable_CmodReset(DrawableColorModifier *cm);
__hidden void                 __drawable_DataCmodApply(DATA32 *data, int w, int h, 
					   int jump, DrawableImageFlags *fl,
					   DrawableColorModifier *cm);

__hidden void                 __drawable_CmodGetTables(DrawableColorModifier *cm, DATA8 *r, 
					   DATA8 *g, DATA8 *b, DATA8 *a);
__hidden void                 __drawable_CmodModBrightness(DrawableColorModifier *cm, 
					       double v);
__hidden void                 __drawable_CmodModContrast(DrawableColorModifier *cm, 
					     double v);
__hidden void                 __drawable_CmodModGamma(DrawableColorModifier *cm, 
					  double v);
#endif

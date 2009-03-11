#ifndef __SCALE
#define __SCALE 1

typedef struct _drawable_scale_info DrawableScaleInfo;

__hidden DrawableScaleInfo *
__drawable_CalcScaleInfo(DrawableImage *im, int sw, int sh, int dw, int dh, char aa);
__hidden DrawableScaleInfo *
__drawable_FreeScaleInfo(DrawableScaleInfo *isi);
__hidden void
__drawable_ScaleSampleRGBA(DrawableScaleInfo *isi, DATA32 *dest, int dxx, int dyy,
			int dx, int dy, int dw, int dh, int dow);
__hidden void
__drawable_ScaleAARGBA(DrawableScaleInfo *isi, DATA32 *dest, int dxx, int dyy,
		    int dx, int dy, int dw, int dh, int dow, int sow);
__hidden void
__drawable_ScaleAARGB(DrawableScaleInfo *isi, DATA32 *dest, int dxx, int dyy,
		   int dx, int dy, int dw, int dh, int dow, int sow);
__hidden void
__drawable_Scale_mmx_AARGBA(DrawableScaleInfo *isi, DATA32 *dest, int dxx, int dyy,
			 int dx, int dy, int dw, int dh, int dow, int sow);
#endif

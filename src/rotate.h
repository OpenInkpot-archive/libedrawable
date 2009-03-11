#ifndef __ROTATE
#define __ROTATE 1

#include "image.h"
#include "colormod.h"
#include "blend.h"

/*\ Calc precision \*/
#define _ROTATE_PREC 12
#define _ROTATE_PREC_MAX (1 << _ROTATE_PREC)
#define _ROTATE_PREC_BITS (_ROTATE_PREC_MAX - 1)

__hidden void __drawable_RotateSample(DATA32 *src, DATA32 *dest, int sow, int sw, int sh,
			  int dow, int dw, int dh, int x, int y,
			  int dxh, int dyh, int dxv, int dyv);
__hidden void __drawable_RotateAA(DATA32 *src, DATA32 *dest, int sow, int sw, int sh,
		      int dow, int dw, int dh, int x, int y,
		      int dx, int dy, int dxv, int dyv);
__hidden void __drawable_BlendImageToImageSkewed(DrawableImage *im_src, DrawableImage *im_dst,
				     char aa, char blend, char merge_alpha,
				     int ssx, int ssy, int ssw, int ssh,
				     int ddx, int ddy,
				     int hsx, int hsy, int vsx, int vsy,
				     DrawableColorModifier *cm, DrawableOp op,
                                     int clx, int cly, int clw, int clh);


#ifdef DO_MMX_ASM
__hidden void __drawable_mmx_RotateAA(DATA32 *src, DATA32 *dest, int sow, int sw, int sh,
			  int dow, int dw, int dh, int x, int y,
			  int dx, int dy, int dxv, int dyv);
#endif
#endif

#ifndef __SPAN
#define __SPAN 1


typedef void (*DrawablePointDrawFunction)(DATA32, DATA32 *);

__hidden DrawablePointDrawFunction
__imlib_GetPointDrawFunction(DrawableOp op, char dst_alpha, char blend);


typedef void (*DrawableSpanDrawFunction)(DATA32, DATA32 *, int);

__hidden DrawableSpanDrawFunction
__imlib_GetSpanDrawFunction(DrawableOp op, char dst_alpha, char blend);


typedef void (*DrawableShapedSpanDrawFunction)(DATA8 *, DATA32, DATA32 *, int);

__hidden DrawableShapedSpanDrawFunction
__imlib_GetShapedSpanDrawFunction(DrawableOp op, char dst_alpha, char blend);


#endif


#ifndef __RGBADRAW
#define __RGBADRAW 1

#define IN_SEGMENT(x, sx, sw) \
((unsigned)((x) - (sx)) < (sw))

#define IN_RANGE(x, y, w, h) \
( ((unsigned)(x) < (w)) && ((unsigned)(y) < (h)) )

#define IN_RECT(x, y, rx, ry, rw, rh) \
( ((unsigned)((x) - (rx)) < (rw)) && ((unsigned)((y) - (ry)) < (rh)) )

#define CLIP_RECT_TO_RECT(x, y, w, h, rx, ry, rw, rh) \
{								\
  int   _t0, _t1;						\
								\
  _t0 = MAX(x, (rx));						\
  _t1 = MIN(x + w, (rx) + (rw));				\
  x = _t0;							\
  w = _t1 - _t0;						\
  _t0 = MAX(y, (ry));						\
  _t1 = MIN(y + h, (ry) + (rh));				\
  y = _t0;							\
  h = _t1 - _t0;						\
}

#define DIV_255(a, x, tmp) \
do {                           \
 tmp = (x) + 0x80;             \
 a = (tmp + (tmp >> 8)) >> 8;  \
} while (0)

#define MULT(na, a0, a1, tmp) \
  DIV_255(na, (a0) * (a1), tmp)


typedef struct _drawable_point DrawablePoint;

struct _drawable_point
{
   int x, y;
};

typedef struct _drawable_rectangle Drawable_Rectangle;

struct _drawable_rectangle
{
   int x, y, w, h;
};

typedef struct _drawable_polygon _DrawablePoly;
typedef _DrawablePoly *DrawablePoly;

struct _drawable_polygon
{
   DrawablePoint *points;
   int pointcount;
   int  lx, rx;
   int  ty, by;
};

/* image related operations: in rgbadraw.c */

__hidden void __drawable_FlipImageHoriz(DrawableImage * im);
__hidden void __drawable_FlipImageVert(DrawableImage * im);
__hidden void __drawable_FlipImageBoth(DrawableImage * im);
__hidden void __drawable_FlipImageDiagonal(DrawableImage * im, int direction);
__hidden void __drawable_BlurImage(DrawableImage * im, int rad);
__hidden void __drawable_SharpenImage(DrawableImage * im, int rad);
__hidden void __drawable_TileImageHoriz(DrawableImage * im);
__hidden void __drawable_TileImageVert(DrawableImage * im);

__hidden void __drawable_copy_alpha_data(DrawableImage * src, DrawableImage * dst, int x, int y,
                             int w, int h, int nx, int ny);

__hidden void __drawable_copy_image_data(DrawableImage * im, int x, int y, int w, int h,
                             int nx, int ny);


/* point and line drawing: in line.c */

__hidden DrawableUpdate *
__drawable_Point_DrawToImage(int x, int y, DATA32 color,
			  DrawableImage *im, int clx, int cly, int clw, int clh,
			  DrawableOp op, char blend, char make_updates);

__hidden DrawableUpdate *
__drawable_Line_DrawToImage(int x0, int y0, int x1, int y1, DATA32 color,
			 DrawableImage *im, int clx, int cly, int clw, int clh,
			 DrawableOp op, char blend, char anti_alias,
			 char make_updates);


/* rectangle drawing and filling: in rectangle.c */

__hidden void
__drawable_Rectangle_DrawToImage(int xc, int yc, int w, int h, DATA32 color, 
			      DrawableImage *im, int clx, int cly, int clw, int clh,
			      DrawableOp op, char blend);

__hidden void
__drawable_Rectangle_FillToImage(int xc, int yc, int w, int h, DATA32 color, 
			      DrawableImage *im, int clx, int cly, int clw, int clh,
			      DrawableOp op, char blend);


/* ellipse drawing and filling: in ellipse.c */

__hidden void
__drawable_Ellipse_DrawToImage(int xc, int yc, int a, int b, DATA32 color, 
			    DrawableImage *im, int clx, int cly, int clw, int clh,
			    DrawableOp op, char blend, char anti_alias);

__hidden void
__drawable_Ellipse_FillToImage(int xc, int yc, int a, int b, DATA32 color, 
			    DrawableImage *im, int clx, int cly, int clw, int clh,
			    DrawableOp op, char blend, char anti_alias);


/* polygon handling functions: in polygon.c */

__hidden DrawablePoly __drawable_polygon_new(void);
__hidden void __drawable_polygon_free(DrawablePoly poly);
__hidden void __drawable_polygon_add_point(DrawablePoly poly, int x, int y);
__hidden unsigned char __drawable_polygon_contains_point(DrawablePoly poly, int x, int y);
__hidden void __drawable_polygon_get_bounds(DrawablePoly poly, int *px1, int *py1, int *px2, int *py2);


/* polygon drawing and filling: in polygon.c */

__hidden void
__drawable_Polygon_DrawToImage(DrawablePoly poly, char closed, DATA32 color,
			    DrawableImage *im, int clx, int cly, int clw, int clh,
			    DrawableOp op, char blend, char anti_alias);
__hidden void
__drawable_Polygon_FillToImage(DrawablePoly poly, DATA32 color,
			    DrawableImage *im, int clx, int cly, int clw, int clh,
			    DrawableOp op, char blend, char anti_alias);


#endif

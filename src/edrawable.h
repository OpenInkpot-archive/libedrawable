#ifndef _EDRAWABLE_H
#define _EDRAWABLE_H 1

#include <Evas.h>
#include <Ewl.h>
#include "eimlib.h"

#define EWL_DRAWABLE(ed) ((Ewl_Drawable *) ed)

struct _Ewl_Drawable {
    Ewl_Image superclass;
    Drawable_Context * context;
};
typedef struct _Ewl_Drawable Ewl_Drawable;
typedef void * EDrawablePolygon;


EAPI Ewl_Drawable *  ewl_drawable_new();
EAPI void         ewl_drawable_destroy(Ewl_Drawable *);

EAPI void         ewl_drawable_draw_line(Ewl_Drawable *, int x1, int y1, int x2, int y2);

EAPI void         ewl_drawable_set_colors(Ewl_Drawable *, Ewl_Color_Set *);
EAPI void         ewl_drawable_draw_rectangle(Ewl_Drawable *, int, int, int, int);
EAPI void         ewl_drawable_draw_rectangle_fill(Ewl_Drawable *, int, int, int, int);

EAPI EDrawablePolygon  ewl_drawable_polygon_new();
EAPI void         ewl_drawable_draw_polygon(Ewl_Drawable *, EDrawablePolygon );
EAPI void         ewl_drawable_draw_polygon_fill(Ewl_Drawable *, EDrawablePolygon );
EAPI void         ewl_drawable_polygon_delete(EDrawablePolygon);
EAPI void         ewl_drawable_polygon_add(EDrawablePolygon, int, int);

EAPI void         ewl_drawable_draw_ellipse(Ewl_Drawable* , int x, int y, int r, int r2);
EAPI void         ewl_drawable_draw_ellipse_filled(Ewl_Drawable*, int x, int y, int r, int r2);

EAPI void         ewl_drawable_draw_text(Ewl_Drawable*, int, int, int, char *, char*, char *);

EAPI void         ewl_drawable_set_clip(Ewl_Drawable*, int, int, int, int);
EAPI void         ewl_drawable_reset_clip(Ewl_Drawable*);

EAPI void         ewl_drawable_commit(Ewl_Drawable *);
#endif

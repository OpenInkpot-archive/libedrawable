#ifndef _EDRAWABLE_H
#define _EDRAWABLE_H 1

#include <Evas.h>
#include <Ewl.h>
#include "eimlib.h"

#define E_DRAWABLE(ed) ((EDrawable *) ed)

struct _Ewl_Drawable {
    Ewl_Image image;
    Drawable_Context * context;
};
typedef struct _Ewl_Drawable Ewl_Drawable;
typedef void * EDrawablePolygon;


EAPI Ewl_Drawable *  e_drawable_new();
EAPI void         e_drawable_destroy(Ewl_Drawable *);

EAPI void         e_drawable_draw_line(Ewl_Drawable *, int x1, int y1, int x2, int y2);

EAPI EDrawablePolygon  e_drawable_polygon_new();
EAPI void         e_drawable_draw_polygon(Ewl_Drawable *, EDrawablePolygon );
EAPI void         e_drawable_draw_polygon_fill(Ewl_Drawable *, EDrawablePolygon );
EAPI void         e_drawable_polygon_delete(EDrawablePolygon);
EAPI void         e_drawable_polygon_add(EDrawablePolygon, int, int);

EAPI void         e_drawable_draw_arc(Ewl_Drawable* , int x, int y, int r);
EAPI void         e_drawable_draw_arc_filled(Ewl_Drawable*, int x, int y, int r);

EAPI void         e_drawable_draw_text(Ewl_Drawable*, int, int, int, char *, char*, char *);

#endif

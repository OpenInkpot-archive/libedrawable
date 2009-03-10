#ifndef _EDRAWABLE_H
#define _EDRAWABLE_H 1

#include <Evas.h>

#define E_DRAWABLE(ed) ((EDrawable *) ed)

EAPI EDrawable *  e_drawable_new();
EAPI void         e_drawable_delete();

EAPI void         e_drawable_draw_line(EDrawable *, int x1, int y1, int x2, int y2);

EAPI EDrawable_Polygon  e_drawable_polygon_new();
EAPI void         e_drawable_draw_polygon(EDrawable *, EDrawablePolygon *);
EAPI void         e_drawable_draw_polygon_fill(EDrawable *, EDrawablePolygon *);
EAPI void         e_drawable_polygon_delete(EDrawablePolygon*);
EAPI void         e_drawable_polygon_add(EDrawablePolygon*, int, int);

EAPI void         e_drawable_draw_arc(EDrawable* , int x, int y, int r);
EAPI void         e_drawable_draw_arc_filled(EDrawable*, int x, int y, int r);

EAPI void         e_drawable_draw_text(EDrawable*, int, int, int, char *, char*, char *);

#endif

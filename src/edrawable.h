#ifndef _EDRAWABLE_H
#define _EDRAWABLE_H 1

#include <Evas.h>
#include "eimlib.h"

struct _EDrawable {
    Evas_Object      *image;
    Drawable_Context  context;
    Drawable_Updates  updates;
};

typedef struct _EDrawable EDrawable;
typedef void * EDrawablePolygon;


extern Evas_Object *
edrawable_add(Evas *, int w, int h);

/*
void
edrawable_image_file_set(Evas_Object *, const char *file, const char *key);

extern Evas_Object *
edrawable_load(Evas *evas, const char *file, const char *key);
*/

extern void
edrawable_draw_line(Evas_Object *, int x1, int y1, int x2, int y2);

extern void
edrawable_set_colors(Evas_Object *, int r, int g, int b, int a);

extern void
edrawable_draw_rectangle(Evas_Object *, int, int, int, int);

extern void
edrawable_draw_rectangle_fill(Evas_Object *, int, int, int, int);

extern EDrawablePolygon
edrawable_polygon_new();

extern void
edrawable_draw_polygon(Evas_Object *, EDrawablePolygon );

extern void
edrawable_draw_polygon_fill(Evas_Object *, EDrawablePolygon );

extern void
edrawable_polygon_delete(EDrawablePolygon);

extern void
edrawable_polygon_add(EDrawablePolygon, int, int);

extern void
edrawable_draw_ellipse(Evas_Object * , int x, int y, int r, int r2);

extern void
edrawable_draw_ellipse_filled(Evas_Object*, int x, int y, int r, int r2);

extern int
edrawable_get_font_ascent(Evas_Object*);

extern int
edrawable_get_font_descent(Evas_Object*);

extern void
edrawable_select_font(Evas_Object *e, const char *fontname, int size);

extern void
edrawable_draw_text(Evas_Object*, int x, int y, const char *text);

extern void
edrawable_get_text_size(Evas_Object*, const char *text, int *horisontal, int *vertical);

extern void
edrawable_set_clip(Evas_Object*, int, int, int, int);

extern void
edrawable_reset_clip(Evas_Object*);

extern void
edrawable_commit(Evas_Object *);

extern void
edrawable_blend_image_onto_image(Evas_Object *obj,
           Drawable_Image source_image,
           char merge_alpha, int source_x,
           int source_y, int source_width,
           int source_height, int destination_x,
           int destination_y, int destination_width,
           int destination_height);

extern Drawable_Image
edrawable_create_cropped_image(Evas_Object *obj, int x, int y, int w, int h);

extern void
edrawable_free_image(Drawable_Image di);

extern void
edrawable_update_append_rect(Evas_Object *obj, int x, int y, int w, int h);
#endif

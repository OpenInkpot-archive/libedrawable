#include "edrawable.h"
#include "ewl_debug.h"
#include "ewl_macros.h"

EAPI Ewl_Drawable *
e_drawable_new() {
    Ewl_Drawable *drawable;

    DENTER_FUNCTION(DLEVEL_STABLE);

    drawable = NEW(Ewl_Drawable, 1);
    if (!drawable)
        DRETURN_PTR(NULL, DLEVEL_STABLE);
    if (!ewl_drawable_init(drawable)) {
        ewl_image_destroy(EWL_IMAGE(drawable));
        drawable = NULL;
    }

    DRETURN_PTR(drawable, DLEVEL_STABLE);
}

EAPI void
e_drawable_configure(Ewl_Drawable *e) {
    Evas_Object * image;
    int h, w;
    DATA32 *data;
    Drawable_Image di;

    image = EWL_IMAGE(e)->image;
    evas_object_size_get(image, &w, &h);
    data = evas_object_image_data_get(image, 1);

    di = drawable_create_image_using_data(w, h, data);
    drawable_context_set_image(e->context, di);
}

EAPI void
e_drawable_destroy(Ewl_Drawable *e) {
    if(e->context)
        drawable_condext_free(e->context);
    ewl_image_destroy(EWL_IMAGE(e));
}

Ewl_Drawable *
e_drawable_init(Ewl_Drawable *e){
    Ewl_Image * i;
    
    i = EWL_IMAGE(e);
    if(!ewl_image_init(i))
        return NULL;
    
    e->context = drawable_context_new();
    if(!e->context)
        return NULL;
    e_drawable_configure(e);

    DRETURN_PTR(e, DLEVEL_STABLE);
}

EAPI void         e_drawable_draw_line(Ewl_Drawable *, int x1, int y1, int x2, int y2);

EAPI EDrawablePolygon  
e_drawable_polygon_new() {
    return (EDrawablePolygon *) drawable_polygon_new();
}

EAPI void
e_drawable_draw_polygon(Ewl_Drawable *e, EDrawablePolygon p) {
    drawable_polygon_draw(e->context, p);
}

EAPI void
e_drawable_draw_polygon_fill(Ewl_Drawable *e, EDrawablePolygon  p) {
    drawable_polygon_draw_fill(e->context, p);
}

EAPI void
e_drawable_polygon_delete(EDrawablePolygon p) {
    drawable_polygon_delete(p);
}

EAPI void
e_drawable_polygon_add(EDrawablePolygon p, int x, int y) {
    drawable_polygon_add(p, x, y);
}

EAPI void
e_drawable_draw_arc(Ewl_Drawable* e, int x, int y, int r) {
    drawable_draw_ellipse(e->context, x, y, r, r);
}

EAPI void
e_drawable_draw_arc_filled(Ewl_Drawable* e, int x, int y, int r) {
    drawable_draw_ellipse_filled(e->context, x, y, r, r);
}

EAPI void 
e_drawable_draw_text(Ewl_Drawable* e, int x, int y, int size, char *font, char*font2, char *text) {
}

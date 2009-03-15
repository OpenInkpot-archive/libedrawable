#include "edrawable.h"
#include "ewl_debug.h"
#include "ewl_macros.h"

Ewl_Drawable *
ewl_drawable_init(Ewl_Drawable *e);

EAPI Ewl_Drawable *
ewl_drawablewl_new() {
    Ewl_Drawable *drawable;

    DENTER_FUNCTION(DLEVEL_STABLE);

    drawable = NEW(Ewl_Drawable, 1);
    if (!drawable)
        DRETURN_PTR(NULL, DLEVEL_STABLE);
    if (!ewl_drawable_init(drawable)) {
        ewl_widget_destroy(EWL_WIDGET(drawable));
        drawable = NULL;
    }

    DRETURN_PTR(drawable, DLEVEL_STABLE);
}

EAPI void
ewl_drawable_configure(Ewl_Drawable *e) {
    Evas_Object * image;
    int h, w;
    DATA32 *data;
    Drawable_Image di;

    image = EWL_IMAGE(e)->image;
    evas_object_image_size_get(image, &w, &h);
    data = evas_object_image_data_get(image, 1);

    di = drawable_create_image_using_data(w, h, data);
    drawable_context_set_image(e->context, di);
}


static void
ewl_drawable_cb_destroy(Ewl_Widget *w, void *ev_data /*__UNUSED__ */,
                                          void *user_data /*__UNUSED__ */)
{
        Ewl_Drawable *e;

        DENTER_FUNCTION(DLEVEL_STABLE);
        DCHECK_PARAM_PTR(w);
//        DCHECK_TYPE(w, EWL_IMAGE_TYPE);

        e = EWL_DRAWABLE(w);
        if(e->context)
            drawable_context_free(e->context);


        DLEAVE_FUNCTION(DLEVEL_STABLE);
}


Ewl_Drawable *
ewl_drawable_init(Ewl_Drawable *e){
    Ewl_Image * i;
    
    i = EWL_IMAGE(e);
    if(!ewl_image_init(i))
        return NULL;
    ewl_callback_prepend(EWL_WIDGET(e),
            EWL_CALLBACK_DESTROY, ewl_drawable_cb_destroy, NULL);
    e->context = drawable_context_new();
    if(!e->context)
        return NULL;
    ewl_drawable_configure(e);

    DRETURN_PTR(e, DLEVEL_STABLE);
}

EAPI void         ewl_drawable_draw_line(Ewl_Drawable *e, int x1, int y1, int x2, int y2) {
    drawable_image_draw_line(e->context, x1, y1, x2, y2, 0);
}

EAPI EDrawablePolygon  
ewl_drawable_polygon_new() {
    return (EDrawablePolygon *) drawable_polygon_new();
}

EAPI void
ewl_drawable_draw_polygon(Ewl_Drawable *e, EDrawablePolygon p) {
    drawable_image_draw_polygon(e->context, p, 0);
}

EAPI void
ewl_drawable_draw_polygon_fill(Ewl_Drawable *e, EDrawablePolygon  p) {
    drawable_image_fill_polygon(e->context, p);
}

EAPI void
ewl_drawable_polygon_delete(EDrawablePolygon p) {
    drawable_polygon_free(p);
}

EAPI void
ewl_drawable_polygon_add(EDrawablePolygon p, int x, int y) {
    drawable_polygon_add_point(p, x, y);
}

EAPI void
ewl_drawable_draw_arc(Ewl_Drawable* e, int x, int y, int r) {
    drawable_image_draw_ellipse(e->context, x, y, r, r);
}

EAPI void
ewl_drawable_draw_arc_filled(Ewl_Drawable* e, int x, int y, int r) {
    drawable_image_fill_ellipse(e->context, x, y, r, r);
}

EAPI void 
ewl_drawable_draw_text(Ewl_Drawable* e, int x, int y, int size, char *font, char*font2, char *text) {
}

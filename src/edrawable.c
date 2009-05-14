#include "edrawable.h"
#include "ewl_debug.h"
#include "ewl_macros.h"

Ewl_Drawable *
ewl_drawable_init(Ewl_Drawable *e);

EAPI Ewl_Drawable *
ewl_drawable_new() {
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

    Ewl_Widget *wi = EWL_WIDGET(e);
    image = EWL_IMAGE(e)->image;
//        if (wi->fx_clip_box)
//                        evas_object_stack_below(image, wi->fx_clip_box);

//          if (wi->fx_clip_box)
//                     evas_object_clip_set(image, wi->fx_clip_box);
    evas_object_pass_events_set(image, TRUE);
                                                
    evas_object_image_fill_set(image, 0, 0, CURRENT_W(e), CURRENT_H(e));
    evas_object_image_size_set(image, CURRENT_W(e), CURRENT_H(e));
    evas_object_image_size_get(image, &w, &h);
    data = evas_object_image_data_get(image, 1);
    printf("Create image: %d x %d, %d x %d\n", w, h, CURRENT_W(e), CURRENT_H(e));
    di = drawable_create_image_using_data(w, h, data);
    drawable_context_set_image(e->context, di);
    drawable_image_set_alpha(e->context, 1);
}


static void
ewl_drawable_cb_configure(Ewl_Widget *w, void *ev_data, void *user_data) {
    ewl_drawable_configure(EWL_DRAWABLE(w));
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
    ewl_callback_append(EWL_WIDGET(e), EWL_CALLBACK_CONFIGURE,
            ewl_drawable_cb_configure, NULL);
    e->updates = NULL;
    e->context = drawable_context_new();
    if(!e->context)
        return NULL;
//    ewl_drawable_configure(e);

    DRETURN_PTR(e, DLEVEL_STABLE);
}

EAPI void
ewl_drawable_draw_line(Ewl_Drawable *e, int x1, int y1, int x2, int y2) {
    e->updates = __drawable_AppendUpdates(
        drawable_image_draw_line(e->context, x1, y1, x2, y2, 1),
        e->updates);
}

EAPI void
ewl_drawable_draw_rectangle(Ewl_Drawable *e, int x1, int y1, int x2, int y2) {
    drawable_image_draw_rectangle(e->context, x1, y1, x2, y2);
    e->updates = drawable_update_append_rect(e->updates, x1, y1, x1, y2);
}

EAPI void
ewl_drawable_draw_rectangle_fill(Ewl_Drawable *e, int x1, int y1, int x2, int y2) {
    drawable_image_fill_rectangle(e->context, x1, y1, x2, y2);
    e->updates = drawable_update_append_rect(e->updates, x1, y1, x1, y2);
}

EAPI EDrawablePolygon  
ewl_drawable_polygon_new() {
    return (EDrawablePolygon *) drawable_polygon_new();
}

EAPI void
ewl_drawable_draw_polygon(Ewl_Drawable *e, EDrawablePolygon p) {
    int x, y, w, h;
    drawable_image_draw_polygon(e->context, p, 0);
    drawable_polygon_get_bounds(p, &x, &y, &w, &h);
    e->updates = drawable_update_append_rect(e->updates, x, y, w, h);
}

EAPI void
ewl_drawable_draw_polygon_fill(Ewl_Drawable *e, EDrawablePolygon  p) {
    int x, y, w, h;
    drawable_image_fill_polygon(e->context, p);
    drawable_polygon_get_bounds(p, &x, &y, &w, &h);
    e->updates = drawable_update_append_rect(e->updates, x, y, w, h);
}

EAPI void
ewl_drawable_polygon_delete(EDrawablePolygon p) {
    drawable_polygon_free(p);
}

EAPI void
ewl_drawable_polygon_add(EDrawablePolygon p, int x, int y) {
    drawable_polygon_add_point(p, x, y);
}

static void
_update_ellipse(Ewl_Drawable *e, int x, int y, int r1, int r2) {
    int w, h;
    w = x + r1;
    h = w + r2;
    x -= r1;
    y -= r2;
    e->updates = drawable_update_append_rect(e->updates, x, y, w, h);
}

EAPI void
ewl_drawable_draw_ellipse(Ewl_Drawable* e, int x, int y, int r, int r2) {
    drawable_image_draw_ellipse(e->context, x, y, r, r2);
    _update_ellipse(e, x, y, r, r2);
}

EAPI void
ewl_drawable_draw_ellipse_filled(Ewl_Drawable* e, int x, int y, int r, int r2) {
    drawable_image_fill_ellipse(e->context, x, y, r, r2);
    _update_ellipse(e, x, y, r, r2);
}


EAPI void
ewl_drawable_set_colors(Ewl_Drawable *e, Ewl_Color_Set *set) {
    drawable_context_set_color(e->context, set->r, set->g, set->b, set->a);
}

EAPI void         ewl_drawable_set_clip(Ewl_Drawable *e, int x, int y, int w, int h) {
    drawable_context_set_cliprect(e->context, x, y, w, h);
}

EAPI void         ewl_drawable_reset_clip(Ewl_Drawable* e){
    int w, h;
    w = drawable_image_get_width(e->context);
    h = drawable_image_get_height(e->context);
    ewl_drawable_set_clip(e, 0, 0, w, h);
}

EAPI void
ewl_drawable_select_font(Ewl_Drawable *e, char *fontname, int size) {
    printf("load font...\n");
    drawable_load_font(e->context, fontname, 0, size);
}

EAPI void
ewl_drawable_draw_text(Ewl_Drawable *e, int x, int y, char *text) {
    printf("draw...\n");
    drawable_text_draw(e->context, x, y, text);
}

EAPI void
ewl_drawable_get_text_size(Ewl_Drawable* e, char *text, int *vertical, int *horisontal) {
    drawable_get_text_size(e->context, text, horisontal, vertical); 
}

EAPI int
ewl_drawable_get_font_ascent(Ewl_Drawable* e) {
    drawable_get_maximum_font_ascent(e->context);
}

EAPI int
ewl_drawable_get_font_descent(Ewl_Drawable* e) {
    drawable_get_maximum_font_descent(e->context);
}

EAPI void
ewl_drawable_commit(Ewl_Drawable *e) {
    Ewl_Image *img = EWL_IMAGE(e);
    void *in;
    int w, h;

    in = drawable_image_get_data(e->context);
    w = drawable_image_get_width(e->context);
    h = drawable_image_get_height(e->context);
//    evas_object_image_data_set(img->image, in);
    __drawable_PropagateUpdates(e->updates, img->image);
    e->updates=NULL;
    evas_object_image_data_update_add(img->image, 0, 0, w, h);
}


#include <stdio.h>
#include <Evas.h>
#include "edrawable.h"
#include "eimlib.h"

static void
_edrawable_add(Evas_Object *obj) {
    EDrawable *drawable;
    drawable = calloc(1, sizeof(EDrawable));
    evas_object_smart_data_set(obj, drawable);
}

static void
_edrawable_del(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    if(drawable) {
        if(drawable->context)
            drawable_context_free(drawable->context);
        if(drawable->clip)
            evas_object_del(drawable->clip);
        if(drawable->updates)
            __drawable_FreeUpdates(drawable->updates);
        free(drawable);
    }
}

static void
_edrawable_init(Evas_Object *obj, Evas *evas, int w, int h) {

    EDrawable *drawable  = evas_object_smart_data_get(obj);
    if(!drawable) {
        printf("Can't initialize\n");
        return;
    }

    drawable->clip = evas_object_rectangle_add(evas);
    evas_object_smart_member_add(drawable->clip, obj);

    drawable->image = evas_object_image_add(evas);
    evas_object_pass_events_set(drawable->image, TRUE);
    evas_object_image_size_set(drawable->image, w, h);
    evas_object_image_fill_set(drawable->image, 0, 0, w, h);

    evas_object_stack_above(drawable->image, drawable->clip);
    evas_object_clip_set(drawable->image, drawable->clip);

    drawable->updates = NULL;
    drawable->context = drawable_context_new();

    void * data = evas_object_image_data_get(drawable->image, 1);
    printf("Create image: %d x %d\n", w, h);
    Drawable_Image di = drawable_create_image_using_data(w, h, data);
    drawable_context_set_image(drawable->context, di);
    drawable_image_set_alpha(drawable->context, 1);

}

static void
_edrawable_show(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    evas_object_show(drawable->clip);
    evas_object_show(drawable->image);
}

static void
_edrawable_hide(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    evas_object_hide(drawable->clip);
    evas_object_hide(drawable->image);
}

static void
_edrawable_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    evas_object_move(drawable->clip, x, y);
    evas_object_move(drawable->image, x, y);
}


static void
_edrawable_resize(Evas_Object *obj, Evas_Coord x, Evas_Coord y) {
    printf("Resize called\n");
}

static void
_edrawable_clip_set(Evas_Object *obj, Evas_Object *clip) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    evas_object_clip_set(drawable->clip, clip);
}

static void
_edrawable_clip_unset(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    evas_object_clip_unset(drawable->clip);
}

static Evas_Smart * _edrawable_smart_get() {
    static Evas_Smart * smart = NULL;
    static Evas_Smart_Class klass = {
        .name = "edrawable",
        .version = EVAS_SMART_CLASS_VERSION,
        .add = _edrawable_add,
        .del =  _edrawable_del,
        .move = _edrawable_move,
        .resize = _edrawable_resize,
        .show = _edrawable_show,
        .hide = _edrawable_hide,
        .color_set = NULL,
        .clip_set = _edrawable_clip_set,
        .clip_unset = _edrawable_clip_unset,
        .calculate = NULL,
        .member_add = NULL,
        .member_del = NULL
    };

    if(!smart)
        smart = evas_smart_class_new(&klass);
    return smart;
}

Evas_Object *
edrawable_add(Evas *evas, int w, int h) {
    Evas_Object *obj =  evas_object_smart_add(evas, _edrawable_smart_get());
    EDrawable *drawable  = evas_object_smart_data_get(obj);
    if(drawable)
        _edrawable_init(obj, evas, w, h);
    return obj;
}


void
edrawable_draw_line(Evas_Object *obj, int x1, int y1, int x2, int y2) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable->updates = __drawable_AppendUpdates(
        drawable_image_draw_line(drawable->context, x1, y1, x2, y2, 1),
        drawable->updates);
}

void
edrawable_draw_rectangle(Evas_Object *obj, int x1, int y1, int x2, int y2) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_image_draw_rectangle(drawable->context, x1, y1, x2, y2);
    drawable->updates = drawable_update_append_rect(drawable->updates, x1, y1, x1, y2);
}

void
edrawable_draw_rectangle_fill(Evas_Object *obj, int x1, int y1, int x2, int y2) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_image_fill_rectangle(drawable->context, x1, y1, x2, y2);
    drawable->updates = drawable_update_append_rect(drawable->updates, x1, y1, x1, y2);
}

EAPI EDrawablePolygon
ewl_drawable_polygon_new() {
    return (EDrawablePolygon *) drawable_polygon_new();
}

void
edrawable_draw_polygon(Evas_Object *obj, EDrawablePolygon p) {
    int x, y, w, h;
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_image_draw_polygon(drawable->context, p, 0);
    drawable_polygon_get_bounds(p, &x, &y, &w, &h);
    drawable->updates = drawable_update_append_rect(drawable->updates, x, y, w, h);
}

void
edrawable_draw_polygon_fill(Evas_Object *obj, EDrawablePolygon  p) {
    int x, y, w, h;
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_image_fill_polygon(drawable->context, p);
    drawable_polygon_get_bounds(p, &x, &y, &w, &h);
    drawable->updates = drawable_update_append_rect(drawable->updates, x, y, w, h);
}

EAPI void
edrawable_polygon_delete(EDrawablePolygon p) {
    drawable_polygon_free(p);
}

EAPI void
edrawable_polygon_add(EDrawablePolygon p, int x, int y) {
    drawable_polygon_add_point(p, x, y);
}

static void
_update_ellipse(EDrawable *ed, int x, int y, int r1, int r2) {
    int w, h;
    w = x + r1;
    h = w + r2;
    x -= r1;
    y -= r2;
    ed->updates = drawable_update_append_rect(ed->updates, x, y, w, h);
}

EAPI void
edrawable_draw_ellipse(Evas_Object* obj, int x, int y, int r, int r2) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_image_draw_ellipse(drawable->context, x, y, r, r2);
    _update_ellipse(drawable, x, y, r, r2);
}

void
edrawable_draw_ellipse_filled(Evas_Object *obj, int x, int y, int r, int r2) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_image_fill_ellipse(drawable->context, x, y, r, r2);
    _update_ellipse(drawable, x, y, r, r2);
}


void
edrawable_set_colors(Evas_Object *obj, int r, int g, int b, int a) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_context_set_color(drawable->context, r, g, b, a);
}

void
edrawable_set_clip(Evas_Object *obj, int x, int y, int w, int h) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_context_set_cliprect(drawable->context, x, y, w, h);
}

void
edrawable_reset_clip(Evas_Object *obj){
    int w, h;
    EDrawable *drawable = evas_object_smart_data_get(obj);
    w = drawable_image_get_width(drawable->context);
    h = drawable_image_get_height(drawable->context);
    edrawable_set_clip(obj, 0, 0, w, h);
}

void
edrawable_select_font(Evas_Object *obj, const char *fontname, int size) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    printf("load font...\n");
    drawable_load_font(drawable->context, fontname, 0, size);
}

void
edrawable_draw_text(Evas_Object *obj, int x, int y, const char *text) {
    printf("draw...\n");
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_text_draw(drawable->context, x, y, text);
}

void
edrawable_get_text_size(Evas_Object *obj, const char *text, int *vertical, int *horisontal) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    drawable_get_text_size(drawable->context, text, horisontal, vertical);
}

EAPI int
edrawable_get_font_ascent(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    return drawable_get_maximum_font_ascent(drawable->context);
}

int
edrawable_get_font_descent(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
    return drawable_get_maximum_font_descent(drawable->context);
}

void
edrawable_commit(Evas_Object *obj) {
    EDrawable *drawable = evas_object_smart_data_get(obj);
 //   void *in;
//    int w, h;

//    w = drawable_image_get_width(e->context);
//    h = drawable_image_get_height(e->context);
    __drawable_PropagateUpdates(drawable->updates, drawable->image);
    drawable->updates=NULL;
//    evas_object_image_data_update_add(img->image, 0, 0, w, h);
}


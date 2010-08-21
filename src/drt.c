#include <string.h>
#include <err.h>

#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Evas.h>

#include "edrawable.h"

void exit_all(void* param)
{
    ecore_main_loop_quit();
}

static void main_win_key_handler(void* data, Evas* canvas,
                                 Evas_Object* obj, void* event_info)
{
    Evas_Event_Key_Up* e = event_info;

    if(!strcmp(e->key, "space"))
    {
        Evas_Object* dr = evas_object_name_find(canvas, "drawable");

        edrawable_set_colors(dr, 255, 0, 0, 255);
        edrawable_draw_rectangle_fill(dr, 100, 100, 300, 300);
        edrawable_commit(dr);
    }
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
    int w, h;
    Evas* canvas = ecore_evas_get(main_win);
    evas_output_size_get(canvas, &w, &h);

    Evas_Object* dr = evas_object_name_find(canvas, "drawable");
    evas_object_resize(dr, w, h);
}

int main(int argc, char** argv)
{
    if(!evas_init())
        err(1, "Unable to initialize Evas\n");
    if(!ecore_init())
        err(1, "Unable to initialize Ecore\n");
    if(!ecore_evas_init())
        err(1, "Unable to initialize Ecore_Evas\n");

    ecore_x_io_error_handler_set(exit_all, NULL);

    Ecore_Evas* main_win = ecore_evas_software_x11_8_new(0, 0, 0, 0, 600, 800);
    ecore_evas_borderless_set(main_win, 0);
    ecore_evas_shaped_set(main_win, 0);
    ecore_evas_title_set(main_win, "elock");
    ecore_evas_name_class_set(main_win, "elock", "elock");

    ecore_evas_callback_resize_set(main_win, main_win_resize_handler);

    Evas* canvas = ecore_evas_get(main_win);

    Evas_Object* dr = edrawable_add(canvas, 400, 400);
    evas_object_name_set(dr, "drawable");
    evas_object_move(dr, 0, 0);
    evas_object_resize(dr, 400, 400);
    evas_object_show(dr);
    evas_object_focus_set(dr, 1);

    evas_object_event_callback_add(dr, EVAS_CALLBACK_KEY_UP,
                                   &main_win_key_handler, NULL);

    Evas_Object* r = evas_object_rectangle_add(canvas);
    evas_object_move(r, 0, 0);
    evas_object_resize(r, 100, 100);
//    evas_object_show(r);

//    Evas_Object* im = evas_object_image_add(canvas);
//    evas_object_move(im, 100, 100);
//    evas_object_resize(im, 400, 400);
//    evas_object_image_size_set(im, 400, 400);
//    evas_object_image_file_set(im, "/usr/share/pixmaps/apple-green.png", NULL);
//    evas_object_image_fill_set(im, 0, 0, 400, 400);
//    evas_object_show(im);

    ecore_evas_show(main_win);

    ecore_main_loop_begin();

    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();

    return 0;
}

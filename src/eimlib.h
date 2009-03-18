#ifndef __EIMLIB_API_H
#define __EIMLIB_API_H 1

# ifdef EAPI
# undef EAPI
# endif
# ifdef WIN32
#  ifdef BUILDING_DLL
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI __declspec(dllimport)
#  endif
# else
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#   else
#    define EAPI
#   endif
#  else
#   define EAPI
#  endif
# endif

/* Data types to use */
# ifndef DATA64
#  define DATA64 unsigned long long
#  define DATA32 unsigned int
#  define DATA16 unsigned short
#  define DATA8  unsigned char
# endif

/* opaque data types */
typedef void *Drawable_Context;
typedef void *Drawable_Image;
typedef void *Drawable_Color_Modifier;
typedef void *Drawable_Updates;
typedef void *Drawable_Font;
typedef void *Drawable_Color_Range;
typedef void *Drawable_Filter;
typedef struct _drawable_border Drawable_Border;
typedef struct _drawable_color Drawable_Color;
typedef void *DrawablePolygon;

/* blending operations */
enum _drawable_operation
{
   DRAWABLE_OP_COPY,
   DRAWABLE_OP_ADD,
   DRAWABLE_OP_SUBTRACT,
   DRAWABLE_OP_RESHADE
};

enum _drawable_text_direction
{
   DRAWABLE_TEXT_TO_RIGHT = 0,
   DRAWABLE_TEXT_TO_LEFT = 1,
   DRAWABLE_TEXT_TO_DOWN = 2,
   DRAWABLE_TEXT_TO_UP = 3,
   DRAWABLE_TEXT_TO_ANGLE = 4
};

enum _drawable_load_error
{
   DRAWABLE_LOAD_ERROR_NONE,
   DRAWABLE_LOAD_ERROR_FILE_DOES_NOT_EXIST,
   DRAWABLE_LOAD_ERROR_FILE_IS_DIRECTORY,
   DRAWABLE_LOAD_ERROR_PERMISSION_DENIED_TO_READ,
   DRAWABLE_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT,
   DRAWABLE_LOAD_ERROR_PATH_TOO_LONG,
   DRAWABLE_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT,
   DRAWABLE_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY,
   DRAWABLE_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE,
   DRAWABLE_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS,
   DRAWABLE_LOAD_ERROR_OUT_OF_MEMORY,
   DRAWABLE_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS,
   DRAWABLE_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE,
   DRAWABLE_LOAD_ERROR_OUT_OF_DISK_SPACE,
   DRAWABLE_LOAD_ERROR_UNKNOWN
};

/* Encodings known to Imlib2 (so far) */
enum _drawable_TTF_encoding
{
   DRAWABLE_TTF_ENCODING_ISO_8859_1,
   DRAWABLE_TTF_ENCODING_ISO_8859_2,
   DRAWABLE_TTF_ENCODING_ISO_8859_3,
   DRAWABLE_TTF_ENCODING_ISO_8859_4,
   DRAWABLE_TTF_ENCODING_ISO_8859_5
};

typedef enum _drawable_operation Drawable_Operation;
typedef enum _drawable_load_error Drawable_Load_Error;
typedef enum _drawable_load_error DrawableLoadError;
typedef enum _drawable_text_direction Drawable_Text_Direction;
typedef enum _drawable_TTF_encoding Drawable_TTF_Encoding;

struct _drawable_border
{
   int left, right, top, bottom;
};

struct _drawable_color
{
   int alpha, red, green, blue;
};

/* Progressive loading callbacks */
typedef int (*Drawable_Progress_Function) (Drawable_Image im, char percent,
                                        int update_x, int update_y,
                                        int update_w, int update_h);
typedef void (*Drawable_Data_Destructor_Function) (Drawable_Image im, void *data);

# ifdef __cplusplus
extern "C"
{
# endif

/* context handling */
   EAPI Drawable_Context drawable_context_new(void);
   EAPI void drawable_context_free(Drawable_Context context);


/* context setting */
   EAPI void drawable_context_set_dither_mask(Drawable_Context context, char dither_mask);
   EAPI void drawable_context_set_mask_alpha_threshold(Drawable_Context context, int mask_alpha_threshold);
   EAPI void drawable_context_set_anti_alias(Drawable_Context context, char anti_alias);
   EAPI void drawable_context_set_dither(Drawable_Context context, char dither);
   EAPI void drawable_context_set_blend(Drawable_Context context, char blend);
   EAPI void drawable_context_set_color_modifier(Drawable_Context context, Drawable_Color_Modifier color_modifier);
   EAPI void drawable_context_set_operation(Drawable_Context context, Drawable_Operation operation);
   EAPI void drawable_context_set_font(Drawable_Context context, Drawable_Font font);
   EAPI void drawable_context_set_direction(Drawable_Context context, Drawable_Text_Direction direction);
   EAPI void drawable_context_set_angle(Drawable_Context context, double angle);
   EAPI void drawable_context_set_color(Drawable_Context context, int red, int green, int blue, int alpha);
   EAPI void drawable_context_set_color_hsva(Drawable_Context context, float hue, float saturation, float value, int alpha);
   EAPI void drawable_context_set_color_hlsa(Drawable_Context context, float hue, float lightness, float saturation, int alpha);
   EAPI void drawable_context_set_color_cmya(Drawable_Context context, int cyan, int magenta, int yellow, int alpha);
   EAPI void drawable_context_set_color_range(Drawable_Context context, Drawable_Color_Range color_range);
   EAPI void drawable_context_set_progress_function(Drawable_Context context, Drawable_Progress_Function
                                                 progress_function);
   EAPI void drawable_context_set_progress_granularity(Drawable_Context context, char progress_granularity);
   EAPI void drawable_context_set_image(Drawable_Context context, Drawable_Image image);
   EAPI void drawable_context_set_cliprect(Drawable_Context context, int x, int y, int w, int h);
   EAPI void drawable_context_set_TTF_encoding(Drawable_Context context, Drawable_TTF_Encoding encoding);

   EAPI char drawable_context_get_dither_mask(Drawable_Context context);
   EAPI char drawable_context_get_anti_alias(Drawable_Context context);
   EAPI int drawable_context_get_mask_alpha_threshold(Drawable_Context context);
   EAPI char drawable_context_get_dither(Drawable_Context context);
   EAPI char drawable_context_get_blend(Drawable_Context context);
   EAPI Drawable_Color_Modifier drawable_context_get_color_modifier(Drawable_Context context);
   EAPI Drawable_Operation drawable_context_get_operation(Drawable_Context context);
   EAPI Drawable_Font drawable_context_get_font(Drawable_Context context);
   EAPI double drawable_context_get_angle(Drawable_Context context);
   EAPI Drawable_Text_Direction drawable_context_get_direction(Drawable_Context context);
   EAPI void drawable_context_get_color(Drawable_Context context, int *red, int *green, int *blue, int *alpha);
   EAPI void drawable_context_get_color_hsva(Drawable_Context context, float *hue, float *saturation, float *value, int *alpha);
   EAPI void drawable_context_get_color_hlsa(Drawable_Context context, float *hue, float *lightness, float *saturation, int *alpha);
   EAPI void drawable_context_get_color_cmya(Drawable_Context context, int *cyan, int *magenta, int *yellow, int *alpha);
   EAPI Drawable_Color *drawable_context_get_drawable_color(Drawable_Context context);
   EAPI Drawable_Color_Range drawable_context_get_color_range(Drawable_Context context);
   EAPI Drawable_Progress_Function drawable_context_get_progress_function(Drawable_Context context);
   EAPI char drawable_context_get_progress_granularity(Drawable_Context context);
   EAPI Drawable_Image drawable_context_get_image(Drawable_Context context);
   EAPI void drawable_context_get_cliprect(Drawable_Context context, int *x, int *y, int *w, int *h);
   EAPI Drawable_TTF_Encoding drawable_context_get_TTF_encoding(Drawable_Context context);

   EAPI int drawable_get_cache_size(Drawable_Context context);
   EAPI void drawable_set_cache_size(Drawable_Context context, int bytes);
   EAPI int drawable_get_color_usage(Drawable_Context context);
   EAPI void drawable_set_color_usage(Drawable_Context context, int max);
   EAPI void drawable_flush_loaders(void);


/* query/modify image parameters */
   EAPI int drawable_image_get_width(Drawable_Context context);
   EAPI int drawable_image_get_height(Drawable_Context context);
   EAPI const char *drawable_image_get_filename(Drawable_Context context);
   EAPI DATA32 *drawable_image_get_data(Drawable_Context context);
   EAPI DATA32 *drawable_image_get_data_for_reading_only(Drawable_Context context);
   EAPI void drawable_image_put_back_data(Drawable_Context context, DATA32 * data);
   EAPI void drawable_image_set_has_alpha(Drawable_Context context, char has_alpha);
   EAPI void drawable_image_set_alpha(Drawable_Context context, char flag);
   EAPI void drawable_image_query_pixel(Drawable_Context context, int x, int y, Drawable_Color * color_return);
   EAPI void drawable_image_query_pixel_hsva(Drawable_Context context, int x, int y, float *hue, float *saturation, float *value, int *alpha);
   EAPI void drawable_image_query_pixel_hlsa(Drawable_Context context, int x, int y, float *hue, float *lightness, float *saturation, int *alpha);
   EAPI void drawable_image_query_pixel_cmya(Drawable_Context context, int x, int y, int *cyan, int *magenta, int *yellow, int *alpha);

   EAPI void drawable_blend_image_onto_image(Drawable_Context context,
                                          Drawable_Image source_image,
                                          char merge_alpha, int source_x,
                                          int source_y, int source_width,
                                          int source_height, int destination_x,
                                          int destination_y, int destination_width,
                                          int destination_height);

/* creation functions */
   EAPI Drawable_Image drawable_create_image(int width, int height);
   EAPI Drawable_Image drawable_create_image_using_data(int width, int height,
                                                  DATA32 * data);
   EAPI Drawable_Image drawable_create_image_using_copied_data(int width, int height,
                                                         DATA32 * data);
   EAPI Drawable_Image drawable_clone_image(Drawable_Context context);
   EAPI Drawable_Image drawable_create_cropped_image(Drawable_Context context, int x, int y, int width,
                                               int height);
   EAPI Drawable_Image drawable_create_cropped_scaled_image(Drawable_Context context, int source_x, int source_y,
                                                      int source_width,
                                                      int source_height,
                                                      int destination_width,
                                                      int destination_height);

#if 0
/* image modification */
   EAPI void drawable_image_flip_horizontal(void);
   EAPI void drawable_image_flip_vertical(void);
   EAPI void drawable_image_flip_diagonal(void);
   EAPI void drawable_image_orientate(int orientation);
   EAPI void drawable_image_blur(int radius);
   EAPI void drawable_image_sharpen(int radius);
   EAPI void drawable_image_tile_horizontal(void);
   EAPI void drawable_image_tile_vertical(void);
   EAPI void drawable_image_tile(void);
#endif
/* fonts and text */
   EAPI Drawable_Font drawable_load_font(Drawable_Context context, const char *font_name);
   EAPI void drawable_free_font(Drawable_Font font);
#if 0
   /* NB! The four functions below are deprecated. */
   EAPI int drawable_insert_font_into_fallback_chain(Drawable_Font font, Drawable_Font fallback_font);
   EAPI void drawable_remove_font_from_fallback_chain(Drawable_Font fallback_font);
   EAPI Drawable_Font drawable_get_prev_font_in_fallback_chain(Drawable_Font fn);
   EAPI Drawable_Font drawable_get_next_font_in_fallback_chain(Drawable_Font fn);
#endif
   /* NB! The four functions above are deprecated. */
   EAPI void drawable_text_draw(Drawable_Context context, int x, int y, const char *text);
   EAPI void drawable_text_draw_with_return_metrics(Drawable_Context context, int x, int y, const char *text,
                                                 int *width_return,
                                                 int *height_return,
                                                 int *horizontal_advance_return,
                                                 int *vertical_advance_return);
   EAPI void drawable_get_text_size(Drawable_Context context, const char *text, int *width_return,
                                 int *height_return);
   EAPI void drawable_get_text_advance(Drawable_Context context, const char *text, 
			       int *horizontal_advance_return,
			       int *vertical_advance_return);
   EAPI int drawable_get_text_inset(Drawable_Context context, const char *text);
   EAPI void drawable_add_path_to_font_path(const char *path);
   EAPI void drawable_remove_path_from_font_path(const char *path);
   EAPI char **drawable_list_font_path(int *number_return);
   EAPI int drawable_text_get_index_and_location(Drawable_Context context, const char *text, int x, int y,
                                              int *char_x_return,
                                              int *char_y_return,
                                              int *char_width_return,
                                              int *char_height_return);
   EAPI void drawable_text_get_location_at_index(Drawable_Context context, const char *text, int index,
                                              int *char_x_return,
                                              int *char_y_return,
                                              int *char_width_return,
                                              int *char_height_return);
   EAPI char **drawable_list_fonts(int *number_return);
   EAPI void drawable_free_font_list(char **font_list, int number);
   EAPI int drawable_get_font_cache_size(void);
   EAPI void drawable_set_font_cache_size(int bytes);
   EAPI void drawable_flush_font_cache(void);
   EAPI int drawable_get_font_ascent(Drawable_Context context);
   EAPI int drawable_get_font_descent(Drawable_Context context);
   EAPI int drawable_get_maximum_font_ascent(Drawable_Context context);
   EAPI int drawable_get_maximum_font_descent(Drawable_Context context);

/* color modifiers */
   EAPI Drawable_Color_Modifier drawable_create_color_modifier(Drawable_Context context);
   EAPI void drawable_free_color_modifier(Drawable_Color_Modifier);
   EAPI void drawable_modify_color_modifier_gamma(Drawable_Context context, double gamma_value);
   EAPI void drawable_modify_color_modifier_brightness(Drawable_Context context, double brightness_value);
   EAPI void drawable_modify_color_modifier_contrast(Drawable_Context context, double contrast_value);
   EAPI void drawable_set_color_modifier_tables(Drawable_Context context, DATA8 * red_table,
                                             DATA8 * green_table,
                                             DATA8 * blue_table,
                                             DATA8 * alpha_table);
   EAPI void drawable_get_color_modifier_tables(Drawable_Context context, DATA8 * red_table,
                                             DATA8 * green_table,
                                             DATA8 * blue_table,
                                             DATA8 * alpha_table);
   EAPI void drawable_reset_color_modifier(Drawable_Context context);
   EAPI void drawable_apply_color_modifier(Drawable_Context context);
   EAPI void drawable_apply_color_modifier_to_rectangle(Drawable_Context context, int x, int y, int width,
                                                     int height);

/* drawing on images */
   EAPI Drawable_Updates drawable_image_draw_pixel(Drawable_Context context, int x, int y, char make_updates);
   EAPI Drawable_Updates drawable_image_draw_line(Drawable_Context context, int x1, int y1, int x2, int y2,
                                            char make_updates);
   EAPI int drawable_clip_line(Drawable_Context context, int x0, int y0, int x1, int y1, int xmin, int xmax,
                            int ymin, int ymax, int *clip_x0, int *clip_y0,
                            int *clip_x1, int *clip_y1);
   EAPI void drawable_image_draw_rectangle(Drawable_Context context, int x, int y, int width, int height);
   EAPI void drawable_image_fill_rectangle(Drawable_Context context, int x, int y, int width, int height);
   EAPI void drawable_image_copy_alpha_to_image(Drawable_Context context, Drawable_Image image_source, int x,
                                             int y);
   EAPI void drawable_image_copy_alpha_rectangle_to_image(Drawable_Context context, Drawable_Image image_source,
                                                       int x, int y, int width,
                                                       int height,
                                                       int destination_x,
                                                       int destination_y);
   EAPI void drawable_image_scroll_rect(Drawable_Context context, int x, int y, int width, int height,
                                     int delta_x, int delta_y);
   EAPI void drawable_image_copy_rect(Drawable_Context context, int x, int y, int width, int height, int new_x,
                                   int new_y);

/* polygons */
   EAPI DrawablePolygon drawable_polygon_new(void);
   EAPI void drawable_polygon_free(DrawablePolygon poly);
   EAPI void drawable_polygon_add_point(DrawablePolygon poly, int x, int y);
   EAPI void drawable_image_draw_polygon(Drawable_Context context, DrawablePolygon poly, unsigned char closed);
   EAPI void drawable_image_fill_polygon(Drawable_Context context, DrawablePolygon poly);
   EAPI void drawable_polygon_get_bounds(DrawablePolygon poly, int *px1, int *py1,
                                      int *px2, int *py2);
   EAPI unsigned char drawable_polygon_contains_point(DrawablePolygon poly, int x,
                                                   int y);

/* ellipses */
   EAPI void drawable_image_draw_ellipse(Drawable_Context context, int xc, int yc, int a, int b);
   EAPI void drawable_image_fill_ellipse(Drawable_Context context, int xc, int yc, int a, int b);

#if 0
/* color ranges */
   EAPI Drawable_Color_Range drawable_create_color_range(void);
   EAPI void drawable_free_color_range(void);
   EAPI void drawable_add_color_to_color_range(int distance_away);
   EAPI void drawable_image_fill_color_range_rectangle(int x, int y, int width,
                                                    int height, double angle);
   EAPI void drawable_image_fill_hsva_color_range_rectangle(int x, int y, int width,
                                                         int height, double angle);


   EAPI void drawable_blend_image_onto_image_at_angle(Drawable_Image source_image,
                                                   char merge_alpha, int source_x,
                                                   int source_y, int source_width,
                                                   int source_height,
                                                   int destination_x,
                                                   int destination_y, int angle_x,
                                                   int angle_y);
   EAPI void drawable_blend_image_onto_image_skewed(Drawable_Image source_image,
                                                 char merge_alpha, int source_x,
                                                 int source_y, int source_width,
                                                 int source_height,
                                                 int destination_x,
                                                 int destination_y, int h_angle_x,
                                                 int h_angle_y, int v_angle_x,
                                                 int v_angle_y);

#endif
# ifdef __cplusplus
}
# endif

#endif

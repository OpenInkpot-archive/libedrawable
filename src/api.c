#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "colormod.h"
#include "image.h"
#include "scale.h"
#include "blend.h"
#include "span.h"
#include "updates.h"
#include "rgbadraw.h"

#include "eimlib.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include "font.h"
//#include "grad.h"
#include "rotate.h"
#include <math.h>
#include "color_helpers.h"

/* convenience macros */
#define   CAST_IMAGE(im, image) (im) = (DrawableImage *)(image)
#define   CHECK_PARAM_POINTER_RETURN(func, sparam, param, ret) \
if (!(param)) \
{ \
  fprintf(stderr, "***** Drawable2 Developer Warning ***** :\n" \
                  "\tThis program is calling the Drawable call:\n\n" \
                  "\t%s();\n\n" \
                  "\tWith the parameter:\n\n" \
                  "\t%s\n\n" \
                  "\tbeing NULL. Please fix your program.\n", func, sparam); \
  return ret; \
}

#define   CHECK_PARAM_POINTER(func, sparam, param) \
if (!(param)) \
{ \
  fprintf(stderr, "***** Drawable2 Developer Warning ***** :\n" \
                  "\tThis program is calling the Drawable call:\n\n" \
                  "\t%s();\n\n" \
                  "\tWith the parameter:\n\n" \
                  "\t%s\n\n" \
                  "\tbeing NULL. Please fix your program.\n", func, sparam); \
  return; \
}

/* internal typedefs for function pointers */
typedef void        (*Drawable_Internal_Progress_Function) (void *, char, int, int,
                                                         int, int);
typedef void        (*Drawable_Internal_Data_Destructor_Function) (void *, void *);

struct _drawablecontext;
typedef struct _drawablecontext DrawableContext;

struct _drawablecontext {
   char                anti_alias;
   char                dither;
   char                blend;
   Drawable_Color_Modifier color_modifier;
   Drawable_Operation     operation;
   Drawable_Font          font;
   Drawable_Text_Direction direction;
   double              angle;
   Drawable_Color         color;
   Drawable_Color_Range   color_range;
   Drawable_Image         image;
   Drawable_Progress_Function progress_func;
   char                progress_granularity;
   char                dither_mask;
   int                 mask_alpha_threshold;
   Drawable_Filter        filter;
   Drawable_Rectangle     cliprect;
   Drawable_TTF_Encoding  encoding;

   int                 references;
   char                dirty;
};


/* frees the given context including all its members */
static void
__drawable_free_context(DrawableContext * ctx)
{
   if (ctx->image)
     {
        __drawable_FreeImage(ctx->image);
        ctx->image = NULL;
     }
   if (ctx->font)
     {
        drawable_free_font(ctx->font);
        ctx->font = NULL;
     }
#if 0
   if (ctx->color_modifier)
     {
        drawable_free_color_modifier(ctx->color_modifier);
        ctx->color_modifier = NULL;
     }
#endif
   free(ctx);
}

EAPI                Drawable_Context
drawable_context_new(void)
{
   DrawableContext       *context = malloc(sizeof(DrawableContext));

   context->anti_alias = 1;
   context->dither = 0;
   context->blend = 1;
   context->color_modifier = NULL;
   context->operation = DRAWABLE_OP_COPY;
   context->font = NULL;
   context->direction = DRAWABLE_TEXT_TO_RIGHT;
   context->angle = 0.0;
   context->color.alpha = 255;
   context->color.red = 255;
   context->color.green = 255;
   context->color.blue = 255;
   context->color_range = NULL;
   context->image = NULL;
   context->progress_func = NULL;
   context->progress_granularity = 0;
   context->dither_mask = 0;
   context->mask_alpha_threshold = 128;
   context->filter = NULL;
   context->cliprect.x = 0;
   context->cliprect.y = 0;
   context->cliprect.w = 0;
   context->cliprect.h = 0;
   context->encoding = DRAWABLE_TTF_ENCODING_ISO_8859_1;

   context->references = 0;
   context->dirty = 0;

   return (Drawable_Context) context;
}


/* frees the given context if it doesn't have any reference anymore. The
   last (default) context can never be freed.
   If context is the current context, the context below will be made the
   current context.
*/
EAPI void
drawable_context_free(Drawable_Context context)
{
   DrawableContext       *c = (DrawableContext *) context;

   CHECK_PARAM_POINTER("drawable_context_free", "context", context);

   if (c->references == 0)
      __drawable_free_context(c);
   else
      c->dirty = 1;
}

/* context setting/getting functions */

/**
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 *
 * Sets the rectangle of the current context.
 **/
EAPI void
drawable_context_set_cliprect(Drawable_Context context,int x, int y, int w, int h)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->cliprect.x = x;
   ctx->cliprect.y = y;
   ctx->cliprect.w = w;
   ctx->cliprect.h = h;
}

EAPI void
drawable_context_get_cliprect(Drawable_Context context,int *x, int *y, int *w, int *h)
{
   DrawableContext *ctx = (DrawableContext *) context;
   *x = ctx->cliprect.x;
   *y = ctx->cliprect.y;
   *w = ctx->cliprect.w;
   *h = ctx->cliprect.h;
}


/**
 * @param dither_mask The dither mask flag.
 *
 * Selects if, you are rendering to a mask, or producing pixmap masks
 * from images, if the mask is to be dithered or not. passing in 1 for
 * dither_mask means the mask pixmap will be dithered, 0 means it will
 * not be dithered.
 */
EAPI void
drawable_context_set_dither_mask(Drawable_Context context, char dither_mask)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->dither_mask = dither_mask;
}

/**
 * @return The current dither mask flag.
 *
 * Returns the current mode for dithering pixmap masks. 1 means
 * dithering is enabled and 0 means it is not.
 */
EAPI char
drawable_context_get_dither_mask(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->dither_mask;
}

/**
 * @param mask_alpha_threshold The mask alpha threshold.
 *
 * Selects, if you are rendering to a mask, the alpha threshold above which
 * mask bits are set. The default mask alpha threshold is 128, meaning that
 * a mask bit will be set if the pixel alpha is >= 128.
 */
EAPI void
drawable_context_set_mask_alpha_threshold(Drawable_Context context, int mask_alpha_threshold)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->mask_alpha_threshold = mask_alpha_threshold;
}

/**
 * @return The current mask mask alpha threshold.
 *
 * Returns the current mask alpha threshold.
 */
EAPI int
drawable_context_get_mask_alpha_threshold(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->mask_alpha_threshold;
}

/**
 * @param anti_alias The anti alias flag.
 *
 * Toggles "anti-aliased" scaling of images. This
 * isn't quite correct since it's actually super and sub pixel
 * sampling that it turns on and off, but anti-aliasing is used for
 * having "smooth" edges to lines and shapes and this means when
 * images are scaled they will keep their smooth appearance. Passing
 * in 1 turns this on and 0 turns it off.
 */
EAPI void
drawable_context_set_anti_alias(Drawable_Context context, char anti_alias)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->anti_alias = anti_alias;
}

/**
 * @return The current anti alias flag.
 *
 * Returns if Drawable2 currently will smoothly scale images. 1 means it
 * will and 0 means it will not.
 */
EAPI char
drawable_context_get_anti_alias(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->anti_alias;
}

/**
 * @param dither The dithering flag.
 *
 * Sets the dithering flag for rendering to a drawable or when pixmaps
 * are produced. This affects the color image appearance by enabling
 * dithering. Dithering slows down rendering but produces considerably
 * better results. this option has no effect foe rendering in 24 bit
 * and up, but in 16 bit and lower it will dither, producing smooth
 * gradients and much better quality images. setting dither to 1
 * enables it and 0 disables it.
 */
EAPI void
drawable_context_set_dither(Drawable_Context context, char dither)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->dither = dither;
}

/**
 * @return The current dithering flag.
 *
 * Returns if image data is rendered with dithering currently. 1 means
 * yes and 0 means no.
 */
EAPI char
drawable_context_get_dither(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->dither;
}

/**
 * @param blend The blending flag.
 *
 * When rendering an image to a drawable, Drawable2 is able to blend the
 * image directly onto the drawable during rendering. Setting this to 1
 * will enable this. If the image has no alpha channel this has no
 * effect. Setting it to 0 will disable this.
 */
EAPI void
drawable_context_set_blend(Drawable_Context context, char blend)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->blend = blend;
}

/**
 * @return The current blending flag.
 *
 * Returns if Drawable2 will blend images onto a drawable whilst
 * rendering to that drawable. 1 means yes and 0 means no.
 */
EAPI char
drawable_context_get_blend(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->blend;
}

#if 0
/**
 * @param color_modifier Current color modifier.
 *
 * Sets the current color modifier used for rendering pixmaps or
 * images to a drawable or images onto other images. Color modifiers
 * are lookup tables that map the values in the red, green, blue and
 * alpha channels to other values in the same channel when rendering,
 * allowing for fades, color correction etc. to be done whilst
 * rendering. pass in NULL as the color_modifier to disable the color
 * modifier for rendering.
 */
EAPI void
drawable_context_set_color_modifier(Drawable_Context context, Drawable_Color_Modifier color_modifier)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->color_modifier = color_modifier;
}

/**
 * @return The current color modifier.
 *
 * Returns the current color modifier being used.
 */
EAPI                Drawable_Color_Modifier
drawable_context_get_color_modifier(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->color_modifier;
}
#endif

/**
 * @param operation
 *
 * When Drawable2 draws an image onto another or an image onto a drawable
 * it is able to do more than just blend the result on using the given
 * alpha channel of the image. It is also able to do saturating
 * additive, subtractive and a combination of the both (called reshade)
 * rendering. The default mode is DRAWABLE_OP_COPY. you can also set it to
 * DRAWABLE_OP_ADD, DRAWABLE_OP_SUBTRACT or DRAWABLE_OP_RESHADE. Use this
 * function to set the rendering operation. DRAWABLE_OP_COPY performs
 * basic alpha blending: DST = (SRC * A) + (DST * (1 -
 * A)). DRAWABLE_OP_ADD does DST = DST + (SRC * A). DRAWABLE_OP_SUBTRACT does
 * DST = DST - (SRC * A) and DRAWABLE_OP_RESHADE does DST = DST + (((SRC -
 * 0.5) / 2) * A).
 */
EAPI void
drawable_context_set_operation(Drawable_Context context, Drawable_Operation operation)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->operation = operation;
}

/**
 * @return The current operation mode.
 *
 * Returns the current operation mode.
 */
EAPI                Drawable_Operation
drawable_context_get_operation(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->operation;
}

/**
 * @param font Current font.
 *
 * Sets the current font to use when rendering text. you should load
 * the font first with drawable_load_font().
 */
EAPI void
drawable_context_set_font(Drawable_Context context, Drawable_Font font)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->font = font;
}

/**
 * @return The current font.
 *
 * Returns the current font.
 */
EAPI                Drawable_Font
drawable_context_get_font(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->font;
}

/**
 * @param direction Text direction.
 *
 * Sets the direction in which to draw text in terms of simple 90
 * degree orientations or an arbitrary angle. The direction can be one
 * of DRAWABLE_TEXT_TO_RIGHT, DRAWABLE_TEXT_TO_LEFT, DRAWABLE_TEXT_TO_DOWN,
 * DRAWABLE_TEXT_TO_UP or DRAWABLE_TEXT_TO_ANGLE. The default is
 * DRAWABLE_TEXT_TO_RIGHT. If you use DRAWABLE_TEXT_TO_ANGLE, you will also
 * have to set the angle with drawable_context_set_angle().
 */
EAPI void
drawable_context_set_direction(Drawable_Context context, Drawable_Text_Direction direction)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->direction = direction;
}

/**
 * @param angle Angle of the text strings.
 *
 * Sets the angle at which text strings will be drawn if the text
 * direction has been set to DRAWABLE_TEXT_TO_ANGLE with
 * drawable_context_set_direction().
 */
EAPI void
drawable_context_set_angle(Drawable_Context context, double angle)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->angle = angle;
}

/**
 * @return The current angle of the text strings.
 *
 * Returns the current angle used to render text at if the direction
 * is DRAWABLE_TEXT_TO_ANGLE.
 */
EAPI double
drawable_context_get_angle(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->angle;
}

/**
 * @return The current direction of the text.
 *
 * Returns the current direction to render text in.
 */
EAPI                Drawable_Text_Direction
drawable_context_get_direction(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->direction;
}

/**
 * @param red Red channel of the current color.
 * @param green Green channel of the current color.
 * @param blue Blue channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Sets the color with which text, lines and rectangles are drawn when
 * being rendered onto an image. Values for @p red, @p green, @p blue
 * and @p alpha are between 0 and 255 - any other values have
 * undefined results.
 */
EAPI void
drawable_context_set_color(Drawable_Context context, int red, int green, int blue, int alpha)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->color.red = red;
   ctx->color.green = green;
   ctx->color.blue = blue;
   ctx->color.alpha = alpha;
}

/**
 * @param red Red channel of the current color.
 * @param green Green channel of the current color.
 * @param blue Blue channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Returns the current color for rendering text, rectangles and lines.
 */
EAPI void
drawable_context_get_color(Drawable_Context context, int *red, int *green, int *blue, int *alpha)
{
   DrawableContext *ctx = (DrawableContext *) context;
   *red = ctx->color.red;
   *green = ctx->color.green;
   *blue = ctx->color.blue;
   *alpha = ctx->color.alpha;
}

/**
 * @return The current color.
 *
 * Returns the current color as a color struct. Do NOT free this
 * pointer.
 */
EAPI Drawable_Color   *
drawable_context_get_drawable_color(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return &ctx->color;
}

/**
 * @param hue Hue channel of the current color.
 * @param saturation Saturation channel of the current color.
 * @param value Value channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Sets the color in HSVA space. Values for @p hue are between 0 and 360,
 * values for @p saturation and @p value between 0 and 1, and values for
 * @p alpha are between 0 and 255 - any other values have undefined
 * results.
 */
EAPI void
drawable_context_set_color_hsva(Drawable_Context context, float hue, float saturation, float value,
                             int alpha)
{
   int                 r, g, b;

   __drawable_hsv_to_rgb(hue, saturation, value, &r, &g, &b);
   drawable_context_set_color(context, r, g, b, alpha);
}

/**
 * @param hue Hue channel of the current color.
 * @param saturation Saturation channel of the current color.
 * @param value Value channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Returns the current color for rendering text, rectangles and lines
 * in HSVA space.
 */
EAPI void
drawable_context_get_color_hsva(Drawable_Context context, float *hue, float *saturation, float *value,
                             int *alpha)
{
   int                 r, g, b;

   drawable_context_get_color(context, &r, &g, &b, alpha);
   __drawable_rgb_to_hsv(r, g, b, hue, saturation, value);
}

/**
 * @param hue Hue channel of the current color.
 * @param lightness Lightness channel of the current color.
 * @param saturation Saturation channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Sets the color in HLSA space. Values for @p hue are between 0 and 360,
 * values for @p lightness and @p saturation between 0 and 1, and values for
 * @p alpha are between 0 and 255 - any other values have undefined
 * results.
 */
EAPI void
drawable_context_set_color_hlsa(Drawable_Context context, float hue, float lightness, float saturation,
                             int alpha)
{
   int                 r, g, b;

   __drawable_hls_to_rgb(hue, lightness, saturation, &r, &g, &b);
   drawable_context_set_color(context, r, g, b, alpha);
}

/**
 * @param hue Hue channel of the current color.
 * @param lightness Lightness channel of the current color.
 * @param saturation Saturation channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Returns the current color for rendering text, rectangles and lines
 * in HLSA space.
 */
EAPI void
drawable_context_get_color_hlsa(Drawable_Context context, float *hue, float *lightness, float *saturation,
                             int *alpha)
{
   int                 r, g, b;

   drawable_context_get_color(context, &r, &g, &b, alpha);
   __drawable_rgb_to_hls(r, g, b, hue, lightness, saturation);
}

/**
 * @param cyan Cyan channel of the current color.
 * @param magenta Magenta channel of the current color.
 * @param yellow Yellow channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Sets the color in CMYA space. Values for @p cyan, @p magenta, @p yellow and
 * @p alpha are between 0 and 255 - any other values have undefined
 * results.
 */
EAPI void
drawable_context_set_color_cmya(Drawable_Context context, int cyan, int magenta, int yellow, int alpha)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->color.red = 255 - cyan;
   ctx->color.green = 255 - magenta;
   ctx->color.blue = 255 - yellow;
   ctx->color.alpha = alpha;
}

/**
 * @param cyan Cyan channel of the current color.
 * @param magenta Magenta channel of the current color.
 * @param yellow Yellow channel of the current color.
 * @param alpha Alpha channel of the current color.
 *
 * Returns the current color for rendering text, rectangles and lines
 * in CMYA space.
 */
EAPI void
drawable_context_get_color_cmya(Drawable_Context context, int *cyan, int *magenta, int *yellow, int *alpha)
{
   DrawableContext *ctx = (DrawableContext *) context;
   *cyan = 255 - ctx->color.red;
   *magenta = 255 - ctx->color.green;
   *yellow = 255 - ctx->color.blue;
   *alpha = ctx->color.alpha;
}

#if 0
/**
 * @param color_range Color range.
 *
 * Sets the current color range to use for rendering gradients.
 */
EAPI void
drawable_context_set_color_range(Drawable_Color_Range color_range)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->color_range = color_range;
}

/**
 * @return The current color range.
 *
 * Returns the current color range being used for gradients.
 */
EAPI                Drawable_Color_Range
drawable_context_get_color_range(void)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->color_range;
}

#endif

/**
 * @param progress_function A progress function.
 *
 * Sets the progress function to be called back whilst loading
 * images. Set this to the function to be called, or set it to NULL to
 * disable progress callbacks whilst loading.
 */
EAPI void
drawable_context_set_progress_function(Drawable_Context context, Drawable_Progress_Function progress_function)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->progress_func = progress_function;
}

/**
 * @return The current progress function.
 *
 * Returns the current progress function being used.
 */
EAPI                Drawable_Progress_Function
drawable_context_get_progress_function(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->progress_func;
}

/**
 * @param progress_granularity A char.
 *
 * This hints as to how often to call the progress callback. 0 means
 * as often as possible. 1 means whenever 15 more of the image has been
 * decoded, 10 means every 10% of the image decoding, 50 means every
 * 50% and 100 means only call at the end. Values outside of the range
 * 0-100 are undefined.
 */
EAPI void
drawable_context_set_progress_granularity(Drawable_Context context, char progress_granularity)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->progress_granularity = progress_granularity;
}

/**
 * @return The current progress granularity
 *
 * Returns the current progress granularity being used.
 */
EAPI char
drawable_context_get_progress_granularity(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->progress_granularity;
}

/**
 * @param image Current image.
 *
 * Sets the current image Drawable2 will be using with its function calls.
 */
EAPI void
drawable_context_set_image(Drawable_Context context, Drawable_Image image)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->image = image;
}

/**
 * @return The current image.
 *
 * Returns the current context image.
 */
EAPI                Drawable_Image
drawable_context_get_image(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->image;
}

EAPI void
drawable_context_set_TTF_encoding(Drawable_Context context, Drawable_TTF_Encoding encoding)
{
   DrawableContext *ctx = (DrawableContext *) context;
   ctx->encoding = encoding;
}

EAPI                Drawable_TTF_Encoding
drawable_context_get_TTF_encoding(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return ctx->encoding;
}

/* imlib api */


/**
 * Frees the image that is set as the current image in Drawable2's context.
 */
EAPI void
drawable_free_image(DrawableImage * i)
{
   __drawable_FreeImage((DrawableImage *)i);
}

/**
 * Frees the current image in Drawable2's context AND removes it from the
 * cache.
 */
EAPI void
drawable_free_image_and_decache(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_free_image_and_decache", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   SET_FLAG(im->flags, F_INVALID);
   __drawable_FreeImage(im);
   ctx->image = NULL;
}

/**
 * Returns the width in pixels of the current image in Drawable2's context.
 */
EAPI int
drawable_image_get_width(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_get_width", "image", ctx->image, 0);
   CAST_IMAGE(im, ctx->image);
   return im->w;
}

/**
 * Returns the height in pixels of the current image in Drawable2's context.
 */
EAPI int
drawable_image_get_height(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_get_height", "image", ctx->image, 0);
   CAST_IMAGE(im, ctx->image);
   return im->h;
}


/**
 * @return A pointer to the image data.
 *
 * Returns a pointer to the image data in the image set as the image
 * for the current context. When you get this pointer it is assumed you
 * are planning on writing to the data, thus once you do this the image
 * can no longer be used for caching - in fact all images cached from
 * this one will also be affected when you put the data back. If this
 * matters it is suggested you clone the image first before playing
 * with the image data. The image data is returned in the format of a
 * DATA32 (32 bits) per pixel in a linear array ordered from the top
 * left of the image to the bottom right going from left to right each
 * line. Each pixel has the upper 8 bits as the alpha channel and the
 * lower 8 bits are the blue channel - so a pixel's bits are ARGB (from
 * most to least significant, 8 bits per channel). You must put the
 * data back at some point.
 */
EAPI DATA32        *
drawable_image_get_data(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_get_data", "image", ctx->image,
                              NULL);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!im->data)
      return NULL;
   __drawable_DirtyImage(im);
   return im->data;
}

/**
 * @return A pointer to the image data.
 *
 * Functions the same way as drawable_image_get_data(), but returns a
 * pointer expecting the program to NOT write to the data returned (it
 * is for inspection purposes only). Writing to this data has undefined
 * results. The data does not need to be put back.
 */
EAPI DATA32        *
drawable_image_get_data_for_reading_only(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_get_data_for_reading_only",
                              "image", ctx->image, NULL);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!im->data)
      return NULL;
   return im->data;
}

/**
 * @param data The pointer to the image data.
 *
 * Puts back @p data when it was obtained by
 * drawable_image_get_data(). @p data must be the same pointer returned
 * by drawable_image_get_data(). This operated on the current context
 * image.
 */
EAPI void
drawable_image_put_back_data(Drawable_Context context, DATA32 * data)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_put_back_data", "image", ctx->image);
   CHECK_PARAM_POINTER("drawable_image_put_back_data", "data", data);
   CAST_IMAGE(im, ctx->image);
   __drawable_DirtyImage(im);
   data = NULL;
}

/**
 * @return Current alpha channel flag.
 *
 * Returns 1 if the current context image has an alpha channel, or 0
 * if it does not (the alpha data space is still there and available -
 * just "unused").
 */
EAPI char
drawable_image_has_alpha(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_has_alpha", "image", ctx->image, 0);
   CAST_IMAGE(im, ctx->image);
   if (IMAGE_HAS_ALPHA(im))
      return 1;
   return 0;
}

EAPI void
drawable_image_set_alpha(Drawable_Context context, char flag)
{
   DrawableContext *ctx = (DrawableContext *) context;
    __drawable_SetImageAlphaFlag(ctx->image, flag);
}

#if 0
/**
 * @param border The border of the image.
 *
 * Fills the Drawable_Border structure to which @p border points to with the
 * values of the border of the current context image. The border is the
 * area at the edge of the image that does not scale with the rest of
 * the image when resized - the borders remain constant in size. This
 * is useful for scaling bevels at the edge of images differently to
 * the image center.
 */
EAPI void
drawable_image_get_border(Drawable_Context context, Drawable_Border * border)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_get_border", "image", ctx->image);
   CHECK_PARAM_POINTER("drawable_image_get_border", "border", border);
   CAST_IMAGE(im, ctx->image);
   border->left = im->border.left;
   border->right = im->border.right;
   border->top = im->border.top;
   border->bottom = im->border.bottom;
}

/**
 * @param border The border of the image.
 *
 * Sets the border of the current context image to the values contained
 * in the Drawable_Border structure @p border points to.
 */
EAPI void
drawable_image_set_border(Drawable_Context context, Drawable_Border * border)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_set_border", "image", ctx->image);
   CHECK_PARAM_POINTER("drawable_image_set_border", "border", border);
   CAST_IMAGE(im, ctx->image);
   if ((im->border.left == border->left)
       && (im->border.right == border->right)
       && (im->border.top == border->top)
       && (im->border.bottom == border->bottom))
      return;
   im->border.left = border->left;
   im->border.right = border->right;
   im->border.top = border->top;
   im->border.bottom = border->bottom;
   __drawable_DirtyPixmapsForImage(im);
}
#endif

/**
 * @param source_image The source image.
 * @param merge_alpha Alpha flag.
 * @param source_x X coordinate of the source image.
 * @param source_y Y coordinate of the source image.
 * @param source_width Width of the source image.
 * @param source_height Height of the source image.
 * @param destination_x X coordinate of the destination image.
 * @param destination_y Y coordinate of the destination image.
 * @param destination_width Width of the destination image.
 * @param destination_height Height of the destination image.
 *
 * Blends the source rectangle (@p source_x, @p source_y, @p
 * source_width, @p source_height) from
 * @p source_image onto the current image at the destination (@p
 * destination_x, @p destination_y) location
 * scaled to the width @p destination_width and height @p
 * destination_height. If @p merge_alpha is set to 1
 * it will also modify the destination image alpha channel, otherwise
 * the destination alpha channel is left untouched.
 */
EAPI void
drawable_blend_image_onto_image(Drawable_Context context, Drawable_Image source_image, char merge_alpha,
                             int source_x, int source_y, int source_width,
                             int source_height, int destination_x,
                             int destination_y, int destination_width,
                             int destination_height)
{
   DrawableImage         *im_src, *im_dst;
   int                 aa;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_blend_image_onto_image", "source_image",
                       source_image);
   CHECK_PARAM_POINTER("drawable_blend_image_onto_image", "image", ctx->image);
   CAST_IMAGE(im_src, source_image);
   CAST_IMAGE(im_dst, ctx->image);
   if ((!(im_src->data)) && (im_src->loader) && (im_src->loader->load))
      im_src->loader->load(im_src, NULL, 0, 1);
   if (!(im_src->data))
      return;
   if ((!(im_dst->data)) && (im_dst->loader) && (im_dst->loader->load))
      im_dst->loader->load(im_dst, NULL, 0, 1);
   if (!(im_dst->data))
      return;
   __drawable_DirtyImage(im_dst);
   /* FIXME: hack to get around infinite loops for scaling down too far */
   aa = ctx->anti_alias;
   if ((abs(destination_width) < (source_width >> 7))
       || (abs(destination_height) < (source_height >> 7)))
      aa = 0;
   __drawable_BlendImageToImage(im_src, im_dst, aa, ctx->blend,
                             merge_alpha, source_x, source_y, source_width,
                             source_height, destination_x, destination_y,
                             destination_width, destination_height,
                             ctx->color_modifier, ctx->operation,
                             ctx->cliprect.x, ctx->cliprect.y,
                             ctx->cliprect.w, ctx->cliprect.h);
}

/**
 * @param width The width of the image.
 * @param height The height of the image.
 * @return A new blank image.
 *
 * Creates a new blank image of size @p width and @p height. The contents of
 * this image at creation time are undefined (they could be garbage
 * memory). You are free to do whatever you like with this image. It
 * is not cached. On success an image handle is returned - on failure
 * NULL is returned.
 **/
EAPI                Drawable_Image
drawable_create_image(int width, int height)
{
   DATA32             *data;

   if ((width <= 0) || (height <= 0))
      return NULL;
   data = malloc(width * height * sizeof(DATA32));
   if (data)
      return (Drawable_Image) __drawable_CreateImage(width, height, data);
   return NULL;
}

/**
 * @param width The width of the image.
 * @param height The height of the image.
 * @param data The data.
 * @return A valid image, otherwise NULL.
 *
 * Creates an image from the image data specified with the width @p width and
 * the height @p height specified. The image data @p data must be in the same format as
 * drawable_image_get_data() would return. You are responsible for
 * freeing this image data once the image is freed - Drawable2 will not
 * do that for you. This is useful for when you already have static
 * buffers of the same format Drawable2 uses (many video grabbing devices
 * use such a format) and wish to use Drawable2 to render the results
 * onto another image, or X drawable. You should free the image when
 * you are done with it. Drawable2 returns a valid image handle on
 * success or NULL on failure
 *
 **/
EAPI                Drawable_Image
drawable_create_image_using_data(int width, int height, DATA32 * data)
{
   DrawableImage         *im;

   CHECK_PARAM_POINTER_RETURN("drawable_create_image_using_data", "data", data,
                              NULL);
   if ((width <= 0) || (height <= 0))
      return NULL;
   im = __drawable_CreateImage(width, height, data);
   if (im)
      SET_FLAG(im->flags, F_DONT_FREE_DATA);
   return (Drawable_Image) im;
}

/**
 * @param width The width of the image.
 * @param height The height of the image.
 * @param data The data.
 * @return A valid image, otherwise NULL.
 *
 * Works the same way as drawable_create_image_using_data() but Drawable2
 * copies the image data to the image structure. You may now do
 * whatever you wish with the original data as it will not be needed
 * anymore. Drawable2 returns a valid image handle on success or NULL on
 * failure.
 *
 **/
EAPI                Drawable_Image
drawable_create_image_using_copied_data(int width, int height, DATA32 * data)
{
   DrawableImage         *im;

   CHECK_PARAM_POINTER_RETURN("drawable_create_image_using_copied_data", "data",
                              data, NULL);
   if ((width <= 0) || (height <= 0))
      return NULL;
   im = __drawable_CreateImage(width, height, NULL);
   if (!im)
      return NULL;
   im->data = malloc(width * height * sizeof(DATA32));
   if (data)
     {
        memcpy(im->data, data, width * height * sizeof(DATA32));
        return (Drawable_Image) im;
     }
   else
      __drawable_FreeImage(im);
   return NULL;
}


/**
 * @return A valid image, otherwise NULL.
 *
 * Creates an exact duplicate of the current image and returns a valid
 * image handle on success, or NULL on failure.
 *
 **/
EAPI                Drawable_Image
drawable_clone_image(Drawable_Context context)
{
   DrawableImage         *im, *im_old;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_clone_image", "image", ctx->image, NULL);
   CAST_IMAGE(im_old, ctx->image);
   if ((!(im_old->data)) && (im_old->loader) && (im_old->loader->load))
      im_old->loader->load(im_old, NULL, 0, 1);
   if (!(im_old->data))
      return NULL;
   im = __drawable_CreateImage(im_old->w, im_old->h, NULL);
   if (!(im))
      return NULL;
   im->data = malloc(im->w * im->h * sizeof(DATA32));
   if (!(im->data))
     {
        __drawable_FreeImage(im);
        return NULL;
     }
   memcpy(im->data, im_old->data, im->w * im->h * sizeof(DATA32));
   im->flags = im_old->flags;
   SET_FLAG(im->flags, F_UNCACHEABLE);
   im->moddate = im_old->moddate;
   im->border = im_old->border;
   im->loader = im_old->loader;
   if (im_old->format)
     {
        im->format = malloc(strlen(im_old->format) + 1);
        strcpy(im->format, im_old->format);
     }
   if (im_old->file)
     {
        im->file = malloc(strlen(im_old->file) + 1);
        strcpy(im->file, im_old->file);
     }
   return (Drawable_Image) im;
}

/**
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @return A valid image, otherwise NULL.
 *
 * Creates a duplicate of a (@p x, @p y, @p width, @p height) rectangle in the
 * current image and returns a valid image handle on success, or NULL
 * on failure.
 *
 **/
EAPI                Drawable_Image
drawable_create_cropped_image(Drawable_Context context, int x, int y, int width, int height)
{
   DrawableImage         *im, *im_old;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_create_cropped_image", "image",
                              ctx->image, NULL);
   CAST_IMAGE(im_old, ctx->image);
   if ((!(im_old->data)) && (im_old->loader) && (im_old->loader->load))
      im_old->loader->load(im_old, NULL, 0, 1);
   if (!(im_old->data))
      return NULL;
   im = __drawable_CreateImage(abs(width), abs(height), NULL);
   im->data = malloc(abs(width * height) * sizeof(DATA32));
   if (!(im->data))
     {
        __drawable_FreeImage(im);
        return NULL;
     }
   if (IMAGE_HAS_ALPHA(im_old))
     {
        SET_FLAG(im->flags, F_HAS_ALPHA);
        __drawable_BlendImageToImage(im_old, im, 0, 0, 1, x, y, abs(width),
                                  abs(height), 0, 0, width, height, NULL,
                                  DRAWABLE_OP_COPY,
                                  ctx->cliprect.x, ctx->cliprect.y,
                                  ctx->cliprect.w, ctx->cliprect.h);
     }
   else
     {
        __drawable_BlendImageToImage(im_old, im, 0, 0, 0, x, y, abs(width),
                                  abs(height), 0, 0, width, height, NULL,
                                  DRAWABLE_OP_COPY,
                                  ctx->cliprect.x, ctx->cliprect.y,
                                  ctx->cliprect.w, ctx->cliprect.h);
     }
   return (Drawable_Image) im;
}

/**
 * @param source_x The top left x coordinate of the source rectangle.
 * @param source_y The top left y coordinate of the source rectangle.
 * @param source_width The width of the source rectangle.
 * @param source_height The height of the source rectangle.
 * @param destination_width The width of the destination image.
 * @param destination_height The height of the destination image.
 * @return A valid image, otherwise NULL.
 *
 * Works the same as drawable_create_cropped_image() but will scale the
 * new image to the new destination @p destination_width and
 * @p destination_height whilst cropping.
 *
 **/
EAPI                Drawable_Image
drawable_create_cropped_scaled_image(Drawable_Context context, int source_x, int source_y,
                                  int source_width, int source_height,
                                  int destination_width, int destination_height)
{
   DrawableImage         *im, *im_old;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_create_cropped_scaled_image", "image",
                              ctx->image, NULL);
   CAST_IMAGE(im_old, ctx->image);
   if ((!(im_old->data)) && (im_old->loader) && (im_old->loader->load))
      im_old->loader->load(im_old, NULL, 0, 1);
   if (!(im_old->data))
      return NULL;
   im = __drawable_CreateImage(abs(destination_width), abs(destination_height),
                            NULL);
   im->data =
      malloc(abs(destination_width * destination_height) * sizeof(DATA32));
   if (!(im->data))
     {
        __drawable_FreeImage(im);
        return NULL;
     }
   if (IMAGE_HAS_ALPHA(im_old))
     {
        SET_FLAG(im->flags, F_HAS_ALPHA);
        __drawable_BlendImageToImage(im_old, im, ctx->anti_alias, 0, 1, source_x,
                                  source_y, source_width, source_height, 0, 0,
                                  destination_width, destination_height, NULL,
                                  DRAWABLE_OP_COPY,
                                  ctx->cliprect.x, ctx->cliprect.y,
                                  ctx->cliprect.w, ctx->cliprect.h);
     }
   else
     {
        __drawable_BlendImageToImage(im_old, im, ctx->anti_alias, 0, 0, source_x,
                                  source_y, source_width, source_height, 0, 0,
                                  destination_width, destination_height, NULL,
                                  DRAWABLE_OP_COPY,
                                  ctx->cliprect.x, ctx->cliprect.y,
                                  ctx->cliprect.w, ctx->cliprect.h);
     }
   return (Drawable_Image) im;
}

/**
 * @param updates An updates list.
 * @return Duplicate of @p updates.
 *
 * Creates a duplicate of the updates list passed into the function.
 **/
EAPI                Drawable_Updates
drawable_updates_clone(Drawable_Updates updates)
{
   DrawableUpdate        *u;
   u = (DrawableUpdate *) updates;
   return (Drawable_Updates) __drawable_DupUpdates(u);
}

/**
 * @param updates An updates list.
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @return The updates handle.
 *
 * Appends an update rectangle to the updates list passed in (if the
 * updates is NULL it will create a new updates list) and returns a
 * handle to the modified updates list (the handle may be modified so
 * only use the new updates handle returned).
 **/
EAPI                Drawable_Updates
drawable_update_append_rect(Drawable_Updates updates, int x, int y, int w, int h)
{
   DrawableUpdate        *u;

   u = (DrawableUpdate *) updates;
   return (Drawable_Updates) __drawable_AddUpdate(u, x, y, w, h);
}

/**
 * @param updates An updates list.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @return The updates handle.
 *
 * Takes an updates list, and modifies it by merging overlapped
 * rectangles and lots of tiny rectangles into larger rectangles to
 * minimize the number of rectangles in the list for optimized
 * redrawing. The new updates handle is now valid and the old one
 * passed in is not.
 **/
EAPI                Drawable_Updates
drawable_updates_merge(Drawable_Updates updates, int w, int h)
{
   DrawableUpdate        *u;

   u = (DrawableUpdate *) updates;
   return (Drawable_Updates) __drawable_MergeUpdate(u, w, h, 0);
}

/**
 * @param updates An updates list.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @return The updates handle.
 *
 * Works almost exactly as drawable_updates_merge() but is more lenient
 * on the spacing between update rectangles - if they are very close it
 * amalgamates 2 smaller rectangles into 1 larger one.
 **/
EAPI                Drawable_Updates
drawable_updates_merge_for_rendering(Drawable_Updates updates, int w, int h)
{
   DrawableUpdate        *u;

   u = (DrawableUpdate *) updates;
   return (Drawable_Updates) __drawable_MergeUpdate(u, w, h, 3);
}

/**
 * @param updates An updates list.
 *
 * Frees an updates list.
 **/
EAPI void
drawable_updates_free(Drawable_Updates updates)
{
   DrawableUpdate        *u;

   u = (DrawableUpdate *) updates;
   __drawable_FreeUpdates(u);
}

/**
 * @param updates An updates list.
 * @return The next updates.
 *
 * Gets the next update in the updates list relative to the one passed
 * in.
 **/
EAPI                Drawable_Updates
drawable_updates_get_next(Drawable_Updates updates)
{
   DrawableUpdate        *u;

   u = (DrawableUpdate *) updates;
   return (Drawable_Updates) (u->next);
}

/**
 * @param updates An updates list.
 * @param x_return The top left x coordinate of the update.
 * @param y_return The top left y coordinate of the update.
 * @param width_return The width of the update.
 * @param height_return The height of the update.
 *
 * Returns the coordinates of an update.
 **/
EAPI void
drawable_updates_get_coordinates(Drawable_Updates updates, int *x_return,
                              int *y_return, int *width_return,
                              int *height_return)
{
   DrawableUpdate        *u;

   CHECK_PARAM_POINTER("drawable_updates_get_coordinates", "updates", updates);
   u = (DrawableUpdate *) updates;
   if (x_return)
      *x_return = u->x;
   if (y_return)
      *y_return = u->y;
   if (width_return)
      *width_return = u->w;
   if (height_return)
      *height_return = u->h;
}

/**
 * @param updates An updates list.
 * @param x The top left x coordinate of the update.
 * @param y The top left y coordinate of the update.
 * @param width The width of the update.
 * @param height The height of the update.
 *
 * Modifies the coordinates of an update in @p update.
 **/
EAPI void
drawable_updates_set_coordinates(Drawable_Updates updates, int x, int y, int width,
                              int height)
{
   DrawableUpdate        *u;

   CHECK_PARAM_POINTER("drawable_updates_set_coordinates", "updates", updates);
   u = (DrawableUpdate *) updates;
   u->x = x;
   u->y = y;
   u->w = width;
   u->h = height;
}


/**
 * @return The initialized updates list.
 *
 * Initializes an updates list before you add any updates to it or
 * merge it for rendering etc.
 **/
EAPI                Drawable_Updates
drawable_updates_init(void)
{
   return (Drawable_Updates) NULL;
}

/**
 * @param updates An updates list.
 * @param appended_updates The updates list to append.
 * @return The new updates list.
 *
 * Appends @p appended_updates to the updates list @p updates and
 * returns the new list.
 **/
EAPI                Drawable_Updates
drawable_updates_append_updates(Drawable_Updates updates,
                             Drawable_Updates appended_updates)
{
   DrawableUpdate        *u, *uu;

   u = (DrawableUpdate *) updates;
   uu = (DrawableUpdate *) appended_updates;
   if (!uu)
      return (Drawable_Updates) u;
   if (!u)
      return (Drawable_Updates) uu;
   while (u)
     {
        if (!(u->next))
          {
             u->next = uu;
             return updates;
          }
        u = u->next;
     }
   return (Drawable_Updates) u;
}

/**
 * Flips/mirrors the current image horizontally.
 **/
EAPI void
drawable_image_flip_horizontal(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_flip_horizontal", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_FlipImageHoriz(im);
}

/**
 * Flips/mirrors the current image vertically.
 **/
EAPI void
drawable_image_flip_vertical(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_flip_vertical", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_FlipImageVert(im);
}

/**
 * Flips/mirrors the current image diagonally (good for quick and dirty
 * 90 degree rotations if used before to after a horizontal or vertical
 * flip).
 **/
EAPI void
drawable_image_flip_diagonal(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_flip_diagonal", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_FlipImageDiagonal(im, 0);
}

/**
 * @param orientation The orientation.
 *
 * Performs 90 degree rotations on the current image. Passing in
 * @p orientation does not rotate, 1 rotates clockwise by 90 degree, 2,
 * rotates clockwise by 180 degrees, 3 rotates clockwise by 270
 * degrees.
 **/
EAPI void
drawable_image_orientate(Drawable_Context context, int orientation)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_orientate", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   switch (orientation)
     {
     default:
     case 0:
        break;
     case 1:
        __drawable_FlipImageDiagonal(im, 1);
        break;
     case 2:
        __drawable_FlipImageBoth(im);
        break;
     case 3:
        __drawable_FlipImageDiagonal(im, 2);
        break;
     case 4:
        __drawable_FlipImageHoriz(im);
        break;
     case 5:
        __drawable_FlipImageDiagonal(im, 3);
        break;
     case 6:
        __drawable_FlipImageVert(im);
        break;
     case 7:
        __drawable_FlipImageDiagonal(im, 0);
        break;
     }
}

/**
 * @param radius The radius.
 *
 * Blurs the current image. A @p radius value of 0 has no effect, 1 and above
 * determine the blur matrix radius that determine how much to blur the
 * image.
 **/
EAPI void
drawable_image_blur(Drawable_Context context, int radius)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_blur", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_BlurImage(im, radius);
}

/**
 * @param radius The radius.
 *
 * Sharpens the current image. The @p radius value affects how much to sharpen
 * by.
 **/
EAPI void
drawable_image_sharpen(Drawable_Context context, int radius)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CAST_IMAGE(im, ctx->image);
   CHECK_PARAM_POINTER("drawable_image_sharpen", "image", ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_SharpenImage(im, radius);
}

/**
 * Modifies an image so it will tile seamlessly horizontally if used
 * as a tile (i.e. drawn multiple times horizontally).
 **/
EAPI void
drawable_image_tile_horizontal(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_tile_horizontal", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_TileImageHoriz(im);
}

/**
 * Modifies an image so it will tile seamlessly vertically if used as
 * a tile (i.e. drawn multiple times vertically).
 **/
EAPI void
drawable_image_tile_vertical(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_tile_vertical", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_TileImageVert(im);
}

/**
 * Modifies an image so it will tile seamlessly horizontally and
 * vertically if used as a tile (i.e. drawn multiple times horizontally
 * and vertically).
 **/
EAPI void
drawable_image_tile(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_tile", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_TileImageHoriz(im);
   __drawable_TileImageVert(im);
}

/**
 * @param font_name The font name with the size.
 * @return NULL if no font found.
 *
 * Loads a truetype font from the first directory in the font path that
 * contains that font. The font name @p font_name format is "font_name/size". For
 * example. If there is a font file called blum.ttf somewhere in the
 * font path you might use "blum/20" to load a 20 pixel sized font of
 * blum. If the font cannot be found NULL is returned.
 *
 **/
EAPI    void
drawable_load_font(Drawable_Context context, const char *font_name, int face, int size)
{
    DrawableFont *font;
    font = drawable_font_load(font_name, face, size);
    drawable_context_set_font(context, font);
//   return drawable_font_load_joined(font_name);
}

/**
 * Removes the current font from any fallback chain it's in and frees it.
 **/
EAPI void
drawable_free_font(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_free_font", "font", ctx->font);
//   drawable_remove_font_from_fallback_chain(ctx->font);
   drawable_font_free(ctx->font);
   ctx->font = NULL;
}

/**
 * @param font A previously loaded font.
 * @param fallback_font A previously loaded font to be chained to the given font.
 * @return 0 on success.
 *
 * This arranges for the given fallback font to be used if a glyph does not
 * exist in the given font when text is being rendered.
 * Fonts can be arranged in an aribitrarily long chain and attempts will be
 * made in order on the chain.
 * Cycles in the chain are not possible since the given fallback font is
 * removed from any chain it's already in.
 * A fallback font may be a member of only one chain. Adding it as the
 * fallback font to another font will remove it from it's first fallback chain.
 *
 * @deprecated This function may be removed.
 **/
EAPI int
drawable_insert_font_into_fallback_chain(Drawable_Font font, Drawable_Font fallback_font)
{
   CHECK_PARAM_POINTER_RETURN("drawable_insert_font_into_fallback_chain",
                              "font", font, 1);
   CHECK_PARAM_POINTER_RETURN("drawable_insert_font_into_fallback_chain",
                              "fallback_font", fallback_font, 1);
   return drawable_font_insert_into_fallback_chain_imp(font, fallback_font);
}

#if 0
/**
 * @param fallback_font A font previously added to a fallback chain.
 * @return 0 on success.
 *
 * This removes the given font from any fallback chain it may be in.
 * Removing this font joins its previous and next font together in the fallback
 * chain.
 *
 * @deprecated This function may be removed.
 **/
EAPI void
drawable_remove_font_from_fallback_chain(Drawable_Font fallback_font)
{
   CHECK_PARAM_POINTER("drawable_remove_font_from_fallback_chain",
                       "fallback_font", fallback_font);
   drawable_font_remove_from_fallback_chain_imp(fallback_font);
}

/**
 * @deprecated This function may be removed.
 **/
EAPI                Drawable_Font
drawable_get_prev_font_in_fallback_chain(Drawable_Font fn)
{
   CHECK_PARAM_POINTER_RETURN("drawable_get_prev_font_in_fallback_chain",
                              "fn", fn, 0);
   return ((DrawableFont *) fn)->fallback_prev;
}

/**
 * @deprecated This function may be removed.
 **/
EAPI                Drawable_Font
drawable_get_next_font_in_fallback_chain(Drawable_Font fn)
{
   CHECK_PARAM_POINTER_RETURN("drawable_get_next_font_in_fallback_chain",
                              "fn", fn, 0);
   return ((DrawableFont *) fn)->fallback_next;
}
#endif

/**
 * @param x The x coordinate of the top left  corner.
 * @param y The y coordinate of the top left  corner.
 * @param text A null-byte terminated string.
 *
 * Draws the null-byte terminated string @p text using the current font on
 * the current image at the (@p x, @p y) location (@p x, @p y denoting the top left
 * corner of the font string)
 **/
EAPI void
drawable_text_draw(Drawable_Context context, int x, int y, const char *text)
{
   drawable_text_draw_with_return_metrics(context, x, y, text, NULL, NULL, NULL, NULL);
}

/**
 * @param x The x coordinate of the top left  corner.
 * @param y The y coordinate of the top left  corner.
 * @param text A null-byte terminated string.
 * @param width_return The width of the string.
 * @param height_return The height of the string.
 * @param horizontal_advance_return Horizontal offset.
 * @param vertical_advance_return Vertical offset.
 *
 * Works just like drawable_text_draw() but also returns the width and
 * height of the string drawn, and @p horizontal_advance_return returns
 * the number of pixels you should advance horizontally to draw another
 * string (useful if you are drawing a line of text word by word) and
 * @p vertical_advance_return does the same for the vertical direction
 * (i.e. drawing text line by line).
 **/
EAPI void
drawable_text_draw_with_return_metrics(Drawable_Context context, int x, int y, const char *text,
                                    int *width_return, int *height_return,
                                    int *horizontal_advance_return,
                                    int *vertical_advance_return)
{
   DrawableImage         *im;
   DrawableFont          *fn;
   int                 dir;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_text_draw_with_return_metrics", "font",
                       ctx->font);
   CHECK_PARAM_POINTER("drawable_text_draw_with_return_metrics", "image",
                       ctx->image);
   CHECK_PARAM_POINTER("drawable_text_draw_with_return_metrics", "text", text);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   fn = (DrawableFont *) ctx->font;
   __drawable_DirtyImage(im);

   dir = ctx->direction;
   if (ctx->direction == DRAWABLE_TEXT_TO_ANGLE && ctx->angle == 0.0)
      dir = DRAWABLE_TEXT_TO_RIGHT;

    printf("here..\n");
   drawable_render_str(im, fn, x, y, text, (DATA8) ctx->color.red,
                    (DATA8) ctx->color.green, (DATA8) ctx->color.blue,
                    (DATA8) ctx->color.alpha, (char)dir,
                    ctx->angle, width_return, height_return, 0,
                    horizontal_advance_return, vertical_advance_return,
                    ctx->operation,
                    ctx->cliprect.x, ctx->cliprect.y,
                    ctx->cliprect.w, ctx->cliprect.h);
}

/**
 * @param text A string.
 * @param width_return The width of the text.
 * @param height_return The height of the text.
 *
 * Gets the width and height in pixels the @p text string would use up
 * if drawn with the current font.
 **/
EAPI void
drawable_get_text_size(Drawable_Context context, const char *text, int *width_return, int *height_return)
{
   DrawableFont          *fn;
   int                 w, h;
   int                 dir;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_get_text_size", "font", ctx->font);
   CHECK_PARAM_POINTER("drawable_get_text_size", "text", text);
   fn = (DrawableFont *) ctx->font;

   dir = ctx->direction;
   if (ctx->direction == DRAWABLE_TEXT_TO_ANGLE && ctx->angle == 0.0)
      dir = DRAWABLE_TEXT_TO_RIGHT;

   drawable_font_query_size(fn, text, &w, &h);

   switch (dir)
     {
     case DRAWABLE_TEXT_TO_RIGHT:
     case DRAWABLE_TEXT_TO_LEFT:
        if (width_return)
           *width_return = w;
        if (height_return)
           *height_return = h;
        break;
     case DRAWABLE_TEXT_TO_DOWN:
     case DRAWABLE_TEXT_TO_UP:
        if (width_return)
           *width_return = h;
        if (height_return)
           *height_return = w;
        break;
     case DRAWABLE_TEXT_TO_ANGLE:
        if (width_return || height_return)
          {
             double              sa, ca;

             sa = sin(ctx->angle);
             ca = cos(ctx->angle);

             if (width_return)
               {
                  double              x1, x2, xt;

                  x1 = x2 = 0.0;
                  xt = ca * w;
                  if (xt < x1)
                     x1 = xt;
                  if (xt > x2)
                     x2 = xt;
                  xt = -(sa * h);
                  if (xt < x1)
                     x1 = xt;
                  if (xt > x2)
                     x2 = xt;
                  xt = ca * w - sa * h;
                  if (xt < x1)
                     x1 = xt;
                  if (xt > x2)
                     x2 = xt;
                  *width_return = (int)(x2 - x1);
               }
             if (height_return)
               {
                  double              y1, y2, yt;

                  y1 = y2 = 0.0;
                  yt = sa * w;
                  if (yt < y1)
                     y1 = yt;
                  if (yt > y2)
                     y2 = yt;
                  yt = ca * h;
                  if (yt < y1)
                     y1 = yt;
                  if (yt > y2)
                     y2 = yt;
                  yt = sa * w + ca * h;
                  if (yt < y1)
                     y1 = yt;
                  if (yt > y2)
                     y2 = yt;
                  *height_return = (int)(y2 - y1);
               }
          }
        break;
     default:
        break;
     }
}

/**
 * @param text A string.
 * @param horizontal_advance_return Horizontal offset.
 * @param vertical_advance_return Vertical offset.
 *
 * Gets the advance horizontally and vertically in pixels the next
 * text string would need to be placed at for the current font. The
 * advances are not adjusted for rotation so you will have to translate
 * the advances (which are calculated as if the text was drawn
 * horizontally from left to right) depending on the text orientation.
 **/
EAPI void
drawable_get_text_advance(Drawable_Context context, const char *text, int *horizontal_advance_return,
                       int *vertical_advance_return)
{
   DrawableFont          *fn;
   int                 w, h;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_get_text_advance", "font", ctx->font);
   CHECK_PARAM_POINTER("drawable_get_text_advance", "text", text);
   fn = (DrawableFont *) ctx->font;
   drawable_font_query_advance(fn, text, &w, &h);
   if (horizontal_advance_return)
      *horizontal_advance_return = w;
   if (vertical_advance_return)
      *vertical_advance_return = h;
}

/**
 * @param text A string.
 * @return The inset value of @text.
 *
 * Returns the inset of the first character of @p text
 * in using the current font and returns that value in pixels.
 *
 **/
EAPI int
drawable_get_text_inset(Drawable_Context context, const char *text)
{
   DrawableFont          *fn;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_get_text_advance", "font", ctx->font, 0);
   CHECK_PARAM_POINTER_RETURN("drawable_get_text_advance", "text", text, 0);
   fn = (DrawableFont *) ctx->font;
   return drawable_font_query_inset(fn, text);
}

#if 0
/**
 * @param path A directory path.
 *
 * Adds the directory @p path to the end of the current list of
 * directories to scan for fonts.
 **/
EAPI void
drawable_add_path_to_font_path(const char *path)
{
   CHECK_PARAM_POINTER("drawable_add_path_to_font_path", "path", path);
   if (!drawable_font_path_exists(path))
      drawable_font_add_font_path(path);
}

/**
 * @param path A directory path.
 *
 * Removes all directories in the font path that match @p path.
 **/
EAPI void
drawable_remove_path_from_font_path(const char *path)
{
   CHECK_PARAM_POINTER("drawable_remove_path_from_font_path", "path", path);
   drawable_font_del_font_path(path);
}

/**
 * @param number_return Number of paths in the list.
 * @return A list of strings.
 *
 * Returns a list of strings that are the directories in the font
 * path. Do not free this list or change it in any way. If you add or
 * delete members of the font path this list will be invalid. If you
 * intend to use this list later duplicate it for your own use. The
 * number of elements in the array of strings is put into
 * @p number_return.
 *
 **/
EAPI char         **
drawable_list_font_path(int *number_return)
{
   CHECK_PARAM_POINTER_RETURN("drawable_list_font_path", "number_return",
                              number_return, NULL);
   return drawable_font_list_font_path(number_return);
}
#endif

/**
 * @param text A string.
 * @param x The x offset.
 * @param y The y offset.
 * @param char_x_return The x coordinate of the character.
 * @param char_y_return The x coordinate of the character.
 * @param char_width_return The width of the character.
 * @param char_height_return The height of the character.
 * @return -1 if no character found.
 *
 * Returns the character number in the string @p text using the current
 * font at the (@p x, @p y) pixel location which is an offset relative to the
 * top left of that string. -1 is returned if there is no character
 * there. If there is a character, @p char_x_return, @p char_y_return,
 * @p char_width_return and @p char_height_return (respectively the
 * character x, y, width and height)  are also filled in.
 *
 **/
EAPI int
drawable_text_get_index_and_location(Drawable_Context context, const char *text, int x, int y,
                                  int *char_x_return, int *char_y_return,
                                  int *char_width_return,
                                  int *char_height_return)
{
   DrawableFont          *fn;
   int                 w, h, cx, cy, cw, ch, cp, xx, yy;
   int                 dir;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_text_get_index_and_location", "font",
                              ctx->font, -1);
   CHECK_PARAM_POINTER_RETURN("drawable_text_get_index_and_location", "text",
                              text, -1);
   fn = (DrawableFont *) ctx->font;

   dir = ctx->direction;
   if (ctx->direction == DRAWABLE_TEXT_TO_ANGLE && ctx->angle == 0.0)
      dir = DRAWABLE_TEXT_TO_RIGHT;

   drawable_get_text_size(context, text, &w, &h);

   switch (dir)
     {
     case DRAWABLE_TEXT_TO_RIGHT:
        xx = x;
        yy = y;
        break;
     case DRAWABLE_TEXT_TO_LEFT:
        xx = w - x;
        yy = h - y;
        break;
     case DRAWABLE_TEXT_TO_DOWN:
        xx = y;
        yy = w - x;
        break;
     case DRAWABLE_TEXT_TO_UP:
        xx = h - y;
        yy = x;
        break;
     default:
        return -1;
     }

   cp = drawable_font_query_text_at_pos(fn, text, xx, yy, &cx, &cy, &cw, &ch);

   switch (dir)
     {
     case DRAWABLE_TEXT_TO_RIGHT:
        if (char_x_return)
           *char_x_return = cx;
        if (char_y_return)
           *char_y_return = cy;
        if (char_width_return)
           *char_width_return = cw;
        if (char_height_return)
           *char_height_return = ch;
        return cp;
        break;
     case DRAWABLE_TEXT_TO_LEFT:
        cx = 1 + w - cx - cw;
        if (char_x_return)
           *char_x_return = cx;
        if (char_y_return)
           *char_y_return = cy;
        if (char_width_return)
           *char_width_return = cw;
        if (char_height_return)
           *char_height_return = ch;
        return cp;
        break;
     case DRAWABLE_TEXT_TO_DOWN:
        if (char_x_return)
           *char_x_return = cy;
        if (char_y_return)
           *char_y_return = cx;
        if (char_width_return)
           *char_width_return = ch;
        if (char_height_return)
           *char_height_return = cw;
        return cp;
        break;
     case DRAWABLE_TEXT_TO_UP:
        cy = 1 + h - cy - ch;
        if (char_x_return)
           *char_x_return = cy;
        if (char_y_return)
           *char_y_return = cx;
        if (char_width_return)
           *char_width_return = ch;
        if (char_height_return)
           *char_height_return = cw;
        return cp;
        break;
     default:
        return -1;
        break;
     }
   return -1;
}

/**
 * @param text A string.
 * @param index The index of @text.
 * @param char_x_return The x coordinate of the character.
 * @param char_y_return The y coordinate of the character.
 * @param char_width_return The width of the character.
 * @param char_height_return The height of the character.
 *
 * Gets the geometry of the character at index @p index in the @p text
 * string using the current font.
 **/
EAPI void
drawable_text_get_location_at_index(Drawable_Context context, const char *text, int index,
                                 int *char_x_return, int *char_y_return,
                                 int *char_width_return,
                                 int *char_height_return)
{
   DrawableFont          *fn;
   int                 cx, cy, cw, ch, w, h;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_text_get_index_and_location", "font", ctx->font);
   CHECK_PARAM_POINTER("drawable_text_get_index_and_location", "text", text);
   fn = (DrawableFont *) ctx->font;

   drawable_font_query_char_coords(fn, text, index, &cx, &cy, &cw, &ch);

   drawable_get_text_size(context, text, &w, &h);

   switch (ctx->direction)
     {
     case DRAWABLE_TEXT_TO_RIGHT:
        if (char_x_return)
           *char_x_return = cx;
        if (char_y_return)
           *char_y_return = cy;
        if (char_width_return)
           *char_width_return = cw;
        if (char_height_return)
           *char_height_return = ch;
        return;
        break;
     case DRAWABLE_TEXT_TO_LEFT:
        cx = 1 + w - cx - cw;
        if (char_x_return)
           *char_x_return = cx;
        if (char_y_return)
           *char_y_return = cy;
        if (char_width_return)
           *char_width_return = cw;
        if (char_height_return)
           *char_height_return = ch;
        return;
        break;
     case DRAWABLE_TEXT_TO_DOWN:
        if (char_x_return)
           *char_x_return = cy;
        if (char_y_return)
           *char_y_return = cx;
        if (char_width_return)
           *char_width_return = ch;
        if (char_height_return)
           *char_height_return = cw;
        return;
        break;
     case DRAWABLE_TEXT_TO_UP:
        cy = 1 + h - cy - ch;
        if (char_x_return)
           *char_x_return = cy;
        if (char_y_return)
           *char_y_return = cx;
        if (char_width_return)
           *char_width_return = ch;
        if (char_height_return)
           *char_height_return = cw;
        return;
        break;
     default:
        return;
        break;
     }
}

#if 0
/**
 * @param number_return Number of fonts in the list.
 * @return A list of fonts.
 *
 * Returns a list of fonts imlib2 can find in its font path.
 *
 **/
EAPI char         **
drawable_list_fonts(int *number_return)
{
   CHECK_PARAM_POINTER_RETURN("drawable_list_fonts", "number_return",
                              number_return, NULL);
   return drawable_font_list_fonts(number_return);
}

/**
 * @param font_list The font list.
 * @param number Number of fonts in the list.
 *
 * Frees the font list returned by drawable_list_fonts().
 *
 **/
EAPI void
drawable_free_font_list(char **font_list, int number)
{
   __drawable_FileFreeDirList(font_list, number);
}
#endif

/**
 * @return The font cache size.
 *
 * Returns the font cache size in bytes.
 *
 **/
EAPI int
drawable_get_font_cache_size(void)
{
   return drawable_font_cache_get();
}

/**
 * @param bytes The font cache size.
 *
 * Sets the font cache in bytes. Whenever you set the font cache size
 * Drawable2 will flush fonts from the cache until the memory used by
 * fonts is less than or equal to the font cache size. Setting the size
 * to 0 effectively frees all speculatively cached fonts.
 **/
EAPI void
drawable_set_font_cache_size(int bytes)
{
   drawable_font_cache_set(bytes);
}

/**
 * Causes a flush of all speculatively cached fonts from the font
 * cache.
 **/
EAPI void
drawable_flush_font_cache(void)
{
   drawable_font_flush();
}

/**
 * @return The font's ascent.
 *
 * Returns the current font's ascent value in pixels.
 *
 **/
EAPI int
drawable_get_font_ascent(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_get_font_ascent", "font", ctx->font, 0);
   return drawable_font_ascent_get(ctx->font);
}

/**
 * @return The font's descent.
 *
 * Returns the current font's descent value in pixels.
 *
 **/
EAPI int
drawable_get_font_descent(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_get_font_ascent", "font", ctx->font, 0);
   return drawable_font_descent_get(ctx->font);
}

/**
 * @return The font's maximum ascent.
 *
 * Returns the current font's maximum ascent extent.
 *
 **/
EAPI int
drawable_get_maximum_font_ascent(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_get_font_ascent", "font", ctx->font, 0);
   return drawable_font_max_ascent_get(ctx->font);
}

/**
 * @return The font's maximum descent.
 *
 * Returns the current font's maximum descent extent.
 *
 **/
EAPI int
drawable_get_maximum_font_descent(Drawable_Context context)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_get_font_ascent", "font", ctx->font, 0);
   return drawable_font_max_descent_get(ctx->font);
}

#if 0
/**
 * @return Valid handle.
 *
 * Creates a new empty color modifier and returns a
 * valid handle on success. NULL is returned on failure.
 *
 **/
EAPI                Drawable_Color_Modifier
drawable_create_color_modifier(void)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return (Drawable_Color_Modifier) __drawable_CreateCmod();
}

/**
 * Frees the current color modifier.
 **/
EAPI void
drawable_free_color_modifier(void)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_free_color_modifier", "color_modifier",
                       ctx->color_modifier);
   __drawable_FreeCmod((DrawableColorModifier *) ctx->color_modifier);
   ctx->color_modifier = NULL;
}

/**
 * @param gamma_value Value of gamma.
 *
 * Modifies the current color modifier by adjusting the gamma by the
 * value specified @p gamma_value. The color modifier is modified not set, so calling
 * this repeatedly has cumulative effects. A gamma of 1.0 is normal
 * linear, 2.0 brightens and 0.5 darkens etc. Negative values are not
 * allowed.
 **/
EAPI void
drawable_modify_color_modifier_gamma(double gamma_value)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_modify_color_modifier_gamma", "color_modifier",
                       ctx->color_modifier);
   __drawable_CmodModGamma((DrawableColorModifier *) ctx->color_modifier,
                        gamma_value);
}

/**
 * @param brightness_value Value of brightness.
 *
 * Modifies the current color modifier by adjusting the brightness by
 * the value @p brightness_value. The color modifier is modified not set, so
 * calling this repeatedly has cumulative effects. brightness values
 * of 0 do not affect anything. -1.0 will make things completely black
 * and 1.0 will make things all white. Values in-between vary
 * brightness linearly.
 **/
EAPI void
drawable_modify_color_modifier_brightness(double brightness_value)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_modify_color_modifier_brightness",
                       "color_modifier", ctx->color_modifier);
   __drawable_CmodModBrightness((DrawableColorModifier *) ctx->color_modifier,
                             brightness_value);
}

/**
 * @param contrast_value Value of contrast.
 *
 * Modifies the current color modifier by adjusting the contrast by
 * the value @p contrast_value. The color modifier is modified not set, so
 * calling this repeatedly has cumulative effects. Contrast of 1.0 does
 * nothing. 0.0 will merge to gray, 2.0 will double contrast etc.
 **/
EAPI void
drawable_modify_color_modifier_contrast(double contrast_value)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_modify_color_modifier_contrast",
                       "color_modifier", ctx->color_modifier);
   __drawable_CmodModContrast((DrawableColorModifier *) ctx->color_modifier,
                           contrast_value);
}

/**
 * @param red_table An array of #DATA8.
 * @param green_table An array of #DATA8.
 * @param blue_table An array of #DATA8.
 * @param alpha_table An array of #DATA8.
 *
 * Explicitly copies the mapping tables from the table pointers passed
 * into this function into those of the current color modifier. Tables
 * are 256 entry arrays of DATA8 which are a mapping of that channel
 * value to a new channel value. A normal mapping would be linear (v[0]
 * = 0, v[10] = 10, v[50] = 50, v[200] = 200, v[255] = 255).
 **/
EAPI void
drawable_set_color_modifier_tables(DATA8 * red_table, DATA8 * green_table,
                                DATA8 * blue_table, DATA8 * alpha_table)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_set_color_modifier_tables", "color_modifier",
                       ctx->color_modifier);
   __drawable_CmodSetTables((DrawableColorModifier *) ctx->color_modifier,
                         red_table, green_table, blue_table, alpha_table);
}

/**
 * @param red_table: an array of #DATA8.
 * @param green_table: an array of #DATA8.
 * @param blue_table: an array of #DATA8.
 * @param alpha_table: an array of #DATA8.
 *
 * Copies the table values from the current color modifier into the
 * pointers to mapping tables specified. They must have 256 entries and
 * be DATA8 format.
 **/
EAPI void
drawable_get_color_modifier_tables(DATA8 * red_table, DATA8 * green_table,
                                DATA8 * blue_table, DATA8 * alpha_table)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_get_color_modifier_tables", "color_modifier",
                       ctx->color_modifier);
   __drawable_CmodGetTables((DrawableColorModifier *) ctx->color_modifier,
                         red_table, green_table, blue_table, alpha_table);
}

/**
 * Resets the current color modifier to have linear mapping tables.
 **/
EAPI void
drawable_reset_color_modifier(void)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_rset_color_modifier", "color_modifier",
                       ctx->color_modifier);
   __drawable_CmodReset((DrawableColorModifier *) ctx->color_modifier);
}

/**
 * Uses the current color modifier and modifies the current image using
 * the mapping tables in the current color modifier.
 **/
EAPI void
drawable_apply_color_modifier(void)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_apply_color_modifier", "image", ctx->image);
   CHECK_PARAM_POINTER("drawable_apply_color_modifier", "color_modifier",
                       ctx->color_modifier);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_DataCmodApply(im->data, im->w, im->h, 0, &(im->flags),
                         (DrawableColorModifier *) ctx->color_modifier);
}

/**
 * @param x The x coordinate of the left edge of the rectangle.
 * @param y  The y coordinate of the top edge of the rectangle.
 * @param width  The width of the rectangle.
 * @param height  The height of the rectangle.
 *
 * Works the same way as drawable_apply_color_modifier() but only modifies
 * a selected rectangle in the current image.
 **/
EAPI void
drawable_apply_color_modifier_to_rectangle(int x, int y, int width, int height)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_apply_color_modifier_to_rectangle", "image",
                       ctx->image);
   CHECK_PARAM_POINTER("drawable_apply_color_modifier_to_rectangle",
                       "color_modifier", ctx->color_modifier);
   CAST_IMAGE(im, ctx->image);
   if (x < 0)
     {
        width += x;
        x = 0;
     }
   if (width <= 0)
      return;
   if ((x + width) > im->w)
      width = (im->w - x);
   if (width <= 0)
      return;
   if (y < 0)
     {
        height += y;
        y = 0;
     }
   if (height <= 0)
      return;
   if ((y + height) > im->h)
      height = (im->h - y);
   if (height <= 0)
      return;
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_DataCmodApply(im->data + (y * im->w) + x, width, height,
                         im->w - width, &(im->flags),
                         (DrawableColorModifier *) ctx->color_modifier);
}
#endif

EAPI                Drawable_Updates
drawable_image_draw_pixel(Drawable_Context context, int x, int y, char make_updates)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_draw_pixel", "image", ctx->image,
                              NULL);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return NULL;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   return (Drawable_Updates) __drawable_Point_DrawToImage(x, y, color, im,
                                                    ctx->cliprect.x,
                                                    ctx->cliprect.y,
                                                    ctx->cliprect.w,
                                                    ctx->cliprect.h,
                                                    ctx->operation, ctx->blend,
                                                    make_updates);
}

/**
 * @param x1 The x coordinate of the first point.
 * @param y1 The y coordinate of the first point.
 * @param x2 The x coordinate of the second point.
 * @param y2 The y coordinate of the second point.
 * @param make_updates: a char.
 * @return An updates list.
 *
 * Draws a line using the current color on the current image from
 * coordinates (@p x1, @p y1) to (@p x2, @p y2). If @p make_updates is 1 it will also
 * return an update you can use for an updates list, otherwise it
 * returns NULL.
 *
 **/
EAPI                Drawable_Updates
drawable_image_draw_line(Drawable_Context context, int x1, int y1, int x2, int y2, char make_updates)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER_RETURN("drawable_image_draw_line", "image", ctx->image,
                              NULL);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return NULL;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   return (Drawable_Updates) __drawable_Line_DrawToImage(x1, y1, x2, y2, color, im,
                                                   ctx->cliprect.x,
                                                   ctx->cliprect.y,
                                                   ctx->cliprect.w,
                                                   ctx->cliprect.h,
                                                   ctx->operation, ctx->blend,
                                                   ctx->anti_alias,
                                                   make_updates);
}

/**
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 *
 * Draws the outline of a rectangle on the current image at the (@p x,
 * @p y)
 * coordinates with a size of @p width and @p height pixels, using the
 * current color.
 **/
EAPI void
drawable_image_draw_rectangle(Drawable_Context context, int x, int y, int width, int height)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_draw_rectangle", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   __drawable_Rectangle_DrawToImage(x, y, width, height, color,
                                 im, ctx->cliprect.x, ctx->cliprect.y,
                                 ctx->cliprect.w, ctx->cliprect.h,
                                 ctx->operation, ctx->blend);
}

/**
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 *
 * Draws a filled rectangle on the current image at the (@p x, @p y)
 * coordinates with a size of @p width and @p height pixels, using the
 * current color.
 **/
EAPI void
drawable_image_fill_rectangle(Drawable_Context context, int x, int y, int width, int height)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_fill_rectangle", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   __drawable_Rectangle_FillToImage(x, y, width, height, color,
                                 im, ctx->cliprect.x, ctx->cliprect.y,
                                 ctx->cliprect.w, ctx->cliprect.h,
                                 ctx->operation, ctx->blend);
}

/**
 * @param image_source An image.
 * @param x The x coordinate.
 * @param y The y coordinate.
 *
 * Copies the alpha channel of the source image @p image_source to the
 * (@p x, @p y) coordinates
 * of the current image, replacing the alpha channel there.
 **/
EAPI void
drawable_image_copy_alpha_to_image(Drawable_Context context, Drawable_Image image_source, int x, int y)
{
   DrawableImage         *im, *im2;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_copy_alpha_to_image", "image_source",
                       image_source);
   CHECK_PARAM_POINTER("drawable_image_copy_alpha_to_image", "image_destination",
                       ctx->image);
   CAST_IMAGE(im, image_source);
   CAST_IMAGE(im2, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if ((!(im2->data)) && (im2->loader) && (im2->loader->load))
      im2->loader->load(im2, NULL, 0, 1);
   if (!(im->data))
      return;
   if (!(im2->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_copy_alpha_data(im, im2, 0, 0, im->w, im->h, x, y);
}

/**
 * @param image_source An image.
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param destination_x The top left x coordinate of the destination rectangle.
 * @param destination_y The top left x coordinate of the destination rectangle.
 *
 * Copies the source (@p x, @p y, @p width, @p height) rectangle alpha channel from
 * the source image @p image_source and replaces the alpha channel on the destination
 * image at the (@p destination_x, @p destination_y) coordinates.
 **/
EAPI void
drawable_image_copy_alpha_rectangle_to_image(Drawable_Context context, Drawable_Image image_source, int x,
                                          int y, int width, int height,
                                          int destination_x, int destination_y)
{
   DrawableImage         *im, *im2;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_copy_alpha_rectangle_to_image",
                       "image_source", image_source);
   CHECK_PARAM_POINTER("drawable_image_copy_alpha_rectangle_to_image",
                       "image_destination", ctx->image);
   CAST_IMAGE(im, image_source);
   CAST_IMAGE(im2, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if ((!(im2->data)) && (im2->loader) && (im2->loader->load))
      im2->loader->load(im2, NULL, 0, 1);
   if (!(im->data))
      return;
   if (!(im2->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_copy_alpha_data(im, im2, x, y, width, height, destination_x,
                           destination_y);
}

/**
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param delta_x Distance along the x coordinates.
 * @param delta_y Distance along the y coordinates.
 *
 * Scrolls a rectangle of size @p width, @p height at the (@p x, @p y)
 * location within the current image
 * by the @p delta_x, @p delta_y distance (in pixels).
 **/
EAPI void
drawable_image_scroll_rect(Drawable_Context context, int x, int y, int width, int height, int delta_x,
                        int delta_y)
{
   DrawableImage         *im;
   int                 xx, yy, w, h, nx, ny;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_scroll_rect", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   if (delta_x > 0)
     {
        xx = x;
        nx = x + delta_x;
        w = width - delta_x;
     }
   else
     {
        xx = x - delta_x;
        nx = x;
        w = width + delta_x;
     }
   if (delta_y > 0)
     {
        yy = y;
        ny = y + delta_y;
        h = height - delta_y;
     }
   else
     {
        yy = y - delta_y;
        ny = y;
        h = height + delta_y;
     }
   __drawable_DirtyImage(im);
   __drawable_copy_image_data(im, xx, yy, w, h, nx, ny);
}

/**
 * @param x The top left x coordinate of the rectangle.
 * @param y The top left y coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param new_x The top left x coordinate of the new location.
 * @param new_y The top left y coordinate of the new location.
 *
 * Copies a rectangle of size @p width, @p height at the (@p x, @p y) location
 * specified in the current image to a new location (@p new_x, @p new_y) in the same
 * image.
 **/
EAPI void
drawable_image_copy_rect(Drawable_Context context, int x, int y, int width, int height, int new_x, int new_y)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_copy_rect", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_copy_image_data(im, x, y, width, height, new_x, new_y);
}

#if 0
/**
 * @return valid handle.
 *
 * Creates a new empty color range and returns a valid handle to that
 * color range.
 **/
EAPI                Drawable_Color_Range
drawable_create_color_range(void)
{
   DrawableContext *ctx = (DrawableContext *) context;
   return (Drawable_Color_Range) __drawable_CreateRange();
}

/**
 * Frees the current color range.
 **/
EAPI void
drawable_free_color_range(void)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_free_color_range", "color_range",
                       ctx->color_range);
   __drawable_FreeRange((DrawableRange *) ctx->color_range);
   ctx->color_range = NULL;
}

/**
 * @param distance_away Distance from the previous color.
 *
 * Adds the current color to the current color range at a @p distance_away
 * distance from the previous color in the range (if it's the first
 * color in the range this is irrelevant).
 **/
EAPI void
drawable_add_color_to_color_range(int distance_away)
{
   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_add_color_to_color_range", "color_range",
                       ctx->color_range);
   __drawable_AddRangeColor((DrawableRange *) ctx->color_range, ctx->color.red,
                         ctx->color.green, ctx->color.blue, ctx->color.alpha,
                         distance_away);
}

/**
 * @param x The x coordinate of the left edge of the rectangle.
 * @param y The y coordinate of the top edge of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param angle Angle of gradient.
 *
 * Fills a rectangle of width @p width and height @p height at the (@p x, @p y) location
 * specified in the current image with a linear gradient of the
 * current color range at an angle of @p angle degrees with 0 degrees
 * being vertical from top to bottom going clockwise from there.
 **/
EAPI void
drawable_image_fill_color_range_rectangle(int x, int y, int width, int height,
                                       double angle)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_fill_color_range_rectangle", "image",
                       ctx->image);
   CHECK_PARAM_POINTER("drawable_image_fill_color_range_rectangle",
                       "color_range", ctx->color_range);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_DrawGradient(im, x, y, width, height,
                        (DrawableRange *) ctx->color_range, angle,
                        ctx->operation,
                        ctx->cliprect.x, ctx->cliprect.y,
                        ctx->cliprect.w, ctx->cliprect.h);
}

/**
 * @param x The x coordinate of the left edge of the rectangle.
 * @param y The y coordinate of the top edge of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param angle Angle of gradient.
 *
 * Fills a rectangle of width @p width and height @p height at the (@p
 * x, @p y) location
 * specified in the current image with a linear gradient in HSVA color
 * space of the current color range at an angle of @p angle degrees with
 * 0 degrees being vertical from top to bottom going clockwise from
 * there.
 **/
EAPI void
drawable_image_fill_hsva_color_range_rectangle(int x, int y, int width, int height,
                                            double angle)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_fill_color_range_rectangle", "image",
                       ctx->image);
   CHECK_PARAM_POINTER("drawable_image_fill_color_range_rectangle",
                       "color_range", ctx->color_range);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   __drawable_DrawHsvaGradient(im, x, y, width, height,
                            (DrawableRange *) ctx->color_range, angle,
                            ctx->operation,
                            ctx->cliprect.x, ctx->cliprect.y,
                            ctx->cliprect.w, ctx->cliprect.h);
}
#endif

/**
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param color_return The returned color.
 *
 * Fills the @p color_return color structure with the color of the pixel
 * in the current image that is at the (@p x, @p y) location specified.
 **/
EAPI void
drawable_image_query_pixel(Drawable_Context context, int x, int y, Drawable_Color * color_return)
{
   DrawableImage         *im;
   DATA32             *p;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_query_pixel", "image", ctx->image);
   CHECK_PARAM_POINTER("drawable_image_query_pixel", "color_return", color_return);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   if ((x < 0) || (x >= im->w) || (y < 0) || (y >= im->h))
     {
        color_return->red = 0;
        color_return->green = 0;
        color_return->blue = 0;
        color_return->alpha = 0;
        return;
     }
   p = im->data + (im->w * y) + x;
   color_return->red = ((*p) >> 16) & 0xff;
   color_return->green = ((*p) >> 8) & 0xff;
   color_return->blue = (*p) & 0xff;
   color_return->alpha = ((*p) >> 24) & 0xff;
}

/**
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param hue The returned hue channel.
 * @param saturation The returned saturation channel.
 * @param value The returned value channel.
 * @param alpha The returned alpha channel.
 *
 * Gets the HSVA color of the pixel from the current image that is at
 * the (@p x, @p y) location specified.
 **/
EAPI void
drawable_image_query_pixel_hsva(Drawable_Context context, int x, int y, float *hue, float *saturation,
                             float *value, int *alpha)
{
   DrawableImage         *im;
   DATA32             *p;
   int                 r, g, b;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_query_pixel", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   if ((x < 0) || (x >= im->w) || (y < 0) || (y >= im->h))
     {
        *hue = 0;
        *saturation = 0;
        *value = 0;
        *alpha = 0;
        return;
     }
   p = im->data + (im->w * y) + x;
   r = ((*p) >> 16) & 0xff;
   g = ((*p) >> 8) & 0xff;
   b = (*p) & 0xff;
   *alpha = ((*p) >> 24) & 0xff;

   __drawable_rgb_to_hsv(r, g, b, hue, saturation, value);
}

/**
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param hue The returned hue channel.
 * @param lightness The returned lightness channel.
 * @param saturation The returned saturation channel.
 * @param alpha The returned alpha channel.
 *
 * Gets the HLSA color of the pixel from the current image that is at
 * the (@p x, @p y) location specified.
 **/
EAPI void
drawable_image_query_pixel_hlsa(Drawable_Context context, int x, int y, float *hue, float *lightness,
                             float *saturation, int *alpha)
{
   DrawableImage         *im;
   DATA32             *p;
   int                 r, g, b;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_query_pixel", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   if ((x < 0) || (x >= im->w) || (y < 0) || (y >= im->h))
     {
        *hue = 0;
        *lightness = 0;
        *saturation = 0;
        *alpha = 0;
        return;
     }
   p = im->data + (im->w * y) + x;
   r = ((*p) >> 16) & 0xff;
   g = ((*p) >> 8) & 0xff;
   b = (*p) & 0xff;
   *alpha = ((*p) >> 24) & 0xff;

   __drawable_rgb_to_hls(r, g, b, hue, lightness, saturation);
}

/**
 * @param x Tthe x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param cyan The returned cyan channel.
 * @param magenta The returned magenta channel.
 * @param yellow The returned yellow channel.
 * @param alpha The returned alpha channel.
 *
 * Gets the CMYA color of the pixel from the current image that is at
 * the (@p x, @p y) location specified.
 *
 **/
EAPI void
drawable_image_query_pixel_cmya(Drawable_Context context, int x, int y, int *cyan, int *magenta, int *yellow,
                             int *alpha)
{
   DrawableImage         *im;
   DATA32             *p;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_query_pixel", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   if ((x < 0) || (x >= im->w) || (y < 0) || (y >= im->h))
     {
        *cyan = 0;
        *magenta = 0;
        *yellow = 0;
        *alpha = 0;
        return;
     }
   p = im->data + (im->w * y) + x;
   *cyan = 255 - (((*p) >> 16) & 0xff);
   *magenta = 255 - (((*p) >> 8) & 0xff);
   *yellow = 255 - ((*p) & 0xff);
   *alpha = ((*p) >> 24) & 0xff;
}


/**
 * Returns a new polygon object with no points set.
 **/
EAPI                DrawablePolygon
drawable_polygon_new(void)
{
   return (DrawablePolygon) __drawable_polygon_new();
}

/**
 * @param poly A polygon
 * @param x The X coordinate.
 * @param y The Y coordinate.
 *
 * Adds the point (@p x, @p y) to the polygon @p poly. The point will be added
 * to the end of the polygon's internal point list. The points are
 * drawn in order, from the first to the last.
 **/
EAPI void
drawable_polygon_add_point(DrawablePolygon poly, int x, int y)
{
   CHECK_PARAM_POINTER("drawable_polygon_add_point", "polygon", poly);
   __drawable_polygon_add_point((DrawablePoly) poly, x, y);
}

/**
 * @param poly A polygon.
 *
 * Frees a polygon object.
 **/
EAPI void
drawable_polygon_free(DrawablePolygon poly)
{
   CHECK_PARAM_POINTER("drawable_polygon_free", "polygon", poly);
   __drawable_polygon_free((DrawablePoly) poly);
}

/**
 * @param poly A polygon.
 * @param closed Closed polygon flag.
 *
 * Draws the polygon @p poly onto the current context image. Points which have
 * been added to the polygon are drawn in sequence, first to last. The
 * final point will be joined with the first point if @p closed is
 * non-zero.
 **/
EAPI void
drawable_image_draw_polygon(Drawable_Context context, DrawablePolygon poly, unsigned char closed)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_draw_polygon", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   __drawable_Polygon_DrawToImage((DrawablePoly) poly, closed, color,
                               im, ctx->cliprect.x, ctx->cliprect.y,
                               ctx->cliprect.w, ctx->cliprect.h,
                               ctx->operation, ctx->blend, ctx->anti_alias);
}

/**
 * @param poly A polygon.
 *
 * Fills the area defined by the polygon @p polyon the current context image
 * with the current context color.
 **/
EAPI void
drawable_image_fill_polygon(Drawable_Context context, DrawablePolygon poly)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_fill_polygon", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   __drawable_Polygon_FillToImage((DrawablePoly) poly, color,
                               im, ctx->cliprect.x, ctx->cliprect.y,
                               ctx->cliprect.w, ctx->cliprect.h,
                               ctx->operation, ctx->blend, ctx->anti_alias);
}

/**
 * @param poly A polygon.
 * @param px1 X coordinate of the upper left corner.
 * @param py1 Y coordinate of the upper left corner.
 * @param px2 X coordinate of the lower right corner.
 * @param py2 Y coordinate of the lower right corner.
 *
 * Calculates the bounding area of the polygon @p poly. (@p px1, @p py1) defines the
 * upper left corner of the bounding box and (@p px2, @p py2) defines it's
 * lower right corner.
 **/
EAPI void
drawable_polygon_get_bounds(DrawablePolygon poly, int *px1, int *py1, int *px2,
                         int *py2)
{
   CHECK_PARAM_POINTER("drawable_polygon_get_bounds", "polygon", poly);
   __drawable_polygon_get_bounds((DrawablePoly) poly, px1, py1, px2, py2);
}

/**
 * @param xc X coordinate of the center of the ellipse.
 * @param yc Y coordinate of the center of the ellipse.
 * @param a The horizontal amplitude of the ellipse.
 * @param b The vertical amplitude of the ellipse.
 *
 * Draws an ellipse on the current context image. The ellipse is
 * defined as (@p x-@p xc)^2/@p a^2 + (@p y-@p yc)^2/@p b^2 = 1. This means that the
 * point (@p xc, @p yc) marks the center of the ellipse, @p a defines the
 * horizontal amplitude of the ellipse, and @p b defines the vertical
 * amplitude.
 **/
EAPI void
drawable_image_draw_ellipse(Drawable_Context context, int xc, int yc, int a, int b)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_draw_ellipse", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   __drawable_Ellipse_DrawToImage(xc, yc, a, b, color,
                               im, ctx->cliprect.x, ctx->cliprect.y,
                               ctx->cliprect.w, ctx->cliprect.h,
                               ctx->operation, ctx->blend, ctx->anti_alias);
}

/**
 * @param xc X coordinate of the center of the ellipse.
 * @param yc Y coordinate of the center of the ellipse.
 * @param a The horizontal amplitude of the ellipse.
 * @param b The vertical amplitude of the ellipse.
 *
 * Fills an ellipse on the current context image using the current
 * context color. The ellipse is
 * defined as (@p x-@p xc)^2/@p a^2 + (@p y-@p yc)^2/@p b^2 = 1. This means that the
 * point (@p xc, @p yc) marks the center of the ellipse, @p a defines the
 * horizontal amplitude of the ellipse, and @p b defines the vertical
 * amplitude.
 **/
EAPI void
drawable_image_fill_ellipse(Drawable_Context context, int xc, int yc, int a, int b)
{
   DrawableImage         *im;
   DATA32              color;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_fill_ellipse", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   A_VAL(&color) = (DATA8) ctx->color.alpha;
   R_VAL(&color) = (DATA8) ctx->color.red;
   G_VAL(&color) = (DATA8) ctx->color.green;
   B_VAL(&color) = (DATA8) ctx->color.blue;
   __drawable_Ellipse_FillToImage(xc, yc, a, b, color,
                               im, ctx->cliprect.x, ctx->cliprect.y,
                               ctx->cliprect.w, ctx->cliprect.h,
                               ctx->operation, ctx->blend, ctx->anti_alias);
}

/**
 * @param poly A polygon
 * @param x The X coordinate.
 * @param y The Y coordinate.
 *
 * Returns non-zero if the point (@p x, @p y) is within the area defined by
 * the polygon @p poly.
 **/
EAPI unsigned char
drawable_polygon_contains_point(DrawablePolygon poly, int x, int y)
{
   CHECK_PARAM_POINTER_RETURN("drawable_polygon_contains_point", "polygon", poly,
                              0);
   return __drawable_polygon_contains_point((DrawablePoly) poly, x, y);
}

EAPI void
drawable_image_clear(Drawable_Context context)
{
   DrawableImage         *im;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_clear", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   memset(im->data, 0, im->w * im->h * sizeof(DATA32));
}

EAPI void
drawable_image_clear_color(Drawable_Context context, int r, int g, int b, int a)
{
   DrawableImage         *im;
   int                 i, max;
   DATA32              col;

   DrawableContext *ctx = (DrawableContext *) context;
   CHECK_PARAM_POINTER("drawable_image_clear_color", "image", ctx->image);
   CAST_IMAGE(im, ctx->image);
   if ((!(im->data)) && (im->loader) && (im->loader->load))
      im->loader->load(im, NULL, 0, 1);
   if (!(im->data))
      return;
   __drawable_DirtyImage(im);
   max = im->w * im->h;
   WRITE_RGBA(&col, r, g, b, a);
   for (i = 0; i < max; i++)
      im->data[i] = col;
}

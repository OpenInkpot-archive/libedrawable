#include "common.h"
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#ifdef BUILD_X11
# include <X11/Xlib.h>
#endif
#include "image.h"
#include "eimlib.h"



/* create an image data struct and fill it in */
DrawableImage         *
__drawable_ProduceImage(void)
{
   DrawableImage         *im;

   im = malloc(sizeof(DrawableImage));
   memset(im, 0, sizeof(DrawableImage));
   im->data = NULL;
   im->file = NULL;
   im->real_file = NULL;
   im->key = NULL;
   im->flags = F_FORMAT_IRRELEVANT | F_BORDER_IRRELEVANT | F_ALPHA_IRRELEVANT;
   im->loader = NULL;
   im->next = NULL;
   im->tags = NULL;
   return im;
}

/* free an image struct */
void
__drawable_ConsumeImage(DrawableImage * im)
{

   if (im->file)
      free(im->file);
   if (im->real_file)
      free(im->real_file);
   if (im->key)
      free(im->key);
   if ((IMAGE_FREE_DATA(im)) && (im->data))
      free(im->data);
   if (im->format)
      free(im->format);
   free(im);
}


/* set or unset the alpha flag on the umage (alpha = 1 / 0 ) */
void
__drawable_SetImageAlphaFlag(DrawableImage * im, char alpha)
{
   if (alpha)
      SET_FLAG(im->flags, F_HAS_ALPHA);
   else
      UNSET_FLAG(im->flags, F_HAS_ALPHA);
}

/* free and image - if its uncachable and refcoutn is 0 - free it in reality */
void
__drawable_FreeImage(DrawableImage * im)
{
   /* if the refcount is positive */
   if (im->references >= 0)
     {
        /* reduce a reference from the count */
        im->references--;
        /* if its uncachchable ... */
        if (IMAGE_IS_UNCACHEABLE(im))
          {
             /* and we're down to no references for the image then free it */
             if (im->references == 0)
                __drawable_ConsumeImage(im);
          }
        /* otherwise clean up our cache if the image becoem 0 ref count */
     }
}


/* dirty and image by settings its invalid flag */
void
__drawable_DirtyImage(DrawableImage * im)
{
   SET_FLAG(im->flags, F_INVALID);
}

/* create a new image struct from data passed that is wize w x h then return */
/* a pointer to that image sturct */
DrawableImage         *
__drawable_CreateImage(int w, int h, DATA32 * data)
{
   DrawableImage         *im;

   im = __drawable_ProduceImage();
   im->w = w;
   im->h = h;
   im->data = data;
   im->references = 1;
   SET_FLAG(im->flags, F_UNCACHEABLE);
   return im;
}



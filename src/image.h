#ifndef __IMAGE
# define __IMAGE 1

# include "common.h"
# ifdef BUILD_X11
#  include <X11/Xlib.h>
# else
#  define X_DISPLAY_MISSING
# endif

# include <dlfcn.h>
# include "eimlib.h"

# ifndef RTLD_LOCAL
#  define RTLD_LOCAL 0
#  warning "your crap box doesnt define RTLD_LOCAL !?"
# endif

typedef struct _drawableimage              DrawableImage;
# ifdef BUILD_X11
typedef struct _drawableimagepixmap        DrawableImagePixmap;
# endif
typedef struct _drawableborder             DrawableBorder;
typedef struct _drawableloader             DrawableLoader;
typedef struct _drawableimagetag           DrawableImageTag;

typedef int (*DrawableProgressFunction)(DrawableImage *im, char percent,
				      int update_x, int update_y,
				      int update_w, int update_h);
typedef void (*DrawableDataDestructorFunction)(DrawableImage *im, void *data);

enum _iflags
{
   F_NONE              = 0,
   F_HAS_ALPHA         = (1 << 0),
   F_UNLOADED          = (1 << 1),
   F_UNCACHEABLE       = (1 << 2),
   F_ALWAYS_CHECK_DISK = (1 << 3),
   F_INVALID           = (1 << 4),
   F_DONT_FREE_DATA    = (1 << 5),
   F_FORMAT_IRRELEVANT = (1 << 6),
   F_BORDER_IRRELEVANT = (1 << 7),
   F_ALPHA_IRRELEVANT  = (1 << 8)
};

typedef enum   _iflags                  DrawableImageFlags;

struct _drawableborder
{
   int left, right, top, bottom;
};

struct _drawableimagetag
{
   char           *key;
   int             val;
   void           *data;
   void          (*destructor)(DrawableImage *im, void *data);
   DrawableImageTag  *next;
};

struct _drawableimage
{
   char             *file;
   int               w, h;
   DATA32           *data;
   DrawableImageFlags   flags;
   time_t            moddate;
   DrawableBorder       border;
   int               references;
   DrawableLoader      *loader;
   char             *format;
   DrawableImage       *next;
   DrawableImageTag    *tags;
   char             *real_file;
   char             *key;
};

# ifdef BUILD_X11
struct _drawableimagepixmap
{
   int               w, h;
   Pixmap            pixmap, mask;
   Display          *display;
   Visual           *visual;
   int               depth;
   int               source_x, source_y, source_w, source_h;
   Colormap          colormap;
   char              antialias, hi_quality, dither_mask;
   DrawableBorder       border;
   DrawableImage       *image;
   char             *file;
   char              dirty;
   int               references;
   DATABIG           modification_count;
   DrawableImagePixmap *next;
};
# endif

struct _drawableloader
{
   char         *file;
   int           num_formats;
   char        **formats;
   void         *handle;
   char        (*load)(DrawableImage *im,
		       DrawableProgressFunction progress,
		       char progress_granularity, char immediate_load);
   char        (*save)(DrawableImage *im,
		       DrawableProgressFunction progress,
		       char progress_granularity);
   DrawableLoader  *next;
};

EAPI     void              __drawable_AttachTag(DrawableImage *im, const char *key,
					     int val, void *data,
					     DrawableDataDestructorFunction destructor);
EAPI     DrawableImageTag    *__drawable_GetTag(DrawableImage *im, const char *key);
__hidden DrawableImageTag    *__drawable_RemoveTag(DrawableImage *im, const char *key);
__hidden void              __drawable_FreeTag(DrawableImage *im, DrawableImageTag *t);
__hidden void              __drawable_FreeAllTags(DrawableImage *im);

__hidden void              __drawable_SetCacheSize(int size);
__hidden int               __drawable_GetCacheSize(void);
__hidden DrawableImage       *__drawable_ProduceImage(void);
__hidden void              __drawable_ConsumeImage(DrawableImage *im);
__hidden DrawableImage       *__drawable_FindCachedImage(const char *file);
__hidden void              __drawable_AddImageToCache(DrawableImage *im);
__hidden void              __drawable_RemoveImageFromCache(DrawableImage *im);
__hidden int               __drawable_CurrentCacheSize(void);
__hidden void              __drawable_CleanupImageCache(void);
# ifdef BUILD_X11
__hidden DrawableImagePixmap *__drawable_ProduceImagePixmap(void);
__hidden void              __drawable_ConsumeImagePixmap(DrawableImagePixmap *ip);
__hidden DrawableImagePixmap *__drawable_FindCachedImagePixmap(DrawableImage *im, int w, int h, 
						Display *d, Visual *v,
						int depth, int sx, int sy, 
						int sw, int sh, Colormap cm,
						char aa, char hiq, char dmask,
						DATABIG modification_count);
__hidden DrawableImagePixmap *__drawable_FindCachedImagePixmapByID(Display *d, Pixmap p);
__hidden void              __drawable_AddImagePixmapToCache(DrawableImagePixmap *ip);
__hidden void              __drawable_RemoveImagePixmapFromCache(DrawableImagePixmap *ip);
__hidden void              __drawable_CleanupImagePixmapCache(void);
# endif
__hidden DrawableLoader      *__drawable_ProduceLoader(char *file);
__hidden char            **__drawable_ListLoaders(int *num_ret);
__hidden char            **__drawable_TrimLoaderList(char **list, int *num);
__hidden int               __drawable_ItemInList(char **list, int size, char *item);
__hidden void              __drawable_ConsumeLoader(DrawableLoader *l);
__hidden void              __drawable_RescanLoaders(void);
__hidden void              __drawable_RemoveAllLoaders(void);
__hidden void              __drawable_LoadAllLoaders(void);
EAPI     DrawableLoader      *__drawable_FindBestLoaderForFile(const char *file, int for_save);
__hidden DrawableLoader      *__drawable_FindBestLoaderForFileFormat(const char *file, char *format, int for_save);
__hidden void              __drawable_SetImageAlphaFlag(DrawableImage *im, char alpha);
__hidden DrawableImage       *__drawable_CreateImage(int w, int h, DATA32 *data);
__hidden DrawableImage       *__drawable_LoadImage(const char *file,
				    DrawableProgressFunction progress,
				    char progress_granularity, char immediate_load,
				    char dont_cache, DrawableLoadError *er);
# ifdef BUILD_X11
__hidden DrawableImagePixmap *__drawable_FindDrawableImagePixmapByID(Display *d, Pixmap p);
# endif
__hidden void              __drawable_FreeImage(DrawableImage *im);
# ifdef BUILD_X11
__hidden void              __drawable_FreePixmap(Display *d, Pixmap p);
# endif
__hidden void              __drawable_FlushCache(void);
# ifdef BUILD_X11
__hidden void              __drawable_DirtyPixmapsForImage(DrawableImage *im);
# else
#  define	__drawable_DirtyPixmapsForImage(x)	/* x */
# endif
__hidden void              __drawable_DirtyImage(DrawableImage *im);
__hidden void              __drawable_SaveImage(DrawableImage *im, const char *file,
				    DrawableProgressFunction progress,
		                    char progress_granularity,
		                    DrawableLoadError *er);

# define IMAGE_HAS_ALPHA(im) ((im)->flags & F_HAS_ALPHA)
# define IMAGE_IS_UNLOADED(im) ((im)->flags & F_UNLOADED)
# define IMAGE_IS_UNCACHEABLE(im) ((im)->flags & F_UNCACHEABLE)
# define IMAGE_ALWAYS_CHECK_DISK(im) ((im)->flags & F_ALWAYS_CHECK_DISK)
# define IMAGE_IS_VALID(im) (!((im)->flags & F_INVALID))
# define IMAGE_FREE_DATA(im) (!((im)->flags & F_DONT_FREE_DATA))

# define SET_FLAG(flags, f) ((flags) |= (f))
# define UNSET_FLAG(flags, f) ((flags) &= (~f))

#endif

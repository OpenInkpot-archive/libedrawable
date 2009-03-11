
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

/* TODO separate fonts and data stuff */

typedef struct _Drawable_Font 		DrawableFont;
typedef struct _Drawable_Font_Glyph 	Drawable_Font_Glyph;

typedef struct _Drawable_Object_List 	Drawable_Object_List;
typedef struct _Drawable_Hash 		Drawable_Hash;
typedef struct _Drawable_Hash_El 		Drawable_Hash_El;

struct _Drawable_Object_List
{
   Drawable_Object_List  *next, *prev;
   Drawable_Object_List  *last;
};

struct _Drawable_Hash
{
   int                 population;
   Drawable_Object_List  *buckets[256];
};

struct _Drawable_Hash_El
{
   Drawable_Object_List   _list_data;
   char               *key;
   void               *data;
};

struct _Drawable_Font
{
   Drawable_Object_List   _list_data;
   char               *name;
   char               *file;
   int                 size;

   struct
   {
      FT_Face             face;
   }
   ft;

   Drawable_Hash         *glyphs;

   int                 usage;

   int                 references;

   /* using a double-linked list for the fallback chain */
   struct _Drawable_Font	*fallback_prev;
   struct _Drawable_Font	*fallback_next;
};

struct _Drawable_Font_Glyph
{
   FT_Glyph            glyph;
   FT_BitmapGlyph      glyph_out;
};

/* functions */

void                drawable_font_init(void);
int                 drawable_font_ascent_get(DrawableFont * fn);
int                 drawable_font_descent_get(DrawableFont * fn);
int                 drawable_font_max_ascent_get(DrawableFont * fn);
int                 drawable_font_max_descent_get(DrawableFont * fn);
int                 drawable_font_get_line_advance(DrawableFont * fn);
int                 drawable_font_utf8_get_next(unsigned char *buf, int *iindex);
void                drawable_font_add_font_path(const char *path);
void                drawable_font_del_font_path(const char *path);
int                 drawable_font_path_exists(const char *path);
char              **drawable_font_list_font_path(int *num_ret);
char              **drawable_font_list_fonts(int *num_ret);

DrawableFont          *drawable_font_load_joined(const char *name);
void                drawable_font_free(DrawableFont * fn);
int                 drawable_font_insert_into_fallback_chain_imp(DrawableFont * fn,
                                                              DrawableFont * fallback);
void                drawable_font_remove_from_fallback_chain_imp(DrawableFont * fn);
int                 drawable_font_cache_get(void);
void                drawable_font_cache_set(int size);
void                drawable_font_flush(void);
void                drawable_font_modify_cache_by(DrawableFont * fn, int dir);
void                drawable_font_modify_cache_by(DrawableFont * fn, int dir);
void                drawable_font_flush_last(void);
DrawableFont          *drawable_font_find(const char *name, int size);
DrawableFont          *drawable_font_find_glyph(DrawableFont * fn, int gl, unsigned int *ret_index);

void                drawable_font_query_size(DrawableFont * fn, const char *text,
					  int *w, int *h);
int                 drawable_font_query_inset(DrawableFont * fn, const char *text);
void                drawable_font_query_advance(DrawableFont * fn, const char *text,
					     int *h_adv, int *v_adv);
int                 drawable_font_query_char_coords(DrawableFont * fn,
						 const char *text, int pos,
						 int *cx, int *cy, int *cw,
						 int *ch);
int                 drawable_font_query_text_at_pos(DrawableFont * fn,
						 const char *text, int x, int y,
						 int *cx, int *cy, int *cw,
						 int *ch);

Drawable_Font_Glyph   *drawable_font_cache_glyph_get(DrawableFont * fn, FT_UInt index);
void                drawable_render_str(DrawableImage * im, DrawableFont * f, int drx,
				     int dry, const char *text, DATA8 r,
				     DATA8 g, DATA8 b, DATA8 a, char dir,
				     double angle, int *retw, int *reth,
				     int blur, int *nextx, int *nexty,
				     DrawableOp op, int clx, int cly, int clw,
				     int clh);
void                drawable_font_draw(DrawableImage * dst, DATA32 col,
				    DrawableFont * fn, int x, int y,
				    const char *text, int *nextx, int *nexty,
				    int clx, int cly, int clw, int clh);

/* data manipulation */

void               *drawable_object_list_prepend(void *in_list, void *in_item);
void               *drawable_object_list_remove(void *in_list, void *in_item);
Drawable_Hash         *drawable_hash_add(Drawable_Hash * hash, const char *key,
				   const void *data);
void               *drawable_hash_find(Drawable_Hash * hash, const char *key);
void                drawable_hash_free(Drawable_Hash * hash);
void                drawable_hash_foreach(Drawable_Hash * hash,
				       int (*func) (Drawable_Hash * hash,
						    const char *key, void *data,
						    void *fdata),
				       const void *fdata);

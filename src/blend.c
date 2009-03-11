#include "common.h"
#include "colormod.h"
#include "image.h"
#include "blend.h"
#include "scale.h"

#define ADD_COPY(r, g, b, dest) \
                ADD_COLOR(R_VAL(dest), r, R_VAL(dest)); \
                ADD_COLOR(G_VAL(dest), g, G_VAL(dest)); \
                ADD_COLOR(B_VAL(dest), b, B_VAL(dest));

#define SUB_COPY(r, g, b, dest) \
                SUB_COLOR(R_VAL(dest), r, R_VAL(dest)); \
                SUB_COLOR(G_VAL(dest), g, G_VAL(dest)); \
                SUB_COLOR(B_VAL(dest), b, B_VAL(dest));

#define RE_COPY(r, g, b, dest) \
                RESHADE_COLOR(R_VAL(dest), r, R_VAL(dest)); \
                RESHADE_COLOR(G_VAL(dest), g, G_VAL(dest)); \
                RESHADE_COLOR(B_VAL(dest), b, B_VAL(dest));

int                 pow_lut_initialized = 0;
DATA8               pow_lut[256][256];

void
__drawable_build_pow_lut(void)
{
   int                 i, j;

   if (pow_lut_initialized)
      return;
   pow_lut_initialized = 1;
   for (i = 0; i < 256; i++)
     {
        for (j = 0; j < 256; j++)
/*	   pow_lut[i][j] = 255 * pow((double)i / 255, (double)j / 255);  */
          {
             int                 divisor;

             divisor = (i + (j * (255 - i)) / 255);
             if (divisor > 0)
                pow_lut[i][j] = (i * 255) / divisor;
             else
                pow_lut[i][j] = 0;
          }
     }
}

/* COPY OPS */

static void
__drawable_BlendRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                       int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = A_VAL(src);
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  *dst = (*dst & 0xff000000) | (*src & 0x00ffffff);
                  break;
               default:
                  BLEND(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_BlendRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                        int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = A_VAL(src);
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  *dst = *src;
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_CopyRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                      int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             *dst = (*dst & 0xff000000) | (*src & 0x00ffffff);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_CopyRGBToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                      int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             *dst = 0xff000000 | (*src & 0x00ffffff);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_CopyRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                       int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             *dst = *src;
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* ADD OPS */

static void
__drawable_AddBlendRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = A_VAL(src);
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  ADD_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
                  break;
               default:
                  BLEND_ADD(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddBlendRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                           int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = A_VAL(src);
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  ADD_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND_ADD(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddCopyRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                         int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             ADD_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddCopyRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = A_VAL(src);
             ADD_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddCopyRGBToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                         int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = 0xff;
             ADD_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* SUBTRACT OPS */

static void
__drawable_SubBlendRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = A_VAL(src);
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  SUB_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
                  break;
               default:
                  BLEND_SUB(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubBlendRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                           int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = A_VAL(src);
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  SUB_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND_SUB(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubCopyRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                         int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             SUB_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubCopyRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = A_VAL(src);
             SUB_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubCopyRGBToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                         int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = 0xff;
             SUB_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* RESHADE OPS */

static void
__drawable_ReBlendRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                         int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = A_VAL(src);
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  RE_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
                  break;
               default:
                  BLEND_RE(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReBlendRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = A_VAL(src);
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  RE_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND_RE(R_VAL(src), G_VAL(src), B_VAL(src), a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReCopyRGBAToRGB(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                        int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             RE_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReCopyRGBAToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                         int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = A_VAL(src);
             RE_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReCopyRGBToRGBA(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                        int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = 0xff;
             RE_COPY(R_VAL(src), G_VAL(src), B_VAL(src), dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* WITH COLOMOD */
/* COPY OPS */

static void
__drawable_BlendRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                           int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = amod[A_VAL(src)];
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  R_VAL(dst) = rmod[R_VAL(src)];
                  G_VAL(dst) = gmod[G_VAL(src)];
                  B_VAL(dst) = bmod[B_VAL(src)];
                  break;
               default:
                  BLEND(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], a,
                        dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_BlendRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                            int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = amod[A_VAL(src)];
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  R_VAL(dst) = rmod[R_VAL(src)];
                  G_VAL(dst) = gmod[G_VAL(src)];
                  B_VAL(dst) = bmod[B_VAL(src)];
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], a,
                        dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_BlendRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                           int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = pow_lut[am][A_VAL(dst)];
             BLEND_COLOR(am, A_VAL(dst), 255, A_VAL(dst))
                BLEND(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], a,
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_BlendRGBToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             BLEND(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], am,
                   dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_CopyRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             R_VAL(dst) = rmod[R_VAL(src)];
             G_VAL(dst) = gmod[G_VAL(src)];
             B_VAL(dst) = bmod[B_VAL(src)];
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_CopyRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                          int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             A_VAL(dst) = am;
             R_VAL(dst) = rmod[R_VAL(src)];
             G_VAL(dst) = gmod[G_VAL(src)];
             B_VAL(dst) = bmod[B_VAL(src)];
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_CopyRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                           int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             A_VAL(dst) = amod[A_VAL(src)];
             R_VAL(dst) = rmod[R_VAL(src)];
             G_VAL(dst) = gmod[G_VAL(src)];
             B_VAL(dst) = bmod[B_VAL(src)];
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* ADD OPS */

static void
__drawable_AddBlendRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = amod[A_VAL(src)];
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  ADD_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                           dst);
                  break;
               default:
                  BLEND_ADD(rmod[R_VAL(src)], gmod[G_VAL(src)],
                            bmod[B_VAL(src)], a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddBlendRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                               int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = amod[A_VAL(src)];
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  ADD_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                           dst);
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND_ADD(rmod[R_VAL(src)], gmod[G_VAL(src)],
                            bmod[B_VAL(src)], a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddBlendRGBToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             BLEND_ADD(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], am,
                       dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddBlendRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = pow_lut[am][A_VAL(dst)];
             BLEND_COLOR(am, A_VAL(dst), 255, A_VAL(dst));
             BLEND_ADD(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], a,
                       dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddCopyRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             ADD_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddCopyRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = amod[A_VAL(src)];
             ADD_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_AddCopyRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = am;
             ADD_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* SUBTRACT OPS */

static void
__drawable_SubBlendRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = amod[A_VAL(src)];
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  SUB_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                           dst);
                  break;
               default:
                  BLEND_SUB(rmod[R_VAL(src)], gmod[G_VAL(src)],
                            bmod[B_VAL(src)], a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubBlendRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                               int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = amod[A_VAL(src)];
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  SUB_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                           dst);
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND_SUB(rmod[R_VAL(src)], gmod[G_VAL(src)],
                            bmod[B_VAL(src)], a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubBlendRGBToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             BLEND_SUB(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], am,
                       dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubBlendRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = pow_lut[am][A_VAL(dst)];
             BLEND_COLOR(am, A_VAL(dst), 255, A_VAL(dst));
             BLEND_SUB(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], a,
                       dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubCopyRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             SUB_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubCopyRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = amod[A_VAL(src)];
             SUB_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_SubCopyRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = am;
             SUB_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/* RESHADE OPS */

static void
__drawable_ReBlendRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = amod[A_VAL(src)];
             switch (a)
               {
               case 0:
                  break;
               case 255:
                  RE_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                          dst);
                  break;
               default:
                  BLEND_RE(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                           a, dst);
                  break;
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReBlendRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                              int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             aa = amod[A_VAL(src)];
             switch (aa)
               {
               case 0:
                  break;
               case 255:
                  A_VAL(dst) = 0xff;
                  RE_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                          dst);
                  break;
               default:
                  a = pow_lut[aa][A_VAL(dst)];
                  BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                  BLEND_RE(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)],
                           a, dst);
               }
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReBlendRGBToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                            int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             BLEND_RE(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], am,
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReBlendRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;
             DATA8               a;

             a = pow_lut[am][A_VAL(dst)];
             BLEND_COLOR(am, A_VAL(dst), 255, A_VAL(dst));
             BLEND_RE(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], a,
                      dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReCopyRGBAToRGBCmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                            int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             RE_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReCopyRGBAToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                             int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = amod[A_VAL(src)];
             RE_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

static void
__drawable_ReCopyRGBToRGBACmod(DATA32 * src, int srcw, DATA32 * dst, int dstw,
                            int w, int h, DrawableColorModifier * cm)
{
   int                 src_step = (srcw - w), dst_step = (dstw - w), ww = w;
   DATA8              *amod = cm->alpha_mapping, *rmod = cm->red_mapping,
      *gmod = cm->green_mapping, *bmod = cm->blue_mapping;
   DATA8               am = amod[255];

   while (h--)
     {
        while (w--)
          {
             DATA32              tmp;

             A_VAL(dst) = am;
             RE_COPY(rmod[R_VAL(src)], gmod[G_VAL(src)], bmod[B_VAL(src)], dst);
             src++;
             dst++;
          }
        src += src_step;
        dst += dst_step;
        w = ww;
     }
}

/*\ Equivalent functions \*/

#define __drawable_CopyRGBToRGB			__drawable_CopyRGBToRGBA
#define __drawable_BlendRGBToRGB			__drawable_CopyRGBToRGB
#define __drawable_BlendRGBToRGBA			__drawable_CopyRGBToRGBA
#define __drawable_mmx_copy_rgb_to_rgb		__drawable_mmx_copy_rgb_to_rgba
#define __drawable_mmx_blend_rgb_to_rgb		__drawable_mmx_copy_rgb_to_rgb
#define __drawable_mmx_blend_rgb_to_rgba		__drawable_mmx_copy_rgb_to_rgba
#define __drawable_amd64_copy_rgb_to_rgb		__drawable_amd64_copy_rgb_to_rgba
#define __drawable_amd64_blend_rgb_to_rgb		__drawable_amd64_copy_rgb_to_rgb
#define __drawable_amd64_blend_rgb_to_rgba		__drawable_amd64_copy_rgb_to_rgba
#define __drawable_CopyRGBToRGBCmod		__drawable_CopyRGBAToRGBCmod
#define __drawable_mmx_copy_rgb_to_rgb_cmod	__drawable_mmx_copy_rgba_to_rgb_cmod
#define __drawable_amd64_copy_rgb_to_rgb_cmod	__drawable_amd64_copy_rgba_to_rgb_cmod

#define __drawable_AddCopyRGBToRGB			__drawable_AddCopyRGBAToRGB
#define __drawable_AddBlendRGBToRGB		__drawable_AddCopyRGBToRGB
#define __drawable_AddBlendRGBToRGBA		__drawable_AddCopyRGBToRGBA
#define __drawable_mmx_add_copy_rgb_to_rgb		__drawable_mmx_add_copy_rgba_to_rgb
#define __drawable_mmx_add_blend_rgb_to_rgb	__drawable_mmx_add_copy_rgb_to_rgb
#define __drawable_mmx_add_blend_rgb_to_rgba	__drawable_mmx_add_copy_rgb_to_rgba
#define __drawable_amd64_add_copy_rgb_to_rgb      	__drawable_amd64_add_copy_rgba_to_rgb
#define __drawable_amd64_add_blend_rgb_to_rgb	__drawable_amd64_add_copy_rgb_to_rgb
#define __drawable_amd64_add_blend_rgb_to_rgba	__drawable_amd64_add_copy_rgb_to_rgba
#define __drawable_AddCopyRGBToRGBCmod		__drawable_AddCopyRGBAToRGBCmod
#define __drawable_mmx_add_copy_rgb_to_rgb_cmod	__drawable_mmx_add_copy_rgb_to_rgba_cmod
#define __drawable_amd64_add_copy_rgb_to_rgb_cmod	__drawable_amd64_add_copy_rgb_to_rgba_cmod

#define __drawable_SubCopyRGBToRGB			__drawable_SubCopyRGBAToRGB
#define __drawable_SubBlendRGBToRGB		__drawable_SubCopyRGBToRGB
#define __drawable_SubBlendRGBToRGBA		__drawable_SubCopyRGBToRGBA
#define __drawable_mmx_subtract_copy_rgb_to_rgba	__drawable_mmx_subtract_copy_rgba_to_rgba
#define __drawable_mmx_subtract_copy_rgb_to_rgb	__drawable_mmx_subtract_copy_rgba_to_rgb
#define __drawable_mmx_subtract_blend_rgb_to_rgb	__drawable_mmx_subtract_copy_rgb_to_rgb
#define __drawable_mmx_subtract_blend_rgb_to_rgba	__drawable_mmx_subtract_copy_rgb_to_rgba
#define __drawable_amd64_subtract_copy_rgb_to_rgb	__drawable_amd64_subtract_copy_rgba_to_rgb
#define __drawable_amd64_subtract_blend_rgb_to_rgb	__drawable_amd64_subtract_copy_rgb_to_rgb
#define __drawable_amd64_subtract_blend_rgb_to_rgba	__drawable_amd64_subtract_copy_rgb_to_rgba
#define __drawable_SubCopyRGBToRGBCmod		__drawable_SubCopyRGBAToRGBCmod
#define __drawable_mmx_subtract_copy_rgb_to_rgb_cmod	__drawable_mmx_subtract_copy_rgb_to_rgba_cmod
#define __drawable_amd64_subtract_copy_rgb_to_rgb_cmod	__drawable_amd64_subtract_copy_rgb_to_rgba_cmod

#define __drawable_ReCopyRGBToRGB			__drawable_ReCopyRGBAToRGB
#define __drawable_ReBlendRGBToRGB			__drawable_ReCopyRGBToRGB
#define __drawable_ReBlendRGBToRGBA		__drawable_ReCopyRGBToRGBA
#define __drawable_mmx_reshade_copy_rgb_to_rgba	__drawable_mmx_reshade_copy_rgba_to_rgba
#define __drawable_mmx_reshade_copy_rgb_to_rgb	__drawable_mmx_reshade_copy_rgba_to_rgb
#define __drawable_mmx_reshade_blend_rgb_to_rgb	__drawable_mmx_reshade_copy_rgb_to_rgb
#define __drawable_mmx_reshade_blend_rgb_to_rgba	__drawable_mmx_reshade_copy_rgb_to_rgba
#define __drawable_amd64_reshade_copy_rgb_to_rgb	__drawable_amd64_reshade_copy_rgba_to_rgb
#define __drawable_amd64_reshade_blend_rgb_to_rgb	__drawable_amd64_reshade_copy_rgb_to_rgb
#define __drawable_amd64_reshade_blend_rgb_to_rgba	__drawable_amd64_reshade_copy_rgb_to_rgba
#define __drawable_ReCopyRGBToRGBCmod		__drawable_ReCopyRGBAToRGBCmod
#define __drawable_mmx_reshade_copy_rgb_to_rgb_cmod	__drawable_mmx_reshade_copy_rgb_to_rgba_cmod
#define __drawable_amd64_reshade_copy_rgb_to_rgb_cmod	__drawable_amd64_reshade_copy_rgb_to_rgba_cmod

DrawableBlendFunction
__drawable_GetBlendFunction(DrawableOp op, char blend, char merge_alpha, char rgb_src,
                         DrawableColorModifier * cm)
{
   /*\ [ mmx ][ operation ][ cmod ][ merge_alpha ][ rgb_src ][ blend ] \ */
   static DrawableBlendFunction ibfuncs[][4][2][2][2][2] = {
      /*\ OP_COPY \ */
      {{{{{__drawable_CopyRGBAToRGB, __drawable_BlendRGBAToRGB},
          {__drawable_CopyRGBToRGB, __drawable_BlendRGBToRGB}},
         {{__drawable_CopyRGBAToRGBA, __drawable_BlendRGBAToRGBA},
          {__drawable_CopyRGBToRGBA, __drawable_BlendRGBToRGBA}}},

        {{{__drawable_CopyRGBAToRGBCmod, __drawable_BlendRGBAToRGBCmod},
          {__drawable_CopyRGBToRGBCmod, __drawable_BlendRGBToRGBCmod}},
         {{__drawable_CopyRGBAToRGBACmod, __drawable_BlendRGBAToRGBACmod},
          {__drawable_CopyRGBToRGBACmod, __drawable_BlendRGBToRGBACmod}}}},
       /*\ OP_ADD \ */
       {{{{__drawable_AddCopyRGBAToRGB, __drawable_AddBlendRGBAToRGB},
          {__drawable_AddCopyRGBToRGB, __drawable_AddBlendRGBToRGB}},
         {{__drawable_AddCopyRGBAToRGBA, __drawable_AddBlendRGBAToRGBA},
          {__drawable_AddCopyRGBToRGBA, __drawable_AddBlendRGBToRGBA}}},

        {{{__drawable_AddCopyRGBAToRGBCmod, __drawable_AddBlendRGBAToRGBCmod},
          {__drawable_AddCopyRGBToRGBCmod, __drawable_AddBlendRGBToRGBCmod}},
         {{__drawable_AddCopyRGBAToRGBACmod, __drawable_AddBlendRGBAToRGBACmod},
          {__drawable_AddCopyRGBToRGBACmod, __drawable_AddBlendRGBToRGBACmod}}}},
       /*\ OP_SUBTRACT \ */
       {{{{__drawable_SubCopyRGBAToRGB, __drawable_SubBlendRGBAToRGB},
          {__drawable_SubCopyRGBToRGB, __drawable_SubBlendRGBToRGB}},
         {{__drawable_SubCopyRGBAToRGBA, __drawable_SubBlendRGBAToRGBA},
          {__drawable_SubCopyRGBToRGBA, __drawable_SubBlendRGBToRGBA}}},

        {{{__drawable_SubCopyRGBAToRGBCmod, __drawable_SubBlendRGBAToRGBCmod},
          {__drawable_SubCopyRGBToRGBCmod, __drawable_SubBlendRGBToRGBCmod}},
         {{__drawable_SubCopyRGBAToRGBACmod, __drawable_SubBlendRGBAToRGBACmod},
          {__drawable_SubCopyRGBToRGBACmod, __drawable_SubBlendRGBToRGBACmod}}}},
       /*\ OP_RESHADE \ */
       {{{{__drawable_ReCopyRGBAToRGB, __drawable_ReBlendRGBAToRGB},
          {__drawable_ReCopyRGBToRGB, __drawable_ReBlendRGBToRGB}},
         {{__drawable_ReCopyRGBAToRGBA, __drawable_ReBlendRGBAToRGBA},
          {__drawable_ReCopyRGBToRGBA, __drawable_ReBlendRGBToRGBA}}},

        {{{__drawable_ReCopyRGBAToRGBCmod, __drawable_ReBlendRGBAToRGBCmod},
          {__drawable_ReCopyRGBToRGBCmod, __drawable_ReBlendRGBToRGBCmod}},
         {{__drawable_ReCopyRGBAToRGBACmod, __drawable_ReBlendRGBAToRGBACmod},
          {__drawable_ReCopyRGBToRGBACmod, __drawable_ReBlendRGBToRGBACmod}}}}},

#ifdef DO_MMX_ASM
      /*\ OP_COPY \ */
      {{{{{__drawable_mmx_copy_rgba_to_rgb, __drawable_mmx_blend_rgba_to_rgb},
          {__drawable_mmx_copy_rgb_to_rgb, __drawable_mmx_blend_rgb_to_rgb}},
         {{__drawable_mmx_copy_rgba_to_rgba,
           __drawable_BlendRGBAToRGBA /*__drawable_mmx_blend_rgba_to_rgba*/ },
          {__drawable_mmx_copy_rgb_to_rgba, __drawable_mmx_blend_rgb_to_rgba}}},

        {{{__drawable_mmx_copy_rgba_to_rgb_cmod,
           __drawable_mmx_blend_rgba_to_rgb_cmod},
          {__drawable_mmx_copy_rgb_to_rgb_cmod,
           __drawable_mmx_blend_rgb_to_rgb_cmod}},
         {{__drawable_mmx_copy_rgba_to_rgba_cmod,
           __drawable_BlendRGBAToRGBACmod /*__drawable_mmx_blend_rgba_to_rgba_cmod*/
           },
          {__drawable_mmx_copy_rgb_to_rgba_cmod,
           __drawable_BlendRGBToRGBACmod /*__drawable_mmx_blend_rgb_to_rgba_cmod*/
           }}}},
       /*\ OP_ADD \ */
       {{{{__drawable_mmx_add_copy_rgba_to_rgb, __drawable_mmx_add_blend_rgba_to_rgb},
          {__drawable_mmx_add_copy_rgb_to_rgb, __drawable_mmx_add_blend_rgb_to_rgb}},
         {{__drawable_AddCopyRGBAToRGBA /*__drawable_mmx_add_copy_rgba_to_rgba*/ ,
           __drawable_AddBlendRGBAToRGBA /*__drawable_mmx_add_blend_rgba_to_rgba*/ },
          {__drawable_mmx_add_copy_rgb_to_rgba,
           __drawable_mmx_add_blend_rgb_to_rgba}}},

        {{{__drawable_mmx_add_copy_rgba_to_rgb_cmod,
           __drawable_mmx_add_blend_rgba_to_rgb_cmod},
          {__drawable_mmx_add_copy_rgb_to_rgb_cmod,
           __drawable_mmx_add_blend_rgb_to_rgb_cmod}},
         {{__drawable_AddCopyRGBAToRGBACmod
           /*__drawable_mmx_add_copy_rgba_to_rgba_cmod*/ ,
           __drawable_AddBlendRGBAToRGBACmod
           /*__drawable_mmx_add_blend_rgba_to_rgba_cmod*/ },
          {__drawable_AddCopyRGBToRGBACmod
           /*__drawable_mmx_add_copy_rgb_to_rgba_cmod*/ ,
           __drawable_AddBlendRGBToRGBACmod
           /*__drawable_mmx_add_blend_rgb_to_rgba_cmod*/ }}}},
       /*\ OP_SUBTRACT \ */
       {{{{__drawable_mmx_subtract_copy_rgba_to_rgb,
           __drawable_mmx_subtract_blend_rgba_to_rgb},
          {__drawable_mmx_subtract_copy_rgb_to_rgb,
           __drawable_mmx_subtract_blend_rgb_to_rgb}},
         {{__drawable_SubCopyRGBAToRGBA /*__drawable_mmx_subtract_copy_rgba_to_rgba*/
           ,
           __drawable_SubBlendRGBAToRGBA
           /*__drawable_mmx_subtract_blend_rgba_to_rgba*/ },
          {__drawable_mmx_subtract_copy_rgb_to_rgba,
           __drawable_mmx_subtract_blend_rgb_to_rgba}}},

        {{{__drawable_mmx_subtract_copy_rgba_to_rgb_cmod,
           __drawable_mmx_subtract_blend_rgba_to_rgb_cmod},
          {__drawable_mmx_subtract_copy_rgb_to_rgb_cmod,
           __drawable_mmx_subtract_blend_rgb_to_rgb_cmod}},
         {{__drawable_SubCopyRGBAToRGBACmod
           /*__drawable_mmx_subtract_copy_rgba_to_rgba_cmod*/ ,
           __drawable_SubBlendRGBAToRGBACmod
           /*__drawable_mmx_subtract_blend_rgba_to_rgba_cmod*/ },
          {__drawable_SubCopyRGBToRGBACmod
           /*__drawable_mmx_subtract_copy_rgb_to_rgba_cmod*/ ,
           __drawable_SubBlendRGBToRGBACmod
           /*__drawable_mmx_subtract_blend_rgb_to_rgba_cmod*/ }}}},
       /*\ OP_RESHADE \ */
       {{{{__drawable_mmx_reshade_copy_rgba_to_rgb,
           __drawable_mmx_reshade_blend_rgba_to_rgb},
          {__drawable_mmx_reshade_copy_rgb_to_rgb,
           __drawable_mmx_reshade_blend_rgb_to_rgb}},
         {{__drawable_ReCopyRGBAToRGBA /*__drawable_mmx_reshade_copy_rgba_to_rgba*/ ,
           __drawable_ReBlendRGBAToRGBA /*__drawable_mmx_reshade_blend_rgba_to_rgba*/
           },
          {__drawable_mmx_reshade_copy_rgb_to_rgba,
           __drawable_mmx_reshade_blend_rgb_to_rgba}}},

        {{{__drawable_mmx_reshade_copy_rgba_to_rgb_cmod,
           __drawable_mmx_reshade_blend_rgba_to_rgb_cmod},
          {__drawable_mmx_reshade_copy_rgb_to_rgb_cmod,
           __drawable_mmx_reshade_blend_rgb_to_rgb_cmod}},
         {{__drawable_ReCopyRGBAToRGBACmod
           /*__drawable_mmx_reshade_copy_rgba_to_rgba_cmod*/ ,
           __drawable_ReBlendRGBAToRGBACmod
           /*__drawable_mmx_reshade_blend_rgba_to_rgba_cmod*/ },
          {__drawable_ReCopyRGBToRGBACmod
           /*__drawable_mmx_reshade_copy_rgb_to_rgba_cmod*/ ,
           __drawable_ReBlendRGBToRGBACmod
           /*__drawable_mmx_reshade_blend_rgb_to_rgba_cmod*/ }}}}},
#elif DO_AMD64_ASM
      /*\ OP_COPY \ */
      {{{{{__drawable_amd64_copy_rgba_to_rgb, __drawable_amd64_blend_rgba_to_rgb},
          {__drawable_amd64_copy_rgb_to_rgb, __drawable_amd64_blend_rgb_to_rgb}},
         {{__drawable_amd64_copy_rgba_to_rgba, __drawable_amd64_blend_rgba_to_rgba},
          {__drawable_amd64_copy_rgb_to_rgba, __drawable_amd64_blend_rgb_to_rgba}}},

        {{{__drawable_amd64_copy_rgba_to_rgb_cmod,
           __drawable_amd64_blend_rgba_to_rgb_cmod},
          {__drawable_amd64_copy_rgb_to_rgb_cmod,
           __drawable_amd64_blend_rgb_to_rgb_cmod}},
         {{__drawable_amd64_copy_rgba_to_rgba_cmod,
           __drawable_amd64_blend_rgba_to_rgba_cmod},
          {__drawable_amd64_copy_rgb_to_rgba_cmod,
           __drawable_amd64_blend_rgb_to_rgba_cmod}}}},
       /*\ OP_ADD \ */
       {{{{__drawable_amd64_add_copy_rgba_to_rgb,
           __drawable_amd64_add_blend_rgba_to_rgb},
          {__drawable_amd64_add_copy_rgb_to_rgb,
           __drawable_amd64_add_blend_rgb_to_rgb}},
         {{__drawable_amd64_add_copy_rgba_to_rgba,
           __drawable_amd64_add_blend_rgba_to_rgba},
          {__drawable_amd64_add_copy_rgb_to_rgba,
           __drawable_amd64_add_blend_rgb_to_rgba}}},

        {{{__drawable_amd64_add_copy_rgba_to_rgb_cmod,
           __drawable_amd64_add_blend_rgba_to_rgb_cmod},
          {__drawable_amd64_add_copy_rgb_to_rgb_cmod,
           __drawable_amd64_add_blend_rgb_to_rgb_cmod}},
         {{__drawable_amd64_add_copy_rgba_to_rgba_cmod,
           __drawable_amd64_add_blend_rgba_to_rgba_cmod},
          {__drawable_amd64_add_copy_rgb_to_rgba_cmod,
           __drawable_amd64_add_blend_rgb_to_rgba_cmod}}}},
       /*\ OP_SUBTRACT \ */
       {{{{__drawable_amd64_subtract_copy_rgba_to_rgb,
           __drawable_amd64_subtract_blend_rgba_to_rgb},
          {__drawable_amd64_subtract_copy_rgb_to_rgb,
           __drawable_amd64_subtract_blend_rgb_to_rgb}},
         {{__drawable_amd64_subtract_copy_rgba_to_rgba,
           __drawable_amd64_subtract_blend_rgba_to_rgba},
          {__drawable_amd64_subtract_copy_rgb_to_rgba,
           __drawable_amd64_subtract_blend_rgb_to_rgba}}},

        {{{__drawable_amd64_subtract_copy_rgba_to_rgb_cmod,
           __drawable_amd64_subtract_blend_rgba_to_rgb_cmod},
          {__drawable_amd64_subtract_copy_rgb_to_rgb_cmod,
           __drawable_amd64_subtract_blend_rgb_to_rgb_cmod}},
         {{__drawable_amd64_subtract_copy_rgba_to_rgba_cmod,
           __drawable_amd64_subtract_blend_rgba_to_rgba_cmod},
          {__drawable_amd64_subtract_copy_rgb_to_rgba_cmod,
           __drawable_amd64_subtract_blend_rgb_to_rgba_cmod}}}},
       /*\ OP_RESHADE \ */
       {{{{__drawable_amd64_reshade_copy_rgba_to_rgb,
           __drawable_amd64_reshade_blend_rgba_to_rgb},
          {__drawable_amd64_reshade_copy_rgb_to_rgb,
           __drawable_amd64_reshade_blend_rgb_to_rgb}},
         {{__drawable_amd64_reshade_copy_rgba_to_rgba,
           __drawable_amd64_reshade_blend_rgba_to_rgba},
          {__drawable_amd64_reshade_copy_rgb_to_rgba,
           __drawable_amd64_reshade_blend_rgb_to_rgba}}},

        {{{__drawable_amd64_reshade_copy_rgba_to_rgb_cmod,
           __drawable_amd64_reshade_blend_rgba_to_rgb_cmod},
          {__drawable_amd64_reshade_copy_rgb_to_rgb_cmod,
           __drawable_amd64_reshade_blend_rgb_to_rgb_cmod}},
         {{__drawable_amd64_reshade_copy_rgba_to_rgba_cmod,
           __drawable_amd64_reshade_blend_rgba_to_rgba_cmod},
          {__drawable_amd64_reshade_copy_rgb_to_rgba_cmod,
           __drawable_amd64_reshade_blend_rgb_to_rgba_cmod}}}}},
#endif
   };

   int                 opi = (op == OP_COPY) ? 0
      : (op == OP_ADD) ? 1
      : (op == OP_SUBTRACT) ? 2 : (op == OP_RESHADE) ? 3 : -1;
   int                 do_mmx = 0;

   if (opi == -1)
      return NULL;

#ifdef DO_MMX_ASM
   do_mmx = !!(__drawable_get_cpuid() & CPUID_MMX);
#elif DO_AMD64_ASM
   do_mmx = 1;                  // instruction set is always present
#endif
   if (cm && rgb_src && (A_CMOD(cm, 0xff) == 0xff))
      blend = 0;
   if (blend && cm && rgb_src && (A_CMOD(cm, 0xff) == 0))
      return NULL;
   return ibfuncs[!!do_mmx][opi][!!cm][!!merge_alpha][!!rgb_src][!!blend];
}

void
__drawable_BlendRGBAToData(DATA32 * src, int src_w, int src_h, DATA32 * dst,
                        int dst_w, int dst_h, int sx, int sy, int dx, int dy,
                        int w, int h, char blend, char merge_alpha,
                        DrawableColorModifier * cm, DrawableOp op, char rgb_src)
{
   DrawableBlendFunction  blender;

   if (sx < 0)
     {
        w += sx;
        dx -= sx;
        sx = 0;
     }
   if (sy < 0)
     {
        h += sy;
        dy -= sy;
        sy = 0;
     }
   if (dx < 0)
     {
        w += dx;
        sx -= dx;
        dx = 0;
     }
   if (dy < 0)
     {
        h += dy;
        sy -= dy;
        dy = 0;
     }
   if ((w <= 0) || (h <= 0))
      return;
   if ((sx + w) > src_w)
      w = src_w - sx;
   if ((sy + h) > src_h)
      h = src_h - sy;
   if ((dx + w) > dst_w)
      w = dst_w - dx;
   if ((dy + h) > dst_h)
      h = dst_h - dy;
   if ((w <= 0) || (h <= 0))
      return;

   __drawable_build_pow_lut();
   blender = __drawable_GetBlendFunction(op, blend, merge_alpha, rgb_src, cm);
   if (blender)
      blender(src + (sy * src_w) + sx, src_w,
              dst + (dy * dst_w) + dx, dst_w, w, h, cm);
}

#define LINESIZE 16

void
__drawable_BlendImageToImage(DrawableImage * im_src, DrawableImage * im_dst,
                          char aa, char blend, char merge_alpha,
                          int ssx, int ssy, int ssw, int ssh,
                          int ddx, int ddy, int ddw, int ddh,
                          DrawableColorModifier * cm, DrawableOp op,
                          int clx, int cly, int clw, int clh)
{
   char                rgb_src = 0;

   if ((!(im_src->data)) && (im_src->loader) && (im_src->loader->load))
      im_src->loader->load(im_src, NULL, 0, 1);
   if ((!(im_dst->data)) && (im_dst->loader) && (im_dst->loader->load))
      im_dst->loader->load(im_dst, NULL, 0, 1);
   if (!im_src->data)
      return;
   if (!im_dst->data)
      return;

   if ((ssw == ddw) && (ssh == ddh))
     {
        if (!IMAGE_HAS_ALPHA(im_dst))
           merge_alpha = 0;
        if (!IMAGE_HAS_ALPHA(im_src))
          {
             rgb_src = 1;
             if (merge_alpha)
                blend = 1;
          }
        if (clw)
          {
             int                 px, py;

             px = ddx;
             py = ddy;
             CLIP_TO(ddx, ddy, ddw, ddh, clx, cly, clw, clh);
             px = ddx - px;
             py = ddy - py;
             ssx += px;
             ssy += py;
             if ((ssw < 1) || (ssh < 1))
                return;
             if ((ddw < 1) || (ddh < 1))
                return;
          }

        __drawable_BlendRGBAToData(im_src->data, im_src->w, im_src->h,
                                im_dst->data, im_dst->w, im_dst->h,
                                ssx, ssy,
                                ddx, ddy,
                                ddw, ddh, blend, merge_alpha, cm, op, rgb_src);
     }
   else
     {
        DrawableScaleInfo     *scaleinfo = NULL;
        DATA32             *buf = NULL;
        int                 sx, sy, sw, sh, dx, dy, dw, dh, dxx, dyy, y2, x2;
        int                 psx, psy, psw, psh;
        int                 y, h, hh;

#ifdef DO_MMX_ASM
        int                 do_mmx;
#endif

        sx = ssx;
        sy = ssy;
        sw = ssw;
        sh = ssh;
        dx = ddx;
        dy = ddy;
        dw = abs(ddw);
        dh = abs(ddh);
        /* don't do anything if we have a 0 width or height image to render */
        /* if the input rect size < 0 don't render either */
        if ((dw <= 0) || (dh <= 0) || (sw <= 0) || (sh <= 0))
           return;
        /* clip the source rect to be within the actual image */
        psx = sx;
        psy = sy;
        psw = sw;
        psh = sh;
        CLIP(sx, sy, sw, sh, 0, 0, im_src->w, im_src->h);
        if (psx != sx)
           dx += ((sx - psx) * abs(ddw)) / ssw;
        if (psy != sy)
           dy += ((sy - psy) * abs(ddh)) / ssh;
        if (psw != sw)
           dw = (dw * sw) / psw;
        if (psh != sh)
           dh = (dh * sh) / psh;
        if ((dw <= 0) || (dh <= 0) || (sw <= 0) || (sh <= 0))
          {
             return;
          }
        /* clip output coords to clipped input coords */
        psx = dx;
        psy = dy;
        psw = dw;
        psh = dh;
        x2 = sx;
        y2 = sy;
        CLIP(dx, dy, dw, dh, 0, 0, im_dst->w, im_dst->h);
        if ((dw <= 0) || (dh <= 0) || (sw <= 0) || (sh <= 0))
           return;
        if (clw)
          {
             CLIP_TO(dx, dy, dw, dh, clx, cly, clw, clh);
             if ((dw < 1) || (dh < 1))
                return;
          }
        if (psx != dx)
           sx += ((dx - psx) * ssw) / abs(ddw);
        if (psy != dy)
           sy += ((dy - psy) * ssh) / abs(ddh);
        if (psw != dw)
           sw = (sw * dw) / psw;
        if (psh != dh)
           sh = (sh * dh) / psh;
        dxx = dx - psx;
        dyy = dy - psy;
        dxx += (x2 * abs(ddw)) / ssw;
        dyy += (y2 * abs(ddh)) / ssh;

        if ((dw > 0) && (sw == 0))
           sw = 1;
        if ((dh > 0) && (sh == 0))
           sh = 1;
        /* do a second check to see if we now have invalid coords */
        /* don't do anything if we have a 0 width or height image to render */
        /* if the input rect size < 0 don't render either */
        if ((dw <= 0) || (dh <= 0) || (sw <= 0) || (sh <= 0))
          {
             return;
          }
        scaleinfo = __drawable_CalcScaleInfo(im_src, ssw, ssh, ddw, ddh, aa);
        if (!scaleinfo)
           return;
        /* if we are scaling the image at all make a scaling buffer */
        /* allocate a buffer to render scaled RGBA data into */
        buf = malloc(dw * LINESIZE * sizeof(DATA32));
        if (!buf)
          {
             __drawable_FreeScaleInfo(scaleinfo);
             return;
          }
        /* setup h */
        h = dh;
        if (!IMAGE_HAS_ALPHA(im_dst))
           merge_alpha = 0;
        if (!IMAGE_HAS_ALPHA(im_src))
          {
             rgb_src = 1;
             if (merge_alpha)
                blend = 1;
          }
        /* scale in LINESIZE Y chunks and convert to depth */
#ifdef DO_MMX_ASM
        do_mmx = __drawable_get_cpuid() & CPUID_MMX;
#endif
        for (y = 0; y < dh; y += LINESIZE)
          {
             hh = LINESIZE;
             if (h < LINESIZE)
                hh = h;
             /* scale the imagedata for this LINESIZE lines chunk of image */
             if (aa)
               {
#ifdef DO_MMX_ASM
                  if (do_mmx)
                     __drawable_Scale_mmx_AARGBA(scaleinfo, buf, dxx, dyy + y,
                                              0, 0, dw, hh, dw, im_src->w);
                  else
#endif
                  if (IMAGE_HAS_ALPHA(im_src))
                     __drawable_ScaleAARGBA(scaleinfo, buf, dxx, dyy + y,
                                         0, 0, dw, hh, dw, im_src->w);
                  else
                     __drawable_ScaleAARGB(scaleinfo, buf, dxx, dyy + y,
                                        0, 0, dw, hh, dw, im_src->w);
               }
             else
                __drawable_ScaleSampleRGBA(scaleinfo, buf, dxx, dyy + y,
                                        0, 0, dw, hh, dw);
             __drawable_BlendRGBAToData(buf, dw, hh,
                                     im_dst->data, im_dst->w,
                                     im_dst->h,
                                     0, 0, dx, dy + y, dw, dh,
                                     blend, merge_alpha, cm, op, rgb_src);
             h -= LINESIZE;
          }
        /* free up our buffers and point tables */
        free(buf);
        __drawable_FreeScaleInfo(scaleinfo);
     }
}

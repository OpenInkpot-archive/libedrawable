#include "common.h"
#include "colormod.h"
#include "image.h"
#include "blend.h"
#include "span.h"

#define ADD_COPY(r, g, b, dest) \
do { \
                ADD_COLOR(R_VAL(dest), r, R_VAL(dest)); \
                ADD_COLOR(G_VAL(dest), g, G_VAL(dest)); \
                ADD_COLOR(B_VAL(dest), b, B_VAL(dest)); \
} while (0)

#define SUB_COPY(r, g, b, dest) \
do { \
                SUB_COLOR(R_VAL(dest), r, R_VAL(dest)); \
                SUB_COLOR(G_VAL(dest), g, G_VAL(dest)); \
                SUB_COLOR(B_VAL(dest), b, B_VAL(dest)); \
} while (0)

#define RE_COPY(r, g, b, dest) \
do { \
                RESHADE_COLOR(R_VAL(dest), r, R_VAL(dest)); \
                RESHADE_COLOR(G_VAL(dest), g, G_VAL(dest)); \
                RESHADE_COLOR(B_VAL(dest), b, B_VAL(dest)); \
} while (0)

#define MULT(na, a0, a1, tmp) \
do { \
   tmp = ((a0) * (a1)) + 0x80;   \
   na = (tmp + (tmp >> 8)) >> 8; \
} while (0)

extern DATA8        pow_lut[256][256];

/*   point drawing functions  */

/* COPY OPS */

static void
__drawable_CopyToRGBA(DATA32 color, DATA32 * dst)
{
   *dst = color;
}

static void
__drawable_CopyToRGB(DATA32 color, DATA32 * dst)
{
   *dst = (*dst & 0xff000000) | (color & 0x00ffffff);
}

static void
__drawable_BlendToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color), dst);
}

static void
__drawable_BlendToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;
   DATA8               a;

   a = pow_lut[A_VAL(&color)][A_VAL(dst)];
   BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
   BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
}

/* ADD OPS */

static void
__drawable_AddCopyToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   A_VAL(dst) = A_VAL(&color);
   ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
}

static void
__drawable_AddCopyToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
}

static void
__drawable_AddBlendToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color), dst);
}

static void
__drawable_AddBlendToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;
   DATA8               a;

   a = pow_lut[A_VAL(&color)][A_VAL(dst)];
   BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
   BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
}

/* SUBTRACT OPS */

static void
__drawable_SubCopyToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   A_VAL(dst) = A_VAL(&color);
   SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
}

static void
__drawable_SubCopyToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
}

static void
__drawable_SubBlendToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color), dst);
}

static void
__drawable_SubBlendToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;
   DATA8               a;

   a = pow_lut[A_VAL(&color)][A_VAL(dst)];
   BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
   BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
}

/* RESHADE OPS */

static void
__drawable_ReCopyToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   A_VAL(dst) = A_VAL(&color);
   RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
}

static void
__drawable_ReCopyToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
}

static void
__drawable_ReBlendToRGB(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;

   BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color), dst);
}

static void
__drawable_ReBlendToRGBA(DATA32 color, DATA32 * dst)
{
   DATA32              tmp;
   DATA8               a;

   a = pow_lut[A_VAL(&color)][A_VAL(dst)];
   BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
   BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
}

/*  span drawing functions*  */

/* COPY OPS */

static void
__drawable_CopySpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        *dst = color;
        dst++;
     }
}

static void
__drawable_CopySpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        *dst = (*dst & 0xff000000) | (color & 0x00ffffff);
        dst++;
     }
}

static void
__drawable_BlendSpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_BlendSpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        a = pow_lut[A_VAL(&color)][A_VAL(dst)];
        BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
        BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
        dst++;
     }
}

/* ADD OPS */

static void
__drawable_AddCopySpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        A_VAL(dst) = A_VAL(&color);
        ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_AddCopySpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_AddBlendSpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color),
                  dst);
        dst++;
     }
}

static void
__drawable_AddBlendSpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        a = pow_lut[A_VAL(&color)][A_VAL(dst)];
        BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
        BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
        dst++;
     }
}

/* SUBTRACT OPS */

static void
__drawable_SubCopySpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        A_VAL(dst) = A_VAL(&color);
        SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_SubCopySpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_SubBlendSpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color),
                  dst);
        dst++;
     }
}

static void
__drawable_SubBlendSpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        a = pow_lut[A_VAL(&color)][A_VAL(dst)];
        BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
        BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
        dst++;
     }
}

/* RESHADE OPS */

static void
__drawable_ReCopySpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        A_VAL(dst) = A_VAL(&color);
        RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_ReCopySpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
        dst++;
     }
}

static void
__drawable_ReBlendSpanToRGB(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color),
                 dst);
        dst++;
     }
}

static void
__drawable_ReBlendSpanToRGBA(DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        a = pow_lut[A_VAL(&color)][A_VAL(dst)];
        BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
        BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
        dst++;
     }
}

/*  shaped span drawing functions*  */

/* COPY OPS */

static void
__drawable_CopyShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   DATA32              col = color;

   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     *dst = color;
                     break;
                  }
               default:
                  {
                     MULT(A_VAL(&col), *src, A_VAL(&color), tmp);
                     *dst = col;
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                *dst = color;
                break;
             }
          default:
             {
                A_VAL(&col) = *src;
                *dst = col;
                break;
             }
          }
        src++;
        dst++;
     }
}

static void
__drawable_CopyShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        if (*src)
           *dst = (*dst & 0xff000000) | (color & 0x00ffffff);

        src++;
        dst++;
     }
}

static void
__drawable_BlendShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color),
                           A_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(a, *src, A_VAL(&color), tmp);
                     BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                *dst = (*dst & 0xff000000) | (color & 0x00ffffff);
                break;
             }
          default:
             {
                BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), *src, dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

static void
__drawable_BlendShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     a = pow_lut[A_VAL(&color)][A_VAL(dst)];
                     BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
                     BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                     break;
                  }
               default:
                  {
                     MULT(aa, *src, A_VAL(&color), tmp);
                     a = pow_lut[aa][A_VAL(dst)];
                     BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                     BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                *dst = color;
                break;
             }
          default:
             {
                a = pow_lut[*src][A_VAL(dst)];
                BLEND_COLOR(*src, A_VAL(dst), 255, A_VAL(dst));
                BLEND(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

/* ADD OPS */

static void
__drawable_AddCopyShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst,
                                int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     A_VAL(dst) = A_VAL(&color);
                     ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(A_VAL(dst), *src, A_VAL(&color), tmp);
                     ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        if (*src)
          {
             A_VAL(dst) = *src;
             ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
          }
        src++;
        dst++;
     }
}

static void
__drawable_AddCopyShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        if (*src)
          {
             ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
          }

        src++;
        dst++;
     }
}

static void
__drawable_AddBlendShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst,
                                int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color),
                               A_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(a, *src, A_VAL(&color), tmp);
                     BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                               dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                break;
             }
          default:
             {
                BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), *src,
                          dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

static void
__drawable_AddBlendShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst,
                                 int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     a = pow_lut[A_VAL(&color)][A_VAL(dst)];
                     BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
                     BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                               dst);
                     break;
                  }
               default:
                  {
                     MULT(aa, *src, A_VAL(&color), tmp);
                     a = pow_lut[aa][A_VAL(dst)];
                     BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                     BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                               dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                A_VAL(dst) = 255;
                ADD_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                break;
             }
          default:
             {
                a = pow_lut[*src][A_VAL(dst)];
                BLEND_COLOR(*src, A_VAL(dst), 255, A_VAL(dst));
                BLEND_ADD(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

/* SUBTRACT OPS */

static void
__drawable_SubCopyShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst,
                                int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     A_VAL(dst) = A_VAL(&color);
                     SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(A_VAL(dst), *src, A_VAL(&color), tmp);
                     SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        if (*src)
          {
             A_VAL(dst) = *src;
             SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
          }
        src++;
        dst++;
     }
}

static void
__drawable_SubCopyShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        if (*src)
          {
             SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
          }

        src++;
        dst++;
     }
}

static void
__drawable_SubBlendShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst,
                                int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color),
                               A_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(a, *src, A_VAL(&color), tmp);
                     BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                               dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                break;
             }
          default:
             {
                BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), *src,
                          dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

static void
__drawable_SubBlendShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst,
                                 int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     a = pow_lut[A_VAL(&color)][A_VAL(dst)];
                     BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
                     BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                               dst);
                     break;
                  }
               default:
                  {
                     MULT(aa, *src, A_VAL(&color), tmp);
                     a = pow_lut[aa][A_VAL(dst)];
                     BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                     BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                               dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                A_VAL(dst) = 255;
                SUB_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                break;
             }
          default:
             {
                a = pow_lut[*src][A_VAL(dst)];
                BLEND_COLOR(*src, A_VAL(dst), 255, A_VAL(dst));
                BLEND_SUB(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

/* RESHADE OPS */

static void
__drawable_ReCopyShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     A_VAL(dst) = A_VAL(&color);
                     RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(A_VAL(dst), *src, A_VAL(&color), tmp);
                     RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        if (*src)
          {
             A_VAL(dst) = *src;
             RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
          }
        src++;
        dst++;
     }
}

static void
__drawable_ReCopyShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   while (len--)
     {
        DATA32              tmp;

        if (*src)
          {
             RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
          }

        src++;
        dst++;
     }
}

static void
__drawable_ReBlendShapedSpanToRGB(DATA8 * src, DATA32 color, DATA32 * dst, int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color),
                              A_VAL(&color), dst);
                     break;
                  }
               default:
                  {
                     MULT(a, *src, A_VAL(&color), tmp);
                     BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                              dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                break;
             }
          default:
             {
                BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), *src,
                         dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

static void
__drawable_ReBlendShapedSpanToRGBA(DATA8 * src, DATA32 color, DATA32 * dst,
                                int len)
{
   if (A_VAL(&color) < 255)
     {
        while (len--)
          {
             DATA32              tmp;
             DATA8               a, aa;

             switch (*src)
               {
               case 0:
                  break;
               case 255:
                  {
                     a = pow_lut[A_VAL(&color)][A_VAL(dst)];
                     BLEND_COLOR(A_VAL(&color), A_VAL(dst), 255, A_VAL(dst));
                     BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                              dst);
                     break;
                  }
               default:
                  {
                     MULT(aa, *src, A_VAL(&color), tmp);
                     a = pow_lut[aa][A_VAL(dst)];
                     BLEND_COLOR(aa, A_VAL(dst), 255, A_VAL(dst));
                     BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), a,
                              dst);
                     break;
                  }
               }
             src++;
             dst++;
          }
        return;
     }

   while (len--)
     {
        DATA32              tmp;
        DATA8               a;

        switch (*src)
          {
          case 0:
             break;
          case 255:
             {
                A_VAL(dst) = 255;
                RE_COPY(R_VAL(&color), G_VAL(&color), B_VAL(&color), dst);
                break;
             }
          default:
             {
                a = pow_lut[*src][A_VAL(dst)];
                BLEND_COLOR(*src, A_VAL(dst), 255, A_VAL(dst));
                BLEND_RE(R_VAL(&color), G_VAL(&color), B_VAL(&color), a, dst);
                break;
             }
          }
        src++;
        dst++;
     }
}

DrawablePointDrawFunction
__drawable_GetPointDrawFunction(DrawableOp op, char dst_alpha, char blend)
{
   /* [ operation ][ dst_alpha ][ blend ]  */
   static DrawablePointDrawFunction ptfuncs[4][2][2] =
      /* OP_COPY */
   { {{__drawable_CopyToRGB, __drawable_BlendToRGB},
      {__drawable_CopyToRGBA, __drawable_BlendToRGBA}},
   /* OP_ADD */
   {{__drawable_AddCopyToRGB, __drawable_AddBlendToRGB},
    {__drawable_AddCopyToRGBA, __drawable_AddBlendToRGBA}},
   /* OP_SUBTRACT */
   {{__drawable_SubCopyToRGB, __drawable_SubBlendToRGB},
    {__drawable_SubCopyToRGBA, __drawable_SubBlendToRGBA}},
   /* OP_RESHADE */
   {{__drawable_ReCopyToRGB, __drawable_ReBlendToRGB},
    {__drawable_ReCopyToRGBA, __drawable_ReBlendToRGBA}},
   };

   int                 opi = (op == OP_COPY) ? 0
      : (op == OP_ADD) ? 1
      : (op == OP_SUBTRACT) ? 2 : (op == OP_RESHADE) ? 3 : -1;

   if (opi == -1)
      return NULL;

   return ptfuncs[opi][!!dst_alpha][!!blend];
}

DrawableSpanDrawFunction
__drawable_GetSpanDrawFunction(DrawableOp op, char dst_alpha, char blend)
{
   static DrawableSpanDrawFunction spanfuncs[4][2][2] =
      /* OP_COPY */
   { {{__drawable_CopySpanToRGB, __drawable_BlendSpanToRGB},
      {__drawable_CopySpanToRGBA, __drawable_BlendSpanToRGBA}},
   /* OP_ADD */
   {{__drawable_AddCopySpanToRGB, __drawable_AddBlendSpanToRGB},
    {__drawable_AddCopySpanToRGBA, __drawable_AddBlendSpanToRGBA}},
   /* OP_SUBTRACT */
   {{__drawable_SubCopySpanToRGB, __drawable_SubBlendSpanToRGB},
    {__drawable_SubCopySpanToRGBA, __drawable_SubBlendSpanToRGBA}},
   /* OP_RESHADE */
   {{__drawable_ReCopySpanToRGB, __drawable_ReBlendSpanToRGB},
    {__drawable_ReCopySpanToRGBA, __drawable_ReBlendSpanToRGBA}},
   };

   int                 opi = (op == OP_COPY) ? 0
      : (op == OP_ADD) ? 1
      : (op == OP_SUBTRACT) ? 2 : (op == OP_RESHADE) ? 3 : -1;

   if (opi == -1)
      return NULL;

   return spanfuncs[opi][!!dst_alpha][!!blend];
}

DrawableShapedSpanDrawFunction
__drawable_GetShapedSpanDrawFunction(DrawableOp op, char dst_alpha, char blend)
{
   static DrawableShapedSpanDrawFunction shapedspanfuncs[4][2][2] =
      /* OP_COPY */
   { {{__drawable_CopyShapedSpanToRGB, __drawable_BlendShapedSpanToRGB},
      {__drawable_CopyShapedSpanToRGBA, __drawable_BlendShapedSpanToRGBA}},
   /* OP_ADD */
   {{__drawable_AddCopyShapedSpanToRGB, __drawable_AddBlendShapedSpanToRGB},
    {__drawable_AddCopyShapedSpanToRGBA, __drawable_AddBlendShapedSpanToRGBA}},
   /* OP_SUBTRACT */
   {{__drawable_SubCopyShapedSpanToRGB, __drawable_SubBlendShapedSpanToRGB},
    {__drawable_SubCopyShapedSpanToRGBA, __drawable_SubBlendShapedSpanToRGBA}},
   /* OP_RESHADE */
   {{__drawable_ReCopyShapedSpanToRGB, __drawable_ReBlendShapedSpanToRGB},
    {__drawable_ReCopyShapedSpanToRGBA, __drawable_ReBlendShapedSpanToRGBA}},
   };

   int                 opi = (op == OP_COPY) ? 0
      : (op == OP_ADD) ? 1
      : (op == OP_SUBTRACT) ? 2 : (op == OP_RESHADE) ? 3 : -1;

   if (opi == -1)
      return NULL;

   return shapedspanfuncs[opi][!!dst_alpha][!!blend];
}

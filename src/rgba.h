#ifndef __RGBA
#define __RGBA 1

#ifdef BUILD_X11

#define DM_BS1 (8 + 3)
#define DM_BS2 (8)
#define DM_X (8)
#define DM_Y (8)

__hidden void    __drawable_RGBASetupContext(Context *ct);
__hidden void    __drawable_RGBA_init(void *rd, void *gd, void *bd, int depth, 
			  DATA8 palette_type);

typedef void (*DrawableRGBAFunction)(DATA32*, int, DATA8*,
				  int, int, int, int, int);
typedef void (*DrawableMaskFunction)(DATA32*, int, DATA8*,
				  int, int, int, int, int, int);
__hidden DrawableRGBAFunction
__drawable_GetRGBAFunction(int depth, 
			unsigned long rm, unsigned long gm, unsigned long bm, 
			char hiq, DATA8 palette_type);
__hidden DrawableMaskFunction
__drawable_GetMaskFunction(char hiq);

#ifdef DO_MMX_ASM
void __drawable_mmx_rgb555_fast(DATA32*, int, DATA8*, int, int, int, int, int);
void __drawable_mmx_bgr555_fast(DATA32*, int, DATA8*, int, int, int, int, int);
void __drawable_mmx_rgb565_fast(DATA32*, int, DATA8*, int, int, int, int, int);
void __drawable_mmx_bgr565_fast(DATA32*, int, DATA8*, int, int, int, int, int);
#endif

#endif

#endif

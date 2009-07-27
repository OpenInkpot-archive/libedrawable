lib_LTLIBRARIES = libedrawable.la
INCLUDES= $(all_includes) @FREETYPE_CFLAGS@ @ECORE_CFLAGS@ @ECORE_FILE_CFLAGS@ @EVAS_CFLAGS@ 
AM_CFLAGS = -Wall -Werror

libedrawable_la_SOURCES = edrawable.c api.c blend.c scale.c rgba.c rgbadraw.c \
	image.c polygon.c ellipse.c line.c updates.c rectangle.c color_helpers.c \
	rotate.c span.c font_draw.c font_load.c font_main.c font_query.c file.c
libedrawable_la_LIBS = @FREETYPE_LIBS@ @EVAS_LIBS@ @ECORE_FILE_LIBS@

include_HEADERS = \
    edrawable.h \
    eimlib.h 

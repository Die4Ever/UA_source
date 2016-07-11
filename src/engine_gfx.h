#ifndef ENGINE_GFX_H_INCLUDED
#define ENGINE_GFX_H_INCLUDED

#include <inttypes.h>
#include <ddraw.h>
#include "engine_tform.h"

extern stored_functions_engine gfx_engine_vtbl;

struct tile_xy
{
    int byteoff;
    int width;
};

struct texStru;

struct bitmap_intern
{
    void *buffer;
    __int16 width;
    __int16 height;
    int pitch;
    BYTE *pallete;
    char flags;
    char field_11;
    __int16 field_12;
    texStru *ddrawSurfTex;
    int field_18;
    int field_1C;
};

struct tiles_stru
{
    NC_STACK_nucleus *font_image; //FFFFFIX ME
    bitmap_intern *field_4;
    void  *field_8;
    tile_xy *chars;
    int font_height;
    int field_12;
    int field_14;
};


struct rstr_207_arg
{
    tiles_stru *tiles;
    int id;
};

struct w3d_a209
{
    char *cmdbuf;
    char **includ;
};

struct tUtV
{
    float tu;
    float tv;
};

struct polysDatSub
{
    int renderFlags;
    int vertexCount;
    xyz *vertexes;
    tUtV *tu_tv;
    float *color;
    bitmap_intern *pbitm;
    int field_18;
};


void sub_4231FC(void *dat);
void gfxEngine__getter(unsigned int a1, ...);
void gfxEngine__setter(unsigned int a1, ...);

tiles_stru * win3d_select_tileset(int id);
void sub_423288(w3d_a209 *arg);

struct wdd_font_st1
{
    const char *string;
    int p1;
    int p2;
    int p3;
    int p4;
    int flag;
};

struct wdd_font
{
    IDirectDraw *ddraw;
    IDirectDrawSurface *ddsurf;
    HFONT hFont;
    int height;
    HDC hDC;
    int strings_count;
    wdd_font_st1 field_18[64];
    int TextColor;
};

struct windd__window_params
{
    HWND hwnd;
    int width;
    int height;
};

void gfx_set_tileset(tiles_stru *a1, int id);
int win3d__load_palette_from_ilbm(const char *palette);

#endif // ENGINE_GFX_H_INCLUDED

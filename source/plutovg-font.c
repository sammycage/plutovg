#include "plutovg-private.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdio.h>

#define CACHE_SIZE 256

struct plutovg_font {
    int ref;
    unsigned char* data;
    int owndata;
    stbtt_fontinfo info;
    plutovg_glyph_t cache[CACHE_SIZE];
};

plutovg_font_t* plutovg_font_load_from_memory(unsigned char* data, int owndata)
{
    plutovg_font_t* font = malloc(sizeof(plutovg_font_t));
    font->ref = 1;
    font->data = data;
    font->owndata = owndata;
    memset(font->cache, 0, sizeof(font->cache));
    if(!stbtt_InitFont(&font->info, data, 0))
    {
        plutovg_font_destroy(font);
        return NULL;
    }

    return font;
}

plutovg_font_t* plutovg_font_load_from_file(const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if(fp==NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);
    size_t size = (size_t)ftell(fp);
    unsigned char* data = malloc(size);
    fseek(fp, 0, SEEK_SET);

    fread(data, 1, size, fp);
    fclose(fp);

    return plutovg_font_load_from_memory(data, 1);
}

plutovg_font_t* plutovg_font_reference(plutovg_font_t* font)
{
    if(font)
        ++font->ref;
    return font;
}

void plutovg_font_destroy(plutovg_font_t* font)
{
    if(font==NULL)
        return;

    if(--font->ref==0)
    {
        for(int i = 0;i < CACHE_SIZE;i++)
        {
            plutovg_glyph_t* glyph = &font->cache[i];
            plutovg_path_destroy(glyph->path);
        }

        if(font->owndata)
            free(font->data);
        free(font);
    }
}

int plutovg_font_get_reference_count(const plutovg_font_t* font)
{
    return font->ref;
}

double plutovg_font_get_scale(const plutovg_font_t* font)
{
    return (double)stbtt_ScaleForMappingEmToPixels(&font->info, 1.f);
}

const plutovg_glyph_t* plutovg_font_get_glyph(plutovg_font_t* font, int codepoint)
{
    plutovg_glyph_t* glyph = &font->cache[codepoint%CACHE_SIZE];
    if(glyph->codepoint==codepoint)
        return glyph;

    glyph->codepoint = codepoint;
    if(glyph->path==NULL)
        glyph->path = plutovg_path_create();
    else
        plutovg_path_clear(glyph->path);

    int index = stbtt_FindGlyphIndex(&font->info, codepoint);
    stbtt_vertex* v;
    int num_vertices = stbtt_GetGlyphShape(&font->info, index, &v);
    for(int i = 0;i < num_vertices;i++)
    {
        switch(v[i].type)
        {
        case STBTT_vmove:
            plutovg_path_move_to(glyph->path, v[i].x, v[i].y);
            break;
        case STBTT_vline:
            plutovg_path_line_to(glyph->path, v[i].x, v[i].y);
            break;
        case STBTT_vcurve:
            plutovg_path_quad_to(glyph->path, v[i].cx, v[i].cy, v[i].x, v[i].y);
            break;
        case STBTT_vcubic:
            plutovg_path_cubic_to(glyph->path, v[i].cx, v[i].cy, v[i].cx1, v[i].cy1, v[i].x, v[i].y);
            break;
        }
    }

    stbtt_FreeShape(&font->info, v);

    int advance;
    stbtt_GetGlyphHMetrics(&font->info, index, &advance, NULL);
    glyph->advance = advance;
    return glyph;
}

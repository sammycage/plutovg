#include "plutovg-private.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static int plutovg_text_iterator_length(const void* data, int length, plutovg_text_encoding_t encoding)
{
    if(length >= 0)
        return length;
    length = 0;
    switch(encoding) {
    case PLUTOVG_TEXT_ENCODING_UTF8:
    case PLUTOVG_TEXT_ENCODING_LATIN1: {
        const uint8_t* text = data;
        while(*text++)
            length++;
        break;
    } case PLUTOVG_TEXT_ENCODING_UTF16: {
        const uint16_t* text = data;
        while(*text++)
            length++;
        break;
    } case PLUTOVG_TEXT_ENCODING_UTF32: {
        const uint32_t* text = data;
        while(*text++)
            length++;
        break;
    } default:
        assert(false);
    }

    return length;
}

void plutovg_text_iterator_init(plutovg_text_iterator_t* it, const void* text, int length, plutovg_text_encoding_t encoding)
{
    it->text = text;
    it->length = plutovg_text_iterator_length(text, length, encoding);
    it->encoding = encoding;
    it->index = 0;
}

bool plutovg_text_iterator_has_next(const plutovg_text_iterator_t* it)
{
    return it->index < it->length;
}

int plutovg_text_iterator_next(plutovg_text_iterator_t* it)
{
    uint32_t codepoint = 0;
    switch(it->encoding) {
    case PLUTOVG_TEXT_ENCODING_UTF8: {
        static const int trailing[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
        };

        static const uint32_t offsets[6] = {
            0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080
        };

        const uint8_t* text = it->text;
        int trailing_bytes = trailing[text[it->index]];
        if(it->index + trailing_bytes >= it->length)
            trailing_bytes = 0;
        switch(trailing_bytes) {
        case 5: codepoint += text[it->index++]; codepoint <<= 6;
        case 4: codepoint += text[it->index++]; codepoint <<= 6;
        case 3: codepoint += text[it->index++]; codepoint <<= 6;
        case 2: codepoint += text[it->index++]; codepoint <<= 6;
        case 1: codepoint += text[it->index++]; codepoint <<= 6;
        case 0: codepoint += text[it->index++];
        }

        codepoint -= offsets[trailing_bytes];
        break;
    } case PLUTOVG_TEXT_ENCODING_UTF16: {
        const uint16_t* text = it->text;
        codepoint = text[it->index++];
        if(((codepoint) & 0xfffffc00) == 0xd800) {
            if(it->index < it->length && (((codepoint) & 0xfffffc00) == 0xdc00)) {
                uint16_t trail = text[it->index];
                codepoint = (codepoint << 10) + trail - ((0xD800u << 10) - 0x10000u + 0xDC00u);
                it->index++;
            }
        }

        break;
    } case PLUTOVG_TEXT_ENCODING_UTF32: {
        const uint32_t* text = it->text;
        codepoint = text[it->index];
        it->index += 1;
        break;
    } case PLUTOVG_TEXT_ENCODING_LATIN1: {
        const uint8_t* text = it->text;
        codepoint = text[it->index];
        it->index += 1;
        break;
    } default:
        assert(false);
    }

    return codepoint;
}

typedef struct {
    stbtt_vertex* vertices;
    int nvertices;
    int index;
    int advance_width;
    int left_side_bearing;
    int x1;
    int y1;
    int x2;
    int y2;
} glyph_t;

#define GLYPH_CACHE_SIZE 256
struct plutovg_font_face {
    int ref_count;
    int ascent;
    int descent;
    int linegap;
    int x1;
    int y1;
    int x2;
    int y2;
    stbtt_fontinfo info;
    glyph_t** glyphs[GLYPH_CACHE_SIZE];
    plutovg_destroy_func_t destroy_func;
    void* closure;
};

plutovg_font_face_t* plutovg_font_face_load_from_file(const char* filename, int ttcindex)
{
    FILE* fp = fopen(filename, "rb");
    if(fp == NULL)
        return NULL;
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    void* data = malloc(length);
    fseek(fp, 0, SEEK_SET);
    fread(data, 1, length, fp);
    fclose(fp);
    return plutovg_font_face_load_from_data(data, length, ttcindex, free, data);
}

plutovg_font_face_t* plutovg_font_face_load_from_data(const void* data, unsigned int length, int ttcindex, plutovg_destroy_func_t destroy_func, void* closure)
{
    stbtt_fontinfo info;
    if(!stbtt_InitFont(&info, data, ttcindex)) {
        if(destroy_func)
            destroy_func(closure);
        return NULL;
    }

    plutovg_font_face_t* face = malloc(sizeof(plutovg_font_face_t));
    face->ref_count = 1;
    face->info = info;
    stbtt_GetFontVMetrics(&face->info, &face->ascent, &face->descent, &face->linegap);
    stbtt_GetFontBoundingBox(&face->info, &face->x1, &face->y1, &face->x2, &face->y2);
    memset(face->glyphs, 0, sizeof(face->glyphs));
    face->destroy_func = destroy_func;
    face->closure = closure;
    return face;
}

plutovg_font_face_t* plutovg_font_face_reference(plutovg_font_face_t* face)
{
    if(face == NULL)
        return NULL;
    ++face->ref_count;
    return face;
}

void plutovg_font_face_destroy(plutovg_font_face_t* face)
{
    if(face == NULL)
        return;
    if(--face->ref_count == 0) {
        for(int i = 0; i < GLYPH_CACHE_SIZE; i++) {
            if(face->glyphs[i] == NULL)
                continue;
            for(int j = 0; j < GLYPH_CACHE_SIZE; j++) {
                glyph_t* glyph = face->glyphs[i][j];
                if(glyph == NULL)
                    continue;
                stbtt_FreeShape(&face->info, glyph->vertices);
                free(glyph);
            }

            free(face->glyphs[i]);
        }

        if(face->destroy_func)
            face->destroy_func(face->closure);
        free(face);
    }
}

int plutovg_font_face_get_reference_count(const plutovg_font_face_t* face)
{
    if(face)
        return face->ref_count;
    return 0;
}

float plutovg_font_face_get_scale(const plutovg_font_face_t* face, float size)
{
    return stbtt_ScaleForMappingEmToPixels(&face->info, size);
}

float plutovg_font_face_get_kerning(const plutovg_font_face_t* face, int ch1, int ch2)
{
    return stbtt_GetCodepointKernAdvance(&face->info, ch1, ch2);
}

void plutovg_font_face_get_extents(const plutovg_font_face_t* face, plutovg_rect_t* extents)
{
    extents->x = face->x1;
    extents->y = face->y1;
    extents->w = face->x2 - face->x1;
    extents->h = face->y2 - face->y1;
}

static glyph_t* get_glyph(const plutovg_font_face_t* face, int codepoint)
{
    unsigned int msb = (codepoint >> 8) & 0xFF;
    if(face->glyphs[msb] == NULL) {
        ((plutovg_font_face_t*)face)->glyphs[msb] = calloc(GLYPH_CACHE_SIZE, sizeof(glyph_t*));
    }

    unsigned int lsb = codepoint & 0xFF;
    if(face->glyphs[msb][lsb]) {
        return face->glyphs[msb][lsb];
    }

    glyph_t* glyph = malloc(sizeof(glyph_t));
    glyph->index = stbtt_FindGlyphIndex(&face->info, codepoint);
    glyph->nvertices = stbtt_GetGlyphShape(&face->info, glyph->index, &glyph->vertices);
    stbtt_GetGlyphHMetrics(&face->info, glyph->index, &glyph->advance_width, &glyph->left_side_bearing);
    if(!stbtt_GetGlyphBox(&face->info, glyph->index, &glyph->x1, &glyph->y1, &glyph->x2, &glyph->y2))
        glyph->x1 = glyph->y1 = glyph->x2 = glyph->y2 = 0.f;
    return (face->glyphs[msb][lsb] = glyph);
}

static void glyph_traverse_func(void* closure, plutovg_path_command_t command, const plutovg_point_t* points, int npoints)
{
    plutovg_path_t* path = (plutovg_path_t*)(closure);
    switch(command) {
    case PLUTOVG_PATH_COMMAND_MOVE_TO:
        plutovg_path_move_to(path, points[0].x, points[0].y);
        break;
    case PLUTOVG_PATH_COMMAND_LINE_TO:
        plutovg_path_line_to(path, points[0].x, points[0].y);
        break;
    case PLUTOVG_PATH_COMMAND_CUBIC_TO:
        plutovg_path_cubic_to(path, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
        break;
    case PLUTOVG_PATH_COMMAND_CLOSE:
        assert(false);
    }
}

void plutovg_font_face_get_glyph_path(const plutovg_font_face_t* face, int codepoint, plutovg_path_t* path)
{
    return plutovg_font_face_traverse_glyph_path(face, codepoint, glyph_traverse_func, path);
}

void plutovg_font_face_traverse_glyph_path(const plutovg_font_face_t* face, int codepoint, plutovg_path_traverse_func_t traverse_func, void* closure)
{
    plutovg_point_t points[3];
    plutovg_point_t current_point = {0, 0};
    glyph_t* glyph = get_glyph(face, codepoint);
    for(int i = 0; i < glyph->nvertices; i++) {
        switch(glyph->vertices[i].type) {
        case STBTT_vmove:
            points[0].x = glyph->vertices[i].x;
            points[0].y = glyph->vertices[i].y;
            traverse_func(closure, PLUTOVG_PATH_COMMAND_MOVE_TO, points, 1);
            current_point = points[0];
            break;
        case STBTT_vline:
            points[0].x = glyph->vertices[i].x;
            points[0].y = glyph->vertices[i].y;
            traverse_func(closure, PLUTOVG_PATH_COMMAND_LINE_TO, points, 1);
            current_point = points[0];
            break;
        case STBTT_vcurve:
            points[0].x = 2.f / 3.f * glyph->vertices[i].cx + 1.f / 3.f * current_point.x;
            points[0].y = 2.f / 3.f * glyph->vertices[i].cy + 1.f / 3.f * current_point.y;
            points[1].x = 2.f / 3.f * glyph->vertices[i].cx + 1.f / 3.f * glyph->vertices[i].x;
            points[1].y = 2.f / 3.f * glyph->vertices[i].cy + 1.f / 3.f * glyph->vertices[i].y;
            points[2].x = glyph->vertices[i].x;
            points[2].y = glyph->vertices[i].y;
            traverse_func(closure, PLUTOVG_PATH_COMMAND_CUBIC_TO, points, 3);
            current_point = points[2];
            break;
        case STBTT_vcubic:
            points[0].x = glyph->vertices[i].cx;
            points[0].y = glyph->vertices[i].cy;
            points[1].x = glyph->vertices[i].cx1;
            points[1].y = glyph->vertices[i].cy1;
            points[2].x = glyph->vertices[i].x;
            points[2].y = glyph->vertices[i].y;
            traverse_func(closure, PLUTOVG_PATH_COMMAND_CUBIC_TO, points, 3);
            current_point = points[2];
            break;
        default:
            assert(false);
        }
    }
}

float plutovg_font_face_get_glyph_extents(const plutovg_font_face_t* face, int codepoint, plutovg_rect_t* extents)
{
    glyph_t* glyph = get_glyph(face, codepoint);
    if(extents) {
        extents->x = glyph->x1;
        extents->y = glyph->y1;
        extents->w = glyph->x2 - glyph->x1;
        extents->h = glyph->y2 - glyph->y1;
    }

    return glyph->advance_width;
}

float plutovg_font_face_get_glyph_advance_width(const plutovg_font_face_t* face, int codepoint)
{
    return get_glyph(face, codepoint)->advance_width;
}

float plutovg_font_face_get_glyph_left_side_bearing(const plutovg_font_face_t* face, int codepoint)
{
    return get_glyph(face, codepoint)->left_side_bearing;
}

float plutovg_font_face_text_extents(const plutovg_font_face_t* face, float size, const void* text, int length, plutovg_text_encoding_t encoding, plutovg_rect_t* extents)
{
    const float scale = plutovg_font_face_get_scale(face, size);
    plutovg_text_iterator_t it;
    plutovg_text_iterator_init(&it, text, length, encoding);
    plutovg_rect_t* text_extents = NULL;
    float advance_width = 0.f;
    while(plutovg_text_iterator_has_next(&it)) {
        int codepoint = plutovg_text_iterator_next(&it);
        if(extents == NULL) {
            advance_width += plutovg_font_face_get_glyph_advance_width(face, codepoint) * scale;
            continue;
        }

        plutovg_matrix_t matrix;
        plutovg_matrix_init_translate(&matrix, advance_width, 0.f);
        plutovg_matrix_scale(&matrix, scale, -scale);

        plutovg_rect_t glyph_extents;
        advance_width += plutovg_font_face_get_glyph_extents(face, codepoint, &glyph_extents) * scale;
        plutovg_matrix_map_rect(&matrix, &glyph_extents, &glyph_extents);

        if(text_extents == NULL) {
            text_extents = extents;
            *text_extents = glyph_extents;
            continue;
        }

        float x1 = plutovg_min(text_extents->x, glyph_extents.x);
        float y1 = plutovg_min(text_extents->y, glyph_extents.y);
        float x2 = plutovg_max(text_extents->x + extents->w, glyph_extents.x + glyph_extents.w);
        float y2 = plutovg_max(text_extents->y + extents->y, glyph_extents.y + glyph_extents.h);

        text_extents->x = x1;
        text_extents->y = y1;
        text_extents->w = x2 - x1;
        text_extents->h = y2 - y1;
    }

    if(extents && !text_extents) {
        extents->x = 0;
        extents->y = 0;
        extents->w = 0;
        extents->h = 0;
    }

    return advance_width;
}

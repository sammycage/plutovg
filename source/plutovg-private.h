#ifndef PLUTOVG_PRIVATE_H
#define PLUTOVG_PRIVATE_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "plutovg.h"

struct plutovg_surface {
    int ref_count;
    int width;
    int height;
    int stride;
    uint8_t* data;
};

struct plutovg_path {
    int ref_count;
    int num_curves;
    int num_contours;
    int num_points;
    plutovg_point_t start_point;
    struct {
        plutovg_path_element_t* data;
        int size;
        int capacity;
    } elements;
};

typedef enum {
    PLUTOVG_PAINT_TYPE_COLOR,
    PLUTOVG_PAINT_TYPE_GRADIENT,
    PLUTOVG_PAINT_TYPE_TEXTURE
} plutovg_paint_type_t;

struct plutovg_paint {
    int ref_count;
    plutovg_paint_type_t type;
};

typedef struct {
    plutovg_paint_t base;
    plutovg_color_t color;
} plutovg_solid_paint_t;

typedef enum {
    PLUTOVG_GRADIENT_TYPE_LINEAR,
    PLUTOVG_GRADIENT_TYPE_RADIAL
} plutovg_gradient_type_t;

typedef struct {
    plutovg_paint_t base;
    plutovg_gradient_type_t type;
    plutovg_spread_method_t spread;
    plutovg_matrix_t matrix;
    float values[6];
    plutovg_gradient_stop_t embedded_stops[2];
    plutovg_gradient_stop_t* stops;
    int nstops;
} plutovg_gradient_paint_t;

typedef struct {
    plutovg_paint_t base;
    plutovg_texture_type_t type;
    float opacity;
    plutovg_matrix_t matrix;
    plutovg_surface_t* surface;
} plutovg_texture_paint_t;

typedef struct {
    int x;
    int len;
    int y;
    unsigned char coverage;
} plutovg_span_t;

typedef struct {
    struct {
        plutovg_span_t* data;
        int size;
        int capacity;
    } spans;

    int x;
    int y;
    int w;
    int h;
} plutovg_span_buffer_t;

typedef struct {
    float offset;
    struct {
        float* data;
        int size;
        int capacity;
    } array;
} plutovg_stroke_dash_t;

typedef struct {
    float width;
    plutovg_line_cap_t cap;
    plutovg_line_join_t join;
    float miter_limit;
} plutovg_stroke_style_t;

typedef struct {
    plutovg_stroke_style_t style;
    plutovg_stroke_dash_t dash;
} plutovg_stroke_data_t;

typedef struct plutovg_state {
    plutovg_paint_t* paint;
    plutovg_matrix_t matrix;
    plutovg_stroke_data_t stroke;
    plutovg_operator_t op;
    plutovg_fill_rule_t winding;
    plutovg_span_buffer_t clip_spans;
    plutovg_font_face_t* font_face;
    float font_size;
    float opacity;
    bool clipping;
    struct plutovg_state* next;
} plutovg_state_t;

struct plutovg_canvas {
    int ref_count;
    plutovg_surface_t* surface;
    plutovg_state_t* state;
    plutovg_state_t* freed_state;
    plutovg_rect_t clip_rect;
    plutovg_span_buffer_t clip_spans;
    plutovg_span_buffer_t fill_spans;
    plutovg_path_t* path;
};

void plutovg_span_buffer_init(plutovg_span_buffer_t* span_buffer);
void plutovg_span_buffer_init_rect(plutovg_span_buffer_t* span_buffer, int x, int y, int width, int height);
void plutovg_span_buffer_reset(plutovg_span_buffer_t* span_buffer);
void plutovg_span_buffer_destroy(plutovg_span_buffer_t* span_buffer);
void plutovg_span_buffer_copy(plutovg_span_buffer_t* span_buffer, const plutovg_span_buffer_t* source);
void plutovg_span_buffer_extents(plutovg_span_buffer_t* span_buffer, plutovg_rect_t* extents);
void plutovg_span_buffer_intersect(plutovg_span_buffer_t* span_buffer, const plutovg_span_buffer_t* a, const plutovg_span_buffer_t* b);

void plutovg_rasterize(plutovg_span_buffer_t* span_buffer, const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_rect_t* clip_rect, const plutovg_stroke_data_t* stroke_data, plutovg_fill_rule_t winding);
void plutovg_blend(plutovg_canvas_t* canvas, const plutovg_span_buffer_t* span_buffer);

#define PLUTOVG_SQRT2 1.41421356237309504880f
#define PLUTOVG_PI 3.14159265358979323846f
#define PLUTOVG_TWO_PI 6.28318530717958647693f
#define PLUTOVG_HALF_PI 1.57079632679489661923f
#define PLUTOVG_KAPPA 0.55228474983079339840f

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define PLUTOVG_THREAD_LOCAL _Thread_local
#elif defined(_MSC_VER)
#define PLUTOVG_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#define PLUTOVG_THREAD_LOCAL __thread
#else
#define PLUTOVG_THREAD_LOCAL
#endif

#define PLUTOVG_IS_NUM(c) ((c) >= '0' && (c) <= '9')
#define PLUTOVG_IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define PLUTOVG_IS_ALNUM(c) (PLUTOVG_IS_ALPHA(c) || PLUTOVG_IS_NUM(c))
#define PLUTOVG_IS_WS(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

#define plutovg_min(a, b) ((a) < (b) ? (a) : (b))
#define plutovg_max(a, b) ((a) > (b) ? (a) : (b))
#define plutovg_clamp(v, lo, hi) ((v) < (lo) ? (lo) : (hi) < (v) ? (hi) : (v))
#define plutovg_div255(x) (((x) + ((x) >> 8) + 0x80) >> 8)

#define plutovg_alpha(c) (((c) >> 24) & 0xff)
#define plutovg_red(c) (((c) >> 16) & 0xff)
#define plutovg_green(c) (((c) >> 8) & 0xff)
#define plutovg_blue(c) (((c) >> 0) & 0xff)

#define PLUTOVG_DEFAULT_STROKE_STYLE ((plutovg_stroke_style_t){1, PLUTOVG_LINE_CAP_BUTT, PLUTOVG_LINE_JOIN_MITER, 10})
#define PLUTOVG_IDENTITY_MATRIX ((plutovg_matrix_t){1, 0, 0, 1, 0, 0})
#define PLUTOVG_DEFAULT_COLOR ((plutovg_color_t){0, 0, 0, 1})

#define plutovg_array_init(array) \
    do { \
        (array).data = NULL; \
        (array).size = 0; \
        (array).capacity = 0; \
    } while(0)

#define plutovg_array_ensure(array, count) \
    do { \
        if((array).data == NULL || ((array).size + (count) > (array).capacity)) { \
            int capacity = (array).size + (count); \
            int newcapacity = (array).capacity == 0 ? 8 : (array).capacity; \
            while(newcapacity < capacity) { newcapacity *= 2; } \
            (array).data = realloc((array).data, newcapacity * sizeof((array).data[0])); \
            (array).capacity = newcapacity; \
        } \
    } while(0)

#define plutovg_array_append_data(array, newdata, count) \
    do { \
        plutovg_array_ensure(array, count); \
        memcpy((array).data + (array).size, newdata, (count) * sizeof((newdata)[0])); \
        (array).size += count; \
    } while(0)

#define plutovg_array_append(array, other) plutovg_array_append_data(array, (other).data, (other).size)
#define plutovg_array_clear(array) ((array).size = 0)
#define plutovg_array_destroy(array) free((array).data)

#endif // PLUTOVG_PRIVATE_H

#ifndef PLUTOVG_PRIVATE_H
#define PLUTOVG_PRIVATE_H

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "plutovg.h"

struct plutovg_surface {
    int ref_count;
    int width;
    int height;
    int stride;
    unsigned char* data;
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
    PLUTOVG_GRADIENT_TYPE_LINEAR,
    PLUTOVG_GRADIENT_TYPE_RADIAL
} plutovg_gradient_type_t;

typedef struct plutovg_gradient {
    plutovg_gradient_type_t type;
    plutovg_spread_method_t spread;
    plutovg_matrix_t matrix;
    float values[6];
    struct {
        plutovg_gradient_stop_t* data;
        int size;
        int capacity;
    } stops;
} plutovg_gradient_t;

typedef struct plutovg_texture {
    plutovg_texture_type_t type;
    float opacity;
    plutovg_matrix_t matrix;
    plutovg_surface_t* surface;
} plutovg_texture_t;

typedef enum {
    PLUTOVG_PAINT_TYPE_COLOR,
    PLUTOVG_PAINT_TYPE_GRADIENT,
    PLUTOVG_PAINT_TYPE_TEXTURE
} plutovg_paint_type_t;

struct plutovg_paint {
    int ref_count;
    plutovg_paint_type_t type;
    union {
        plutovg_color_t color;
        plutovg_gradient_t gradient;
        plutovg_texture_t texture;
    } data;
};

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
} plutovg_rle_t;

typedef struct {
    float line_width;
    float miter_limit;
    plutovg_line_cap_t line_cap;
    plutovg_line_join_t line_join;
    float dash_offset;
    struct {
        float* data;
        int size;
        int capacity;
    } dash_array;
} plutovg_stroke_data_t;

typedef struct plutovg_state {
    plutovg_paint_t* paint;
    plutovg_color_t color;
    plutovg_matrix_t matrix;
    plutovg_stroke_data_t stroke;
    plutovg_operator_t op;
    plutovg_fill_rule_t winding;
    plutovg_rle_t clip_rle;
    bool clipping;
    float opacity;
    struct plutovg_state* next;
} plutovg_state_t;

struct plutovg_canvas {
    int ref_count;
    plutovg_surface_t* surface;
    plutovg_state_t* state;
    plutovg_state_t* freed_state;
    plutovg_rect_t clip_rect;
    plutovg_rle_t clip_rle;
    plutovg_rle_t rle;
    plutovg_path_t* path;
};

void plutovg_rle_init(plutovg_rle_t* rle);
void plutovg_rle_reset(plutovg_rle_t* rle);
void plutovg_rle_finish(plutovg_rle_t* rle);
void plutovg_rle_copy(plutovg_rle_t* rle, const plutovg_rle_t* source);
void plutovg_rle_extents(plutovg_rle_t* rle, plutovg_rect_t* extents);
void plutovg_rle_add_rect(plutovg_rle_t* rle, int x, int y, int width, int height);
void plutovg_rle_intersect(plutovg_rle_t* rle, const plutovg_rle_t* a, const plutovg_rle_t* b);
void plutovg_rasterize(plutovg_rle_t* rle, const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_rect_t* clip_rect, const plutovg_stroke_data_t* stroke_data, plutovg_fill_rule_t winding);

void plutovg_blend(plutovg_canvas_t* canvas, const plutovg_rle_t* rle);
void plutovg_blend_color(plutovg_canvas_t* canvas, const plutovg_rle_t* rle, const plutovg_color_t* color);
void plutovg_blend_gradient(plutovg_canvas_t* canvas, const plutovg_rle_t* rle, const plutovg_gradient_t* gradient);
void plutovg_blend_texture(plutovg_canvas_t* canvas, const plutovg_rle_t* rle, const plutovg_texture_t* texture);

#define PLUTOVG_SQRT2 1.41421356237309504880
#define PLUTOVG_PI 3.14159265358979323846
#define PLUTOVG_TWO_PI 6.28318530717958647693
#define PLUTOVG_HALF_PI 1.57079632679489661923
#define PLUTOVG_KAPPA 0.55228474983079339840

#define plutovg_min(a, b) ((a) < (b) ? (a) : (b))
#define plutovg_max(a, b) ((a) > (b) ? (a) : (b))
#define plutovg_clamp(v, lo, hi) ((v) < (lo) ? (lo) : (hi) < (v) ? (hi) : (v))
#define plutovg_div255(x) (((x) + ((x) >> 8) + 0x80) >> 8)

#define plutovg_alpha(c) (((c) >> 24) & 0xff)
#define plutovg_red(c) (((c) >> 16) & 0xff)
#define plutovg_green(c) (((c) >> 8) & 0xff)
#define plutovg_blue(c) (((c) >> 0) & 0xff)

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

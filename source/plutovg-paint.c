#include "plutovg-private.h"

plutovg_paint_t* plutovg_paint_create_rgb(float r, float g, float b)
{
    return plutovg_paint_create_rgba(r, g, b, 1.f);
}

plutovg_paint_t* plutovg_paint_create_rgba(float r, float g, float b, float a)
{
    plutovg_paint_t* paint = malloc(sizeof(plutovg_paint_t));
    paint->ref_count = 1;
    paint->type = PLUTOVG_PAINT_TYPE_COLOR;
    paint->data.color.r = r;
    paint->data.color.g = g;
    paint->data.color.b = b;
    paint->data.color.a = a;
    return paint;
}

plutovg_paint_t* plutovg_paint_create_color(const plutovg_color_t* color)
{
    return plutovg_paint_create_rgba(color->r, color->g, color->b, color->a);
}

plutovg_paint_t* plutovg_paint_create_linear_gradient(float x1, float y1, float x2, float y2, plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix)
{
    plutovg_paint_t* paint = malloc(sizeof(plutovg_paint_t));
    paint->ref_count = 1;
    paint->type = PLUTOVG_PAINT_TYPE_GRADIENT;
    paint->data.gradient.type = PLUTOVG_GRADIENT_TYPE_LINEAR;
    paint->data.gradient.spread = spread;
    paint->data.gradient.matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
    paint->data.gradient.values[0] = x1;
    paint->data.gradient.values[1] = y1;
    paint->data.gradient.values[2] = x2;
    paint->data.gradient.values[3] = y2;
    plutovg_array_init(paint->data.gradient.stops);
    plutovg_array_append_data(paint->data.gradient.stops, stops, nstops);
    return paint;
}

plutovg_paint_t* plutovg_paint_create_radial_gradient(float cx, float cy, float cr, float fx, float fy, float fr, plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix)
{
    plutovg_paint_t* paint = malloc(sizeof(plutovg_paint_t));
    paint->ref_count = 1;
    paint->type = PLUTOVG_PAINT_TYPE_GRADIENT;
    paint->data.gradient.type = PLUTOVG_GRADIENT_TYPE_RADIAL;
    paint->data.gradient.spread = spread;
    paint->data.gradient.matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
    paint->data.gradient.values[0] = cx;
    paint->data.gradient.values[1] = cy;
    paint->data.gradient.values[2] = cr;
    paint->data.gradient.values[3] = fx;
    paint->data.gradient.values[4] = fy;
    paint->data.gradient.values[5] = fr;
    plutovg_array_init(paint->data.gradient.stops);
    plutovg_array_append_data(paint->data.gradient.stops, stops, nstops);
    return paint;
}

plutovg_paint_t* plutovg_paint_create_texture(plutovg_surface_t* surface, plutovg_texture_type_t type, float opacity, const plutovg_matrix_t* matrix)
{
    plutovg_paint_t* paint = malloc(sizeof(plutovg_paint_t));
    paint->ref_count = 1;
    paint->type = PLUTOVG_PAINT_TYPE_TEXTURE;
    paint->data.texture.type = type;
    paint->data.texture.opacity = opacity;
    paint->data.texture.matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
    paint->data.texture.surface = plutovg_surface_reference(surface);
    return paint;
}

plutovg_paint_t* plutovg_paint_reference(plutovg_paint_t* paint)
{
    if(paint == NULL)
        return NULL;
    ++paint->ref_count;
    return paint;
}

void plutovg_paint_destroy(plutovg_paint_t* paint)
{
    if(paint == NULL)
        return;
    if(--paint->ref_count == 0) {
        if(paint->type == PLUTOVG_PAINT_TYPE_GRADIENT)
            plutovg_array_destroy(paint->data.gradient.stops);
        if(paint->type == PLUTOVG_PAINT_TYPE_TEXTURE)
            plutovg_surface_destroy(paint->data.texture.surface);
        free(paint);
    }
}

int plutovg_paint_get_reference_count(const plutovg_paint_t* paint)
{
    if(paint)
        return paint->ref_count;
    return 0;
}

#include "plutovg-private.h"

static PLUTOVG_THREAD_LOCAL plutovg_paint_t* freed_paint[3];

static void* plutovg_paint_create(plutovg_paint_type_t type, size_t size)
{
    plutovg_paint_t* paint = freed_paint[type];
    if(paint == NULL) {
        paint = malloc(size);
    }

    freed_paint[type] = NULL;
    paint->ref_count = 1;
    paint->type = type;
    return paint;
}

plutovg_paint_t* plutovg_paint_create_rgb(float r, float g, float b)
{
    return plutovg_paint_create_rgba(r, g, b, 1.f);
}

plutovg_paint_t* plutovg_paint_create_rgba(float r, float g, float b, float a)
{
    plutovg_solid_paint_t* solid = plutovg_paint_create(PLUTOVG_PAINT_TYPE_COLOR, sizeof(plutovg_solid_paint_t));
    solid->color.r = r;
    solid->color.g = g;
    solid->color.b = b;
    solid->color.a = a;
    return &solid->base;
}

plutovg_paint_t* plutovg_paint_create_color(const plutovg_color_t* color)
{
    return plutovg_paint_create_rgba(color->r, color->g, color->b, color->a);
}

static void plutovg_gradient_stops_init(plutovg_gradient_paint_t* gradient, const plutovg_gradient_stop_t* stops, int nstops)
{
    if(nstops > 0) {
        gradient->stops = malloc(nstops * sizeof(plutovg_gradient_stop_t));
    } else {
        gradient->stops = gradient->embedded_stops;
    }

    memcpy(gradient->stops, stops, nstops * sizeof(plutovg_gradient_stop_t));
    gradient->nstops = nstops;
}

static void plutovg_gradient_stops_destroy(plutovg_gradient_paint_t* gradient)
{
    if(gradient->stops == gradient->embedded_stops)
        return;
    free(gradient->stops);
}

plutovg_paint_t* plutovg_paint_create_linear_gradient(float x1, float y1, float x2, float y2, plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix)
{
    plutovg_gradient_paint_t* gradient = plutovg_paint_create(PLUTOVG_PAINT_TYPE_GRADIENT, sizeof(plutovg_gradient_paint_t));
    gradient->type = PLUTOVG_GRADIENT_TYPE_LINEAR;
    gradient->spread = spread;
    gradient->matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
    gradient->values[0] = x1;
    gradient->values[1] = y1;
    gradient->values[2] = x2;
    gradient->values[3] = y2;
    plutovg_gradient_stops_init(gradient, stops, nstops);
    return &gradient->base;
}

plutovg_paint_t* plutovg_paint_create_radial_gradient(float cx, float cy, float cr, float fx, float fy, float fr, plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix)
{
    plutovg_gradient_paint_t* gradient = plutovg_paint_create(PLUTOVG_PAINT_TYPE_GRADIENT, sizeof(plutovg_gradient_paint_t));
    gradient->type = PLUTOVG_GRADIENT_TYPE_RADIAL;
    gradient->spread = spread;
    gradient->matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
    gradient->values[0] = cx;
    gradient->values[1] = cy;
    gradient->values[2] = cr;
    gradient->values[3] = fx;
    gradient->values[4] = fy;
    gradient->values[5] = fr;
    plutovg_gradient_stops_init(gradient, stops, nstops);
    return &gradient->base;
}

plutovg_paint_t* plutovg_paint_create_texture(plutovg_surface_t* surface, plutovg_texture_type_t type, float opacity, const plutovg_matrix_t* matrix)
{
    plutovg_texture_paint_t* texture = plutovg_paint_create(PLUTOVG_PAINT_TYPE_TEXTURE, sizeof(plutovg_texture_paint_t));
    texture->type = type;
    texture->opacity = opacity;
    texture->matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
    texture->surface = plutovg_surface_reference(surface);
    return &texture->base;
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
        if(paint->type == PLUTOVG_PAINT_TYPE_GRADIENT) {
            plutovg_gradient_paint_t* gradient = (plutovg_gradient_paint_t*)(paint);
            plutovg_gradient_stops_destroy(gradient);
        } else if(paint->type == PLUTOVG_PAINT_TYPE_TEXTURE) {
            plutovg_texture_paint_t* texture = (plutovg_texture_paint_t*)(paint);
            plutovg_surface_destroy(texture->surface);
        }

        free(freed_paint[paint->type]);
        freed_paint[paint->type] = paint;
    }
}

int plutovg_paint_get_reference_count(const plutovg_paint_t* paint)
{
    if(paint)
        return paint->ref_count;
    return 0;
}

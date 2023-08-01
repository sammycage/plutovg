#include "plutovg-private.h"

void plutovg_color_init_rgb(plutovg_color_t* color, double r, double g, double b)
{
    plutovg_color_init_rgba(color, r, g,  b, 1.0);
}

void plutovg_color_init_rgba(plutovg_color_t* color, double r, double g, double b, double a)
{
    color->r = plutovg_clamp(r, 0.0, 1.0);
    color->g = plutovg_clamp(g, 0.0, 1.0);
    color->b = plutovg_clamp(b, 0.0, 1.0);
    color->a = plutovg_clamp(a, 0.0, 1.0);
}

static void plutovg_texture_copy(plutovg_texture_t* texture, const plutovg_texture_t* source)
{
    plutovg_surface_t* surface = plutovg_surface_reference(source->surface);
    plutovg_surface_destroy(texture->surface);
    texture->type = source->type;
    texture->surface = surface;
    texture->opacity = source->opacity;
    texture->matrix = source->matrix;
}

static void plutovg_gradient_copy(plutovg_gradient_t* gradient, const plutovg_gradient_t* source)
{
    gradient->type = source->type;
    gradient->spread = source->spread;
    gradient->matrix = source->matrix;
    gradient->opacity = source->opacity;
    plutovg_array_ensure(gradient->stops, source->stops.size);
    if (gradient->stops.data != 0) {
        memcpy(gradient->stops.data, source->stops.data, source->stops.size * sizeof(plutovg_gradient_stop_t));
    }
    memcpy(gradient->values, source->values, sizeof(source->values));
}

void plutovg_paint_init(plutovg_paint_t* paint)
{
    paint->type = plutovg_paint_type_color;
    paint->color.r = 0;
    paint->color.g = 0;
    paint->color.b = 0;
    paint->color.a = 1;
    paint->texture.surface = NULL;
    plutovg_array_init(paint->gradient.stops);
}

void plutovg_paint_destroy(plutovg_paint_t* paint)
{
    plutovg_surface_destroy(paint->texture.surface);
    plutovg_array_destroy(paint->gradient.stops);
}

void plutovg_paint_copy(plutovg_paint_t* paint, const plutovg_paint_t* source)
{
    paint->type = source->type;
    paint->color = source->color;
    plutovg_gradient_copy(&paint->gradient, &paint->gradient);
    plutovg_texture_copy(&paint->texture, &paint->texture);
}

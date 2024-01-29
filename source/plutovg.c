#include "plutovg-private.h"

plutovg_surface_t* plutovg_surface_create(int width, int height)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    surface->ref = 1;
    surface->owndata = 1;
    surface->data = calloc(1, (size_t)(width * height * 4));
    surface->width = width;
    surface->height = height;
    surface->stride = width * 4;
    return surface;
}

plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    surface->ref = 1;
    surface->owndata = 0;
    surface->data = data;
    surface->width = width;
    surface->height = height;
    surface->stride = stride;
    return surface;
}

plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface)
{
    if(surface == NULL)
        return NULL;

    ++surface->ref;
    return surface;
}

void plutovg_surface_destroy(plutovg_surface_t* surface)
{
    if(surface == NULL)
        return;

    if(--surface->ref == 0) {
        if(surface->owndata)
            free(surface->data);
        free(surface);
    }
}

int plutovg_surface_get_reference_count(const plutovg_surface_t* surface)
{
    if(surface == NULL)
        return 0;

    return surface->ref;
}

unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface)
{
    return surface->data;
}

int plutovg_surface_get_width(const plutovg_surface_t* surface)
{
    return surface->width;
}

int plutovg_surface_get_height(const plutovg_surface_t* surface)
{
    return surface->height;
}

int plutovg_surface_get_stride(const plutovg_surface_t* surface)
{
    return surface->stride;
}

void plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename)
{
    unsigned char* data = plutovg_surface_get_data(surface);
    int width = plutovg_surface_get_width(surface);
    int height = plutovg_surface_get_height(surface);
    int stride = plutovg_surface_get_stride(surface);
    for(int y = 0;y < height;y++) {
        uint32_t* p = (uint32_t*)(data + stride * y);
        for(int x = 0;x < width;x++) {
            uint32_t a = p[x] >> 24;
            if(a == 0)
                continue;

            uint32_t r = (((p[x] >> 16) & 0xff) * 255) / a;
            uint32_t g = (((p[x] >> 8) & 0xff) * 255) / a;
            uint32_t b = (((p[x] >> 0) & 0xff) * 255) / a;

            p[x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }

    plutovg_stbi_write_png(filename, width, height, 4, data, stride);
    for(int y = 0;y < height;y++) {
        uint32_t* p = (uint32_t*)(data + stride * y);
        for(int x = 0;x < width;x++) {
            uint32_t a = p[x] >> 24;
            if(a == 0)
                continue;

            uint32_t r = (((p[x] >> 16) & 0xff) * a) / 255;
            uint32_t g = (((p[x] >> 8) & 0xff) * a) / 255;
            uint32_t b = (((p[x] >> 0) & 0xff) * a) / 255;

            p[x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

plutovg_state_t* plutovg_state_create(void)
{
    plutovg_state_t* state = malloc(sizeof(plutovg_state_t));
    plutovg_paint_init(&state->paint);
    plutovg_rle_init(&state->clippath);
    plutovg_matrix_init_identity(&state->matrix);
    plutovg_stroke_data_init(&state->stroke);
    state->winding = plutovg_fill_rule_non_zero;
    state->op = plutovg_operator_src_over;
    state->opacity = 1.0;
    state->clipping = 0;
    state->next = NULL;
    return state;
}

void plutovg_state_copy(plutovg_state_t* state, const plutovg_state_t* source)
{
    plutovg_paint_copy(&state->paint, &source->paint);
    plutovg_rle_copy(&state->clippath, &source->clippath);
    plutovg_stroke_data_copy(&state->stroke, &source->stroke);
    state->matrix = state->matrix;
    state->winding = state->winding;
    state->op = source->op;
    state->opacity = source->opacity;
    state->clipping = source->clipping;
    state->next = NULL;
}

void plutovg_state_destroy(plutovg_state_t* state)
{
    plutovg_paint_destroy(&state->paint);
    plutovg_rle_destroy(&state->clippath);
    plutovg_stroke_data_destroy(&state->stroke);
    free(state);
}

plutovg_t* plutovg_create(plutovg_surface_t* surface)
{
    plutovg_t* pluto = malloc(sizeof(plutovg_t));
    pluto->ref = 1;
    pluto->surface = plutovg_surface_reference(surface);
    pluto->state = plutovg_state_create();
    pluto->path = plutovg_path_create();
    pluto->freedstate = NULL;
    pluto->outline_data = NULL;
    pluto->outline_size = 0;
    plutovg_rle_init(&pluto->rle);
    plutovg_rle_init(&pluto->clippath);
    plutovg_rect_init(&pluto->clip, 0, 0, surface->width, surface->height);
    return pluto;
}

plutovg_t* plutovg_reference(plutovg_t* pluto)
{
    if(pluto == NULL)
        return NULL;

    ++pluto->ref;
    return pluto;
}

void plutovg_destroy(plutovg_t* pluto)
{
    if(pluto == NULL)
        return;

    if(--pluto->ref == 0) {
        while(pluto->state) {
            plutovg_state_t* state = pluto->state;
            pluto->state = state->next;
            plutovg_state_destroy(state);
        }

        while(pluto->freedstate) {
            plutovg_state_t* state = pluto->freedstate;
            pluto->freedstate = state->next;
            plutovg_state_destroy(state);
        }

        plutovg_rle_destroy(&pluto->rle);
        plutovg_rle_destroy(&pluto->clippath);
        plutovg_surface_destroy(pluto->surface);
        plutovg_path_destroy(pluto->path);
        free(pluto->outline_data);
        free(pluto);
    }
}

int plutovg_get_reference_count(const plutovg_t* pluto)
{
    if(pluto == NULL)
        return 0;
    return pluto->ref;
}

void plutovg_save(plutovg_t* pluto)
{
    plutovg_state_t* newstate = pluto->freedstate;
    if(newstate == NULL)
        newstate = plutovg_state_create();
    else
        pluto->freedstate = newstate->next;
    plutovg_state_copy(newstate, pluto->state);
    newstate->next = pluto->state;
    pluto->state = newstate;
}

void plutovg_restore(plutovg_t* pluto)
{
    plutovg_state_t* oldstate = pluto->state;
    pluto->state = oldstate->next;
    oldstate->next = pluto->freedstate;
    pluto->freedstate = oldstate;
}

void plutovg_set_paint_type(const plutovg_t* pluto, plutovg_paint_type_t type)
{
    pluto->state->paint.type = plutovg_paint_type_color;
}

plutovg_paint_type_t plutovg_get_paint_type(const plutovg_t* pluto)
{
    return pluto->state->paint.type;
}

void plutovg_set_rgb(plutovg_t* pluto, double r, double g, double b)
{
    plutovg_set_rgba(pluto, r, g, b, 1.0);
}

void plutovg_set_rgba(plutovg_t* pluto, double r, double g, double b, double a)
{
    pluto->state->paint.type = plutovg_paint_type_color;
    pluto->state->paint.color.r = r;
    pluto->state->paint.color.g = g;
    pluto->state->paint.color.b = b;
    pluto->state->paint.color.a = a;
}

void plutovg_set_color(plutovg_t* pluto, const plutovg_color_t* color)
{
    plutovg_set_rgba(pluto, color->r, color->g, color->b, color->a);
}

const plutovg_color_t* plutovg_get_color(const plutovg_t* pluto)
{
    return &pluto->state->paint.color;
}

void plutovg_set_linear_gradient(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_set_paint_type(pluto, plutovg_paint_type_gradient);
    plutovg_set_gradient_type(pluto, plutovg_gradient_type_linear);
    plutovg_set_gradient_spread(pluto, plutovg_spread_method_pad);
    plutovg_set_gradient_opacity(pluto, 1.0);
    plutovg_set_linear_gradient_values(pluto, x1, y1, x2, y2);
    plutovg_reset_gradient_matrix(pluto);
    plutovg_clear_gradient_stops(pluto);
}

void plutovg_set_radial_gradient(plutovg_t* pluto, double cx, double cy, double cr, double fx, double fy, double fr)
{
    plutovg_set_paint_type(pluto, plutovg_paint_type_gradient);
    plutovg_set_gradient_type(pluto, plutovg_gradient_type_radial);
    plutovg_set_gradient_spread(pluto, plutovg_spread_method_pad);
    plutovg_set_gradient_opacity(pluto, 1.0);
    plutovg_set_radial_gradient_values(pluto, cx, cy, cr, fx, fy, fr);
    plutovg_reset_gradient_matrix(pluto);
    plutovg_clear_gradient_stops(pluto);
}

void plutovg_set_gradient_type(const plutovg_t* pluto, plutovg_gradient_type_t type)
{
    pluto->state->paint.gradient.type = type;
}

plutovg_gradient_type_t plutovg_get_gradient_type(const plutovg_t* pluto)
{
    return pluto->state->paint.gradient.type;
}

void plutovg_set_gradient_spread(plutovg_t* pluto, plutovg_spread_method_t spread)
{
    pluto->state->paint.gradient.spread = spread;
}

plutovg_spread_method_t plutovg_get_gradient_spread(const plutovg_t* pluto)
{
    return pluto->state->paint.gradient.spread;
}

void plutovg_set_gradient_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    pluto->state->paint.gradient.matrix = *matrix;
}

void plutovg_reset_gradient_matrix(plutovg_t* pluto)
{
    plutovg_matrix_init_identity(&pluto->state->paint.gradient.matrix);
};

const plutovg_matrix_t* plutovg_get_gradient_matrix(const plutovg_t* pluto)
{
    return &pluto->state->paint.gradient.matrix;
}

void plutovg_set_gradient_opacity(plutovg_t* pluto, double opacity)
{
    pluto->state->paint.gradient.opacity = opacity;
}

double plutovg_get_gradient_opacity(const plutovg_t* pluto)
{
    return pluto->state->paint.gradient.opacity;
}

void plutovg_add_gradient_stop_rgb(plutovg_t* pluto, double offset, double r, double g, double b)
{
    plutovg_add_gradient_stop_rgba(pluto, offset, r, g, b, 1);
}

void plutovg_add_gradient_stop_rgba(plutovg_t* pluto, double offset, double r, double g, double b, double a)
{
    if(offset < 0.0) offset = 0.0;
    if(offset > 1.0) offset = 1.0;

    plutovg_array_ensure(pluto->state->paint.gradient.stops, 1);
    plutovg_gradient_stop_t* stops = pluto->state->paint.gradient.stops.data;
    int nstops = pluto->state->paint.gradient.stops.size;
    int i = 0;
    for(;i < nstops;i++) {
        if(offset < stops[i].offset) {
            memmove(&stops[i+1], &stops[i], (nstops - i) * sizeof(plutovg_gradient_stop_t));
            break;
        }
    }

    plutovg_gradient_stop_t* stop = &stops[i];
    stop->offset = offset;
    plutovg_color_init_rgba(&stop->color, r, g, b, a);
    pluto->state->paint.gradient.stops.size += 1;
}

void plutovg_add_gradient_stop_color(plutovg_t* pluto, double offset, const plutovg_color_t* color)
{
    plutovg_add_gradient_stop_rgba(pluto, offset, color->r, color->g, color->b, color->a);
}

void plutovg_add_gradient_stop(plutovg_t* pluto, const plutovg_gradient_stop_t* stop)
{
    plutovg_add_gradient_stop_color(pluto, stop->offset, &stop->color);
}

void plutovg_clear_gradient_stops(plutovg_t* pluto)
{
    pluto->state->paint.gradient.stops.size = 0;
}

int plutovg_get_gradient_stop_count(const plutovg_t* pluto)
{
    return pluto->state->paint.gradient.stops.size;
}

const plutovg_gradient_stop_t* plutovg_get_gradient_stop_data(const plutovg_t* pluto)
{
    return pluto->state->paint.gradient.stops.data;
}

void plutovg_set_linear_gradient_values(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    pluto->state->paint.gradient.values[0] = x1;
    pluto->state->paint.gradient.values[1] = y1;
    pluto->state->paint.gradient.values[2] = x2;
    pluto->state->paint.gradient.values[3] = y2;
}

void plutovg_set_radial_gradient_values(plutovg_t* pluto, double cx, double cy, double cr, double fx, double fy, double fr)
{
    pluto->state->paint.gradient.values[0] = cx;
    pluto->state->paint.gradient.values[1] = cy;
    pluto->state->paint.gradient.values[2] = cr;
    pluto->state->paint.gradient.values[3] = fx;
    pluto->state->paint.gradient.values[4] = fy;
    pluto->state->paint.gradient.values[5] = fr;
}

void plutovg_get_linear_gradient_values(const plutovg_t* pluto, double* x1, double* y1, double* x2, double* y2)
{
    if(x1) *x1 = pluto->state->paint.gradient.values[0];
    if(y1) *y1 = pluto->state->paint.gradient.values[1];
    if(x2) *x2 = pluto->state->paint.gradient.values[2];
    if(y2) *y2 = pluto->state->paint.gradient.values[3];
}

void plutovg_get_radial_gradient_values(const plutovg_t* pluto, double* cx, double* cy, double* cr, double* fx, double* fy, double* fr)
{
    if(cx) *cx = pluto->state->paint.gradient.values[0];
    if(cy) *cy = pluto->state->paint.gradient.values[1];
    if(cr) *cr = pluto->state->paint.gradient.values[2];
    if(fx) *fx = pluto->state->paint.gradient.values[3];
    if(fy) *fy = pluto->state->paint.gradient.values[4];
    if(fr) *fr = pluto->state->paint.gradient.values[5];
}

void plutovg_set_texture(plutovg_t* pluto, plutovg_surface_t* surface, plutovg_texture_type_t type, double x, double y)
{
    plutovg_set_paint_type(pluto, plutovg_paint_type_texture);
    plutovg_set_texture_surface(pluto, surface);
    plutovg_set_texture_type(pluto, type);
    plutovg_reset_texture_matrix(pluto, x, y);
    plutovg_set_texture_opacity(pluto, 1.0);
}

void plutovg_set_texture_surface(plutovg_t* pluto, plutovg_surface_t* surface)
{
    surface = plutovg_surface_reference(surface);
    plutovg_surface_destroy(pluto->state->paint.texture.surface);
    pluto->state->paint.texture.surface = surface;
}

plutovg_surface_t* plutovg_get_texture_surface(const plutovg_t* pluto)
{
    return pluto->state->paint.texture.surface;
}

void plutovg_set_texture_type(plutovg_t* pluto, plutovg_texture_type_t type)
{
    pluto->state->paint.texture.type = type;
}

plutovg_texture_type_t plutovg_get_texture_type(const plutovg_t* pluto)
{
    return pluto->state->paint.texture.type;
}

void plutovg_set_texture_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    pluto->state->paint.texture.matrix = *matrix;
}

void plutovg_reset_texture_matrix(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_init_translate(&pluto->state->paint.texture.matrix, -x, -y);
}

const plutovg_matrix_t* plutovg_get_texture_matrix(const plutovg_t* pluto)
{
    return &pluto->state->paint.texture.matrix;
}

void plutovg_set_texture_opacity(plutovg_t* pluto, double opacity)
{
    pluto->state->paint.texture.opacity = opacity;
}

double plutovg_get_texture_opacity(const plutovg_t* pluto)
{
    return pluto->state->paint.texture.opacity;
}

void plutovg_set_operator(plutovg_t* pluto, plutovg_operator_t op)
{
    pluto->state->op = op;
}

void plutovg_set_opacity(plutovg_t* pluto, double opacity)
{
    pluto->state->opacity = plutovg_clamp(opacity, 0.0, 1.0);
}

void plutovg_set_fill_rule(plutovg_t* pluto, plutovg_fill_rule_t winding)
{
    pluto->state->winding = winding;
}

plutovg_operator_t plutovg_get_operator(const plutovg_t* pluto)
{
    return pluto->state->op;
}

double plutovg_get_opacity(const plutovg_t* pluto)
{
    return pluto->state->opacity;
}

plutovg_fill_rule_t plutovg_get_fill_rule(const plutovg_t* pluto)
{
    return pluto->state->winding;
}

void plutovg_set_line_width(plutovg_t* pluto, double width)
{
    pluto->state->stroke.width = width;
}

void plutovg_set_line_cap(plutovg_t* pluto, plutovg_line_cap_t cap)
{
    pluto->state->stroke.cap = cap;
}

void plutovg_set_line_join(plutovg_t* pluto, plutovg_line_join_t join)
{
    pluto->state->stroke.join = join;
}

void plutovg_set_miter_limit(plutovg_t* pluto, double limit)
{
    pluto->state->stroke.miterlimit = limit;
}

double plutovg_get_line_width(const plutovg_t* pluto)
{
    return pluto->state->stroke.width;
}

plutovg_line_cap_t plutovg_get_line_cap(const plutovg_t* pluto)
{
    return pluto->state->stroke.cap;
}

plutovg_line_join_t plutovg_get_line_join(const plutovg_t* pluto)
{
    return pluto->state->stroke.join;
}

double plutovg_get_miter_limit(const plutovg_t* pluto)
{
    return pluto->state->stroke.miterlimit;
}

void plutovg_set_dash(plutovg_t* pluto, double offset, const double* data, int size)
{
    plutovg_array_ensure(pluto->state->stroke.dash, 1);
    memcpy(pluto->state->stroke.dash.data, data, size * sizeof(double));
    pluto->state->stroke.dash.offset = offset;
    pluto->state->stroke.dash.size = size;
}

void plutovg_set_dash_offset(plutovg_t* pluto, double offset)
{
    pluto->state->stroke.dash.offset = offset;
}

void plutovg_set_dash_data(plutovg_t* pluto, const double* data, int size)
{
    plutovg_array_ensure(pluto->state->stroke.dash, 1);
    memcpy(pluto->state->stroke.dash.data, data, size * sizeof(double));
    pluto->state->stroke.dash.size = size;
}

void plutovg_add_dash_data(plutovg_t* pluto, double value)
{
    plutovg_array_ensure(pluto->state->stroke.dash, 1);
    pluto->state->stroke.dash.data[pluto->state->stroke.dash.size] = value;
    pluto->state->stroke.dash.size += 1;
}

void plutovg_clear_dash_data(plutovg_t* pluto)
{
    pluto->state->stroke.dash.size = 0;
}

double plutovg_get_dash_offset(const plutovg_t* pluto)
{
    return pluto->state->stroke.dash.offset;
}

const double* plutovg_get_dash_data(const plutovg_t* pluto)
{
    return pluto->state->stroke.dash.data;
}

int plutovg_get_dash_data_count(const plutovg_t* pluto)
{
    return pluto->state->stroke.dash.size;
}

void plutovg_translate(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_translate(&pluto->state->matrix, x, y);
}

void plutovg_scale(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_scale(&pluto->state->matrix, x, y);
}

void plutovg_rotate(plutovg_t* pluto, double radians)
{
    plutovg_matrix_rotate(&pluto->state->matrix, radians);
}

void plutovg_transform(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    plutovg_matrix_multiply(&pluto->state->matrix, matrix, &pluto->state->matrix);
}

void plutovg_set_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    pluto->state->matrix = *matrix;
}

void plutovg_identity_matrix(plutovg_t* pluto)
{
    plutovg_matrix_init_identity(&pluto->state->matrix);
}

void plutovg_get_matrix(const plutovg_t* pluto, plutovg_matrix_t* matrix)
{
    *matrix = pluto->state->matrix;
}

void plutovg_move_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_move_to(pluto->path, x, y);
}

void plutovg_line_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_line_to(pluto->path, x, y);
}

void plutovg_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_path_quad_to(pluto->path, x1, y1, x2, y2);
}

void plutovg_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3)
{
    plutovg_path_cubic_to(pluto->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_rel_move_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_rel_move_to(pluto->path, x, y);
}

void plutovg_rel_line_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_rel_line_to(pluto->path, x, y);
}

void plutovg_rel_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_path_rel_quad_to(pluto->path, x1, y1, x2, y2);
}

void plutovg_rel_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3)
{
    plutovg_path_rel_cubic_to(pluto->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_rect(plutovg_t* pluto, double x, double y, double w, double h)
{
    plutovg_path_add_rect(pluto->path, x, y, w, h);
}

void plutovg_pixel(plutovg_t* pluto, double x, double y)
{
    plutovg_path_add_rect(pluto->path, x, y, 1, 1);
}

void plutovg_bitmap(plutovg_t* pluto, unsigned char* data, int w, int h, int c)
{
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            plutovg_pixel(pluto, j, i);

            unsigned char *pixelOffset = data + (j + w * i) * c;
            const double r = pixelOffset[0] / 255.0;
            const double g = pixelOffset[1] / 255.0;
            const double b = pixelOffset[2] / 255.0;
            const double a = c >= 4 ? pixelOffset[3] / 255.0 : 1.0;

            plutovg_set_rgba(pluto, r, g, b, a);
            plutovg_fill(pluto);
        }
    }
}

void plutovg_round_rect(plutovg_t* pluto, double x, double y, double w, double h, double rx, double ry)
{
    plutovg_path_add_round_rect(pluto->path, x, y, w, h, rx, ry);
}

void plutovg_ellipse(plutovg_t* pluto, double cx, double cy, double rx, double ry)
{
    plutovg_path_add_ellipse(pluto->path, cx, cy, rx, ry);
}

void plutovg_circle(plutovg_t* pluto, double cx, double cy, double r)
{
    plutovg_ellipse(pluto, cx, cy, r, r);
}

void plutovg_arc(plutovg_t* pluto, double cx, double cy, double r, double a0, double a1, int ccw)
{
    plutovg_path_add_arc(pluto->path, cx, cy, r, a0, a1, ccw);
}

void plutovg_add_path(plutovg_t* pluto, const plutovg_path_t* path)
{
    plutovg_path_add_path(pluto->path, path, NULL);
}

void plutovg_new_path(plutovg_t* pluto)
{
    plutovg_path_clear(pluto->path);
}

void plutovg_close_path(plutovg_t* pluto)
{
    plutovg_path_close(pluto->path);
}

void plutovg_get_current_point(const plutovg_t* pluto, double* x, double* y)
{
    plutovg_path_get_current_point(pluto->path, x, y);
}

plutovg_path_t* plutovg_get_path(const plutovg_t* pluto)
{
    return pluto->path;
}

void plutovg_fill(plutovg_t* pluto)
{
    plutovg_fill_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_stroke(plutovg_t* pluto)
{
    plutovg_stroke_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_clip(plutovg_t* pluto)
{
    plutovg_clip_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_paint(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    if(!state->clipping) {
        plutovg_rasterize_rect(&pluto->clippath, pluto->clip.x, pluto->clip.y, pluto->clip.w, pluto->clip.h);
        plutovg_blend(pluto, &pluto->clippath);
        return;
    }

    plutovg_blend(pluto, &state->clippath);
}

void plutovg_fill_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rasterize(pluto, &pluto->rle, pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
    if(!state->clipping) {
        plutovg_blend(pluto, &pluto->rle);
        return;
    }

    plutovg_rle_intersect(&pluto->clippath, &pluto->rle, &state->clippath);
    plutovg_blend(pluto, &pluto->clippath);
}

void plutovg_stroke_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rasterize(pluto, &pluto->rle, pluto->path, &state->matrix, &pluto->clip, &state->stroke, plutovg_fill_rule_non_zero);
    if(!state->clipping) {
        plutovg_blend(pluto, &pluto->rle);
        return;
    }

    plutovg_rle_intersect(&pluto->clippath, &pluto->rle, &state->clippath);
    plutovg_blend(pluto, &pluto->clippath);
}

void plutovg_clip_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rasterize(pluto, &pluto->rle, pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
    if(!state->clipping) {
        plutovg_rle_copy(&state->clippath, &pluto->rle);
        state->clipping = 1;
        return;
    }

    plutovg_rle_intersect(&pluto->clippath, &pluto->rle, &state->clippath);
    plutovg_rle_copy(&state->clippath, &pluto->clippath);
}

void plutovg_fill_extents(plutovg_t* pluto, plutovg_rect_t* rect)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rasterize(pluto, &pluto->rle, pluto->path, &state->matrix, NULL, NULL, state->winding);
    plutovg_rle_rect(&pluto->rle, rect);
}

void plutovg_stroke_extents(plutovg_t* pluto, plutovg_rect_t* rect)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rasterize(pluto, &pluto->rle, pluto->path, &state->matrix, NULL, &state->stroke, plutovg_fill_rule_non_zero);
    plutovg_rle_rect(&pluto->rle, rect);
}

void plutovg_clip_extents(plutovg_t* pluto, plutovg_rect_t* rect)
{
    plutovg_state_t* state = pluto->state;
    if(state->clipping) {
        plutovg_rle_rect(&state->clippath, rect);
        return;
    }

    rect->x = pluto->clip.x;
    rect->y = pluto->clip.y;
    rect->w = pluto->clip.w;
    rect->h = pluto->clip.h;
}

void plutovg_reset_clip(plutovg_t* pluto)
{
    pluto->state->clipping = 0;
}

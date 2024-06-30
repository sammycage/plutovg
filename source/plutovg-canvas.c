#include "plutovg-private.h"

#include <assert.h>

static void plutovg_state_reset(plutovg_state_t* state)
{
    plutovg_paint_destroy(state->paint);
    plutovg_matrix_init_identity(&state->matrix);
    plutovg_rle_init(&state->clip_rle);
    plutovg_array_clear(state->stroke.dash.array);
    state->paint = NULL;
    state->color = PLUTOVG_BLACK_COLOR;
    state->stroke.style = PLUTOVG_DEFAULT_STROKE_STYLE;
    state->stroke.dash.offset = 0.f;
    state->op = PLUTOVG_OPERATOR_SRC_OVER;
    state->winding = PLUTOVG_FILL_RULE_NON_ZERO;
    state->clipping = false;
    state->opacity = 1.f;
}

static plutovg_state_t* plutovg_state_create(void)
{
    plutovg_state_t* state = calloc(1, sizeof(plutovg_state_t));
    plutovg_state_reset(state);
    return state;
}

static void plutovg_state_copy(plutovg_state_t* state, const plutovg_state_t* source)
{
    assert(state->paint == NULL && state->stroke.dash.array.size == 0);
    plutovg_array_append(state->stroke.dash.array, source->stroke.dash.array);
    plutovg_rle_copy(&state->clip_rle, &source->clip_rle);
    state->paint = plutovg_paint_reference(source->paint);
    state->color = source->color;
    state->matrix = source->matrix;
    state->stroke.style = source->stroke.style;
    state->stroke.dash.offset = source->stroke.dash.offset;
    state->op = source->op;
    state->winding = source->winding;
    state->clipping = source->clipping;
    state->opacity = source->opacity;
}

static void plutovg_state_destroy(plutovg_state_t* state)
{
    plutovg_paint_destroy(state->paint);
    plutovg_array_destroy(state->stroke.dash.array);
    plutovg_rle_finish(&state->clip_rle);
    free(state);
}

plutovg_canvas_t* plutovg_canvas_create(plutovg_surface_t* surface)
{
    plutovg_canvas_t* canvas = malloc(sizeof(plutovg_canvas_t));
    canvas->ref_count = 1;
    canvas->surface = plutovg_surface_reference(surface);
    canvas->state = plutovg_state_create();
    canvas->path = plutovg_path_create();
    canvas->freed_state = NULL;
    canvas->clip_rect.x = 0;
    canvas->clip_rect.y = 0;
    canvas->clip_rect.w = surface->width;
    canvas->clip_rect.h = surface->height;
    plutovg_rle_init(&canvas->rle);
    plutovg_rle_init(&canvas->clip_rle);
    return canvas;
}

plutovg_canvas_t* plutovg_canvas_reference(plutovg_canvas_t* canvas)
{
    if(canvas == NULL)
        return NULL;
    ++canvas->ref_count;
    return canvas;
}

void plutovg_canvas_destroy(plutovg_canvas_t* canvas)
{
    if(canvas == NULL)
        return;
    if(--canvas->ref_count == 0) {
        while(canvas->state) {
            plutovg_state_t* state = canvas->state;
            canvas->state = state->next;
            plutovg_state_destroy(state);
        }

        while(canvas->freed_state) {
            plutovg_state_t* state = canvas->freed_state;
            canvas->freed_state = state->next;
            plutovg_state_destroy(state);
        }

        plutovg_rle_finish(&canvas->rle);
        plutovg_rle_finish(&canvas->clip_rle);
        plutovg_surface_destroy(canvas->surface);
        plutovg_path_destroy(canvas->path);
        free(canvas);
    }
}

int plutovg_canvas_get_reference_count(const plutovg_canvas_t* canvas)
{
    if(canvas == NULL)
        return 0;
    return canvas->ref_count;
}

void plutovg_canvas_save(plutovg_canvas_t* canvas)
{
    plutovg_state_t* new_state = canvas->freed_state;
    if(new_state == NULL)
        new_state = plutovg_state_create();
    else
        canvas->freed_state = new_state->next;
    plutovg_state_copy(new_state, canvas->state);
    new_state->next = canvas->state;
    canvas->state = new_state;
}

void plutovg_canvas_restore(plutovg_canvas_t* canvas)
{
    if(canvas->state->next == NULL)
        return;
    plutovg_state_t* old_state = canvas->state;
    canvas->state = old_state->next;
    plutovg_state_reset(old_state);
    old_state->next = canvas->freed_state;
    canvas->freed_state = old_state;
}

void plutovg_canvas_set_rgb(plutovg_canvas_t* canvas, float r, float g, float b)
{
    plutovg_canvas_set_rgba(canvas, r, g, b, 1.0);
}

void plutovg_canvas_set_rgba(plutovg_canvas_t* canvas, float r, float g, float b, float a)
{
    plutovg_paint_destroy(canvas->state->paint);
    canvas->state->color.r = plutovg_clamp(r, 0, 1);
    canvas->state->color.g = plutovg_clamp(g, 0, 1);
    canvas->state->color.b = plutovg_clamp(b, 0, 1);
    canvas->state->color.a = plutovg_clamp(a, 0, 1);
    canvas->state->paint = NULL;
}

void plutovg_canvas_set_color(plutovg_canvas_t* canvas, const plutovg_color_t* color)
{
    plutovg_canvas_set_rgba(canvas, color->r, color->g, color->b, color->a);
}

void plutovg_canvas_set_paint(plutovg_canvas_t* canvas, plutovg_paint_t* paint)
{
    paint = plutovg_paint_reference(paint);
    plutovg_paint_destroy(canvas->state->paint);
    canvas->state->paint = paint;
}

void plutovg_canvas_set_fill_rule(plutovg_canvas_t* canvas, plutovg_fill_rule_t winding)
{
    canvas->state->winding = winding;
}

void plutovg_canvas_set_operator(plutovg_canvas_t* canvas, plutovg_operator_t op)
{
    canvas->state->op = op;
}

void plutovg_canvas_set_opacity(plutovg_canvas_t* canvas, float opacity)
{
    canvas->state->opacity = opacity;
}

plutovg_fill_rule_t plutovg_canvas_get_fill_rule(const plutovg_canvas_t* canvas)
{
    return canvas->state->winding;
}

plutovg_operator_t plutovg_canvas_get_operator(const plutovg_canvas_t* canvas)
{
    return canvas->state->op;
}

float plutovg_canvas_get_opacity(const plutovg_canvas_t* canvas)
{
    return canvas->state->opacity;
}

void plutovg_canvas_set_line_width(plutovg_canvas_t* canvas, float line_width)
{
    canvas->state->stroke.style.width = line_width;
}

void plutovg_canvas_set_line_cap(plutovg_canvas_t* canvas, plutovg_line_cap_t line_cap)
{
    canvas->state->stroke.style.cap = line_cap;
}

void plutovg_canvas_set_line_join(plutovg_canvas_t* canvas, plutovg_line_join_t line_join)
{
    canvas->state->stroke.style.join = line_join;
}

void plutovg_canvas_set_miter_limit(plutovg_canvas_t* canvas, float miter_limit)
{
    canvas->state->stroke.style.miter_limit = miter_limit;
}

float plutovg_canvas_get_line_width(const plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.width;
}

plutovg_line_cap_t plutovg_canvas_get_line_cap(const plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.cap;
}

plutovg_line_join_t plutovg_canvas_get_line_join(const plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.join;
}

float plutovg_canvas_get_miter_limit(const plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.miter_limit;
}

void plutovg_canvas_set_dash(plutovg_canvas_t* canvas, float offset, const float* dashes, int ndashes)
{
    plutovg_canvas_set_dash_offset(canvas, offset);
    plutovg_canvas_set_dash_array(canvas, dashes, ndashes);
}

void plutovg_canvas_set_dash_offset(plutovg_canvas_t* canvas, float offset)
{
    canvas->state->stroke.dash.offset = offset;
}

void plutovg_canvas_set_dash_array(plutovg_canvas_t* canvas, const float* dashes, int ndashes)
{
    plutovg_array_clear(canvas->state->stroke.dash.array);
    plutovg_array_append_data(canvas->state->stroke.dash.array, dashes, ndashes);
}

float plutovg_canvas_get_dash_offset(const plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.dash.offset;
}

int plutovg_canvas_get_dash_array(const plutovg_canvas_t* canvas, const float** dashes)
{
    if(dashes)
        *dashes = canvas->state->stroke.dash.array.data;
    return canvas->state->stroke.dash.array.size;
}

void plutovg_canvas_translate(plutovg_canvas_t* canvas, float x, float y)
{
    plutovg_matrix_translate(&canvas->state->matrix, x, y);
}

void plutovg_canvas_scale(plutovg_canvas_t* canvas, float x, float y)
{
    plutovg_matrix_scale(&canvas->state->matrix, x, y);
}

void plutovg_canvas_rotate(plutovg_canvas_t* canvas, float radians)
{
    plutovg_matrix_rotate(&canvas->state->matrix, radians);
}

void plutovg_canvas_transform(plutovg_canvas_t* canvas, const plutovg_matrix_t* matrix)
{
    plutovg_matrix_multiply(&canvas->state->matrix, matrix, &canvas->state->matrix);
}

void plutovg_canvas_reset_matrix(plutovg_canvas_t* canvas)
{
    plutovg_matrix_init_identity(&canvas->state->matrix);
}

void plutovg_canvas_set_matrix(plutovg_canvas_t* canvas, const plutovg_matrix_t* matrix)
{
    canvas->state->matrix = matrix ? *matrix : PLUTOVG_IDENTITY_MATRIX;
}

void plutovg_canvas_get_matrix(const plutovg_canvas_t* canvas, plutovg_matrix_t* matrix)
{
    *matrix = canvas->state->matrix;
}

void plutovg_canvas_move_to(plutovg_canvas_t* canvas, float x, float y)
{
    plutovg_path_move_to(canvas->path, x, y);
}

void plutovg_canvas_line_to(plutovg_canvas_t* canvas, float x, float y)
{
    plutovg_path_line_to(canvas->path, x, y);
}

void plutovg_canvas_curve_to(plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2, float x3, float y3)
{
    plutovg_path_curve_to(canvas->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_canvas_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h)
{
    plutovg_path_add_rect(canvas->path, x, y, w, h);
}

void plutovg_canvas_round_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h, float rx, float ry)
{
    plutovg_path_add_round_rect(canvas->path, x, y, w, h, rx, ry);
}

void plutovg_canvas_ellipse(plutovg_canvas_t* canvas, float cx, float cy, float rx, float ry)
{
    plutovg_path_add_ellipse(canvas->path, cx, cy, rx, ry);
}

void plutovg_canvas_circle(plutovg_canvas_t* canvas, float cx, float cy, float r)
{
    plutovg_path_add_circle(canvas->path, cx, cy, r);
}

void plutovg_canvas_arc(plutovg_canvas_t* canvas, float cx, float cy, float r, float a0, float a1, bool ccw)
{
    plutovg_path_add_arc(canvas->path, cx, cy, r, a0, a1, ccw);
}

void plutovg_canvas_add_path(plutovg_canvas_t* canvas, const plutovg_path_t* path)
{
    plutovg_path_add_path(canvas->path, path, NULL);
}

void plutovg_canvas_new_path(plutovg_canvas_t* canvas)
{
    plutovg_path_reset(canvas->path);
}

void plutovg_canvas_close_path(plutovg_canvas_t* canvas)
{
    plutovg_path_close(canvas->path);
}

void plutovg_canvas_get_current_point(const plutovg_canvas_t* canvas, float* x, float* y)
{
    plutovg_path_get_current_point(canvas->path, x, y);
}

plutovg_path_t* plutovg_canvas_get_path(const plutovg_canvas_t* canvas)
{
    return canvas->path;
}

void plutovg_canvas_fill_extents(const plutovg_canvas_t* canvas, plutovg_rect_t* extents)
{
    plutovg_path_extents(canvas->path, extents);
    plutovg_matrix_map_rect(&canvas->state->matrix, extents, extents);
}

void plutovg_canvas_stroke_extents(const plutovg_canvas_t* canvas, plutovg_rect_t* extents)
{
    plutovg_stroke_data_t* stroke = &canvas->state->stroke;
    plutovg_canvas_fill_extents(canvas, extents);
    float delta = stroke->style.width / 2.f;
    extents->x -= delta;
    extents->y -= delta;
    extents->w += delta * 2.f;
    extents->h += delta * 2.f;
}

void plutovg_canvas_clip_extents(const plutovg_canvas_t* canvas, plutovg_rect_t* extents)
{
    if(canvas->state->clipping) {
        plutovg_rle_extents(&canvas->state->clip_rle, extents);
    } else {
        extents->x = canvas->clip_rect.x;
        extents->y = canvas->clip_rect.y;
        extents->w = canvas->clip_rect.w;
        extents->h = canvas->clip_rect.h;
    }
}

void plutovg_canvas_fill(plutovg_canvas_t* canvas)
{
    plutovg_canvas_fill_preserve(canvas);
    plutovg_canvas_new_path(canvas);
}

void plutovg_canvas_stroke(plutovg_canvas_t* canvas)
{
    plutovg_canvas_stroke_preserve(canvas);
    plutovg_canvas_new_path(canvas);
}

void plutovg_canvas_clip(plutovg_canvas_t* canvas)
{
    plutovg_canvas_clip_preserve(canvas);
    plutovg_canvas_new_path(canvas);
}

void plutovg_canvas_paint(plutovg_canvas_t* canvas)
{
    plutovg_state_t* state = canvas->state;
    if(state->clipping) {
        plutovg_blend(canvas, &state->clip_rle);
    } else {
        plutovg_rle_reset(&canvas->clip_rle);
        plutovg_rle_add_rect(&canvas->clip_rle, 0, 0, canvas->surface->width, canvas->surface->height);
        plutovg_blend(canvas, &canvas->clip_rle);
    }
}

void plutovg_canvas_fill_preserve(plutovg_canvas_t* canvas)
{
    plutovg_state_t* state = canvas->state;
    plutovg_rasterize(&canvas->rle, canvas->path, &state->matrix, &canvas->clip_rect, NULL, state->winding);
    if(state->clipping) {
        plutovg_rle_intersect(&canvas->clip_rle, &canvas->rle, &state->clip_rle);
        plutovg_blend(canvas, &canvas->clip_rle);
    } else {
        plutovg_blend(canvas, &canvas->rle);
    }
}

void plutovg_canvas_stroke_preserve(plutovg_canvas_t* canvas)
{
    plutovg_state_t* state = canvas->state;
    plutovg_rasterize(&canvas->rle, canvas->path, &state->matrix, &canvas->clip_rect, &state->stroke, PLUTOVG_FILL_RULE_NON_ZERO);
    if(state->clipping) {
        plutovg_rle_intersect(&canvas->clip_rle, &canvas->rle, &state->clip_rle);
        plutovg_blend(canvas, &canvas->clip_rle);
    } else {
        plutovg_blend(canvas, &canvas->rle);
    }
}

void plutovg_canvas_clip_preserve(plutovg_canvas_t* canvas)
{
    plutovg_state_t* state = canvas->state;
    if(state->clipping) {
        plutovg_rasterize(&canvas->rle, canvas->path, &state->matrix, &canvas->clip_rect, NULL, state->winding);
        plutovg_rle_intersect(&canvas->clip_rle, &canvas->rle, &state->clip_rle);
        plutovg_rle_copy(&state->clip_rle, &canvas->clip_rle);
    } else {
        plutovg_rasterize(&state->clip_rle, canvas->path, &state->matrix, &canvas->clip_rect, NULL, state->winding);
        state->clipping = true;
    }
}

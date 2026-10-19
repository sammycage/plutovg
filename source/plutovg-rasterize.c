#include "plutovg-private.h"
#include "plutovg-utils.h"

#include "plutovg-ft-raster.h"
#include "plutovg-ft-stroker.h"

#include <limits.h>

void plutovg_span_buffer_init(plutovg_span_buffer_t* span_buffer)
{
    plutovg_array_init(span_buffer->spans);
    plutovg_span_buffer_reset(span_buffer);
}

void plutovg_span_buffer_init_rect(plutovg_span_buffer_t* span_buffer, int x, int y, int width, int height)
{
    plutovg_array_clear(span_buffer->spans);
    plutovg_array_ensure(span_buffer->spans, height);
    plutovg_span_t* spans = span_buffer->spans.data;
    for(int i = 0; i < height; i++) {
        spans[i].x = x;
        spans[i].y = y + i;
        spans[i].len = width;
        spans[i].coverage = 255;
    }

    span_buffer->x = x;
    span_buffer->y = y;
    span_buffer->w = width;
    span_buffer->h = height;
    span_buffer->spans.size = height;
}

void plutovg_span_buffer_reset(plutovg_span_buffer_t* span_buffer)
{
    plutovg_array_clear(span_buffer->spans);
    span_buffer->x = 0;
    span_buffer->y = 0;
    span_buffer->w = -1;
    span_buffer->h = -1;
}

void plutovg_span_buffer_destroy(plutovg_span_buffer_t* span_buffer)
{
    plutovg_array_destroy(span_buffer->spans);
}

void plutovg_span_buffer_copy(plutovg_span_buffer_t* span_buffer, const plutovg_span_buffer_t* source)
{
    plutovg_array_clear(span_buffer->spans);
    plutovg_array_append(span_buffer->spans, source->spans);
    span_buffer->x = source->x;
    span_buffer->y = source->y;
    span_buffer->w = source->w;
    span_buffer->h = source->h;
}

bool plutovg_span_buffer_contains(const plutovg_span_buffer_t* span_buffer, float x, float y)
{
    /* No span lies outside int range, and the casts below are undefined
     * there (and for NaN). */
    if(!(x >= (float)INT_MIN && x < (float)INT_MAX && y >= (float)INT_MIN && y < (float)INT_MAX))
        return false;
    const int ix = (int)floorf(x);
    const int iy = (int)floorf(y);

    for(int i = 0; i < span_buffer->spans.size; i++) {
        plutovg_span_t* span = &span_buffer->spans.data[i];
        if(span->y != iy)
            continue;
        if(ix >= span->x && ix < (span->x + span->len)) {
            return true;
        }
    }

    return false;
}

static void plutovg_span_buffer_update_extents(plutovg_span_buffer_t* span_buffer)
{
    if(span_buffer->w != -1 && span_buffer->h != -1)
        return;
    if(span_buffer->spans.size == 0) {
        span_buffer->x = 0;
        span_buffer->y = 0;
        span_buffer->w = 0;
        span_buffer->h = 0;
        return;
    }

    plutovg_span_t* spans = span_buffer->spans.data;
    int x1 = INT_MAX;
    int y1 = spans[0].y;
    int x2 = 0;
    int y2 = spans[span_buffer->spans.size - 1].y;
    for(int i = 0; i < span_buffer->spans.size; i++) {
        if(spans[i].x < x1) x1 = spans[i].x;
        if(spans[i].x + spans[i].len > x2) x2 = spans[i].x + spans[i].len;
    }

    span_buffer->x = x1;
    span_buffer->y = y1;
    span_buffer->w = x2 - x1;
    span_buffer->h = y2 - y1 + 1;
}

void plutovg_span_buffer_extents(plutovg_span_buffer_t* span_buffer, plutovg_rect_t* extents)
{
    plutovg_span_buffer_update_extents(span_buffer);
    extents->x = span_buffer->x;
    extents->y = span_buffer->y;
    extents->w = span_buffer->w;
    extents->h = span_buffer->h;
}

void plutovg_span_buffer_intersect(plutovg_span_buffer_t* span_buffer, const plutovg_span_buffer_t* a, const plutovg_span_buffer_t* b)
{
    plutovg_span_buffer_reset(span_buffer);
    plutovg_array_ensure(span_buffer->spans, plutovg_max(a->spans.size, b->spans.size));

    plutovg_span_t* a_spans = a->spans.data;
    plutovg_span_t* a_end = a_spans + a->spans.size;

    plutovg_span_t* b_spans = b->spans.data;
    plutovg_span_t* b_end = b_spans + b->spans.size;
    while(a_spans < a_end && b_spans < b_end) {
        if(b_spans->y > a_spans->y) {
            ++a_spans;
            continue;
        }

        if(a_spans->y != b_spans->y) {
            ++b_spans;
            continue;
        }

        int ax1 = a_spans->x;
        int ax2 = ax1 + a_spans->len;
        int bx1 = b_spans->x;
        int bx2 = bx1 + b_spans->len;
        if(bx1 < ax1 && bx2 < ax1) {
            ++b_spans;
            continue;
        }

        if(ax1 < bx1 && ax2 < bx1) {
            ++a_spans;
            continue;
        }

        int x = plutovg_max(ax1, bx1);
        int len = plutovg_min(ax2, bx2) - x;
        if(len) {
            plutovg_array_ensure(span_buffer->spans, 1);
            plutovg_span_t* span = span_buffer->spans.data + span_buffer->spans.size;
            span->x = x;
            span->len = len;
            span->y = a_spans->y;
            span->coverage = (a_spans->coverage * b_spans->coverage) / 255;
            span_buffer->spans.size += 1;
        }

        if(ax2 < bx2) {
            ++a_spans;
        } else {
            ++b_spans;
        }
    }
}

#define ALIGN_SIZE(size) (((size) + 7ul) & ~7ul)
static PVG_FT_Outline* ft_outline_create(int points, int contours)
{
    size_t points_size = ALIGN_SIZE((points + contours) * sizeof(PVG_FT_Vector));
    size_t tags_size = ALIGN_SIZE((points + contours) * sizeof(char));
    size_t contours_size = ALIGN_SIZE(contours * sizeof(int));
    size_t contours_flag_size = ALIGN_SIZE(contours * sizeof(char));
    PVG_FT_Outline* outline = malloc(points_size + tags_size + contours_size + contours_flag_size + sizeof(PVG_FT_Outline));

    PVG_FT_Byte* outline_data = (PVG_FT_Byte*)(outline + 1);
    outline->points = (PVG_FT_Vector*)(outline_data);
    outline->tags = (char*)(outline_data + points_size);
    outline->contours = (int*)(outline_data + points_size + tags_size);
    outline->contours_flag = (char*)(outline_data + points_size + tags_size + contours_size);
    outline->n_points = 0;
    outline->n_contours = 0;
    outline->flags = 0x0;
    return outline;
}

static void ft_outline_destroy(PVG_FT_Outline* outline)
{
    free(outline);
}

#define FT_COORD(x) (PVG_FT_Pos)(roundf(x * 64))
static void ft_outline_move_to(PVG_FT_Outline* ft, float x, float y)
{
    ft->points[ft->n_points].x = FT_COORD(x);
    ft->points[ft->n_points].y = FT_COORD(y);
    ft->tags[ft->n_points] = PVG_FT_CURVE_TAG_ON;
    if(ft->n_points) {
        ft->contours[ft->n_contours] = ft->n_points - 1;
        ft->n_contours++;
    }

    ft->contours_flag[ft->n_contours] = 1;
    ft->n_points++;
}

static void ft_outline_line_to(PVG_FT_Outline* ft, float x, float y)
{
    ft->points[ft->n_points].x = FT_COORD(x);
    ft->points[ft->n_points].y = FT_COORD(y);
    ft->tags[ft->n_points] = PVG_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void ft_outline_cubic_to(PVG_FT_Outline* ft, float x1, float y1, float x2, float y2, float x3, float y3)
{
    ft->points[ft->n_points].x = FT_COORD(x1);
    ft->points[ft->n_points].y = FT_COORD(y1);
    ft->tags[ft->n_points] = PVG_FT_CURVE_TAG_CUBIC;
    ft->n_points++;

    ft->points[ft->n_points].x = FT_COORD(x2);
    ft->points[ft->n_points].y = FT_COORD(y2);
    ft->tags[ft->n_points] = PVG_FT_CURVE_TAG_CUBIC;
    ft->n_points++;

    ft->points[ft->n_points].x = FT_COORD(x3);
    ft->points[ft->n_points].y = FT_COORD(y3);
    ft->tags[ft->n_points] = PVG_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void ft_outline_close(PVG_FT_Outline* ft)
{
    ft->contours_flag[ft->n_contours] = 0;
    int index = ft->n_contours ? ft->contours[ft->n_contours - 1] + 1 : 0;
    if(index == ft->n_points)
        return;
    ft->points[ft->n_points].x = ft->points[index].x;
    ft->points[ft->n_points].y = ft->points[index].y;
    ft->tags[ft->n_points] = PVG_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void ft_outline_end(PVG_FT_Outline* ft)
{
    if(ft->n_points) {
        ft->contours[ft->n_contours] = ft->n_points - 1;
        ft->n_contours++;
    }
}

/*
 * Keep device coordinates inside the range the FreeType-derived stroker
 * and rasterizer can compute with.
 *
 * FT_COORD() casts float device coordinates to 26.6 fixed point without a
 * range check: undefined above ~1.4e17 px, INT64_MIN on x86-64, and the
 * fixed-point code needs far smaller values than that. The CORDIC helpers
 * in plutovg-ft-math.c take the MSB of a 32-bit truncation, so 26.6
 * deltas must stay below 2^31 (2^25 px), and the rasterizer multiplies
 * pairs of 24.8 deltas in signed long: gray_line_to() clips through
 * PVG_FT_MulDiv(), and gray_render_cubic()'s acute-angle test sums two
 * such products, which is the tightest bound. Out-of-range input
 * overflowed signed arithmetic in all three files and made the stroker
 * and rasterizer subdivide garbage for seconds and gigabytes.
 *
 * A path whose mapped points all lie within +-PLUTOVG_FT_COORD_LIMIT
 * (2^21 px) converts exactly as before. Any other path is first clipped
 * against that box: segments are split where they cross x = +-L or
 * y = +-L and the pieces outside are projected onto the box edge. The
 * projection is continuous and keeps the winding number of every point
 * inside the box, so fills are unchanged there. Cubics that leave the box
 * are halved until each piece is inside, outside or smaller than L/2,
 * at most PLUTOVG_FT_CLIP_DEPTH times; a piece still straddling an edge
 * is replaced by its chord. A piece that got below L/2 lies wholly
 * outside +-L/2, so its chord moves nothing inside that square; a piece
 * cut off by the depth limit (curves spanning more than ~2^44 px) can
 * still be larger, and its chord may then shift geometry nearer the
 * canvas. Strokes add at most PLUTOVG_FT_STROKE_LIMIT (2^20 px, see
 * ft_outline_convert_stroke()), so everything the stroker and rasterizer
 * see stays within 3 * 2^20 px per axis: 26.6 deltas below 2^28.6, single
 * 24.8 products (MulDiv) below 2^61.2, and the acute-angle sum
 * 2 * (6 * 2^28)^2 ~= 2^62.2, under 2^63 by 0.8 bit. Infinite coordinates
 * are treated as +-FLT_MAX; a path with a NaN point has no defined
 * geometry and renders nothing.
 */
#define PLUTOVG_FT_COORD_LIMIT 2097152.0
#define PLUTOVG_FT_CLIP_DEPTH 24
#define PLUTOVG_FT_STROKE_LIMIT 1048576.0

typedef struct {
    plutovg_path_t* path;
    double x, y;
    double start_x, start_y;
    float last_x, last_y;
    float pending_x, pending_y;
    bool pending;
} ft_clipper_t;

static float ft_clip_clamp(double v)
{
    const double limit = PLUTOVG_FT_COORD_LIMIT;
    return (float)(v < -limit ? -limit : (v > limit ? limit : v));
}

static bool ft_clip_on_edge(float a, float b, float c)
{
    return a == b && b == c && fabsf(a) == (float)PLUTOVG_FT_COORD_LIMIT;
}

static void ft_clip_flush(ft_clipper_t* clipper)
{
    if(clipper->pending) {
        plutovg_path_line_to(clipper->path, clipper->pending_x, clipper->pending_y);
        clipper->last_x = clipper->pending_x;
        clipper->last_y = clipper->pending_y;
        clipper->pending = false;
    }
}

/* Runs of projected points along one box edge collapse into one segment. */
static void ft_clip_emit(ft_clipper_t* clipper, double x, double y)
{
    float px = ft_clip_clamp(x);
    float py = ft_clip_clamp(y);
    if(clipper->pending) {
        if(px == clipper->pending_x && py == clipper->pending_y)
            return;
        if(ft_clip_on_edge(clipper->last_x, clipper->pending_x, px)
            || ft_clip_on_edge(clipper->last_y, clipper->pending_y, py)) {
            clipper->pending_x = px;
            clipper->pending_y = py;
            return;
        }

        ft_clip_flush(clipper);
    } else if(px == clipper->last_x && py == clipper->last_y) {
        return;
    }

    clipper->pending_x = px;
    clipper->pending_y = py;
    clipper->pending = true;
}

/*
 * Crossing points come from the line equation x * dy - y * dx = cross
 * rather than from a parameter t: when both ends are far away, t rounds
 * to the same value for every crossing and the clipped segment would
 * swing through the box. For float input the products in cross are
 * exact, so crossings stay accurate to the line's distance from the
 * origin, not to its endpoints' magnitude.
 */
static void ft_clip_line_to(ft_clipper_t* clipper, double x, double y)
{
    const double limit = PLUTOVG_FT_COORD_LIMIT;
    const double x0 = clipper->x;
    const double y0 = clipper->y;
    const double dx = x - x0;
    const double dy = y - y0;
    const double cross = x0 * y - y0 * x;

    double px[4], py[4], key[4];
    int n = 0;
    for(int side = -1; side <= 1; side += 2) {
        const double edge = side * limit;
        if(side < 0 ? (x0 < edge) != (x < edge) : (x0 > edge) != (x > edge)) {
            px[n] = edge;
            py[n] = (edge * dy - cross) / dx;
            n++;
        }

        if(side < 0 ? (y0 < edge) != (y < edge) : (y0 > edge) != (y > edge)) {
            px[n] = (edge * dx + cross) / dy;
            py[n] = edge;
            n++;
        }
    }

    for(int i = 0; i < n; i++)
        key[i] = px[i] * dx + py[i] * dy;
    for(int i = 1; i < n; i++) {
        for(int j = i; j > 0 && key[j - 1] > key[j]; j--) {
            double tmp;
            tmp = key[j]; key[j] = key[j - 1]; key[j - 1] = tmp;
            tmp = px[j]; px[j] = px[j - 1]; px[j - 1] = tmp;
            tmp = py[j]; py[j] = py[j - 1]; py[j - 1] = tmp;
        }
    }

    for(int i = 0; i < n; i++)
        ft_clip_emit(clipper, px[i], py[i]);
    ft_clip_emit(clipper, x, y);
    clipper->x = x;
    clipper->y = y;
}

static void ft_clip_cubic_to(ft_clipper_t* clipper, double x1, double y1, double x2, double y2, double x3, double y3, int depth)
{
    const double limit = PLUTOVG_FT_COORD_LIMIT;
    const double x0 = clipper->x;
    const double y0 = clipper->y;

    double min_x = plutovg_min(plutovg_min(x0, x1), plutovg_min(x2, x3));
    double max_x = plutovg_max(plutovg_max(x0, x1), plutovg_max(x2, x3));
    double min_y = plutovg_min(plutovg_min(y0, y1), plutovg_min(y2, y3));
    double max_y = plutovg_max(plutovg_max(y0, y1), plutovg_max(y2, y3));
    if(min_x >= -limit && max_x <= limit && min_y >= -limit && max_y <= limit) {
        ft_clip_flush(clipper);
        plutovg_path_cubic_to(clipper->path, (float)x1, (float)y1, (float)x2, (float)y2, (float)x3, (float)y3);
        clipper->last_x = (float)x3;
        clipper->last_y = (float)y3;
        clipper->x = x3;
        clipper->y = y3;
        return;
    }

    /* Wholly beyond one edge: the projection is a segment of that edge. */
    if(min_x > limit || max_x < -limit || min_y > limit || max_y < -limit) {
        ft_clip_emit(clipper, x3, y3);
        clipper->x = x3;
        clipper->y = y3;
        return;
    }

    if(depth == 0 || (max_x - min_x <= limit / 2 && max_y - min_y <= limit / 2)) {
        ft_clip_line_to(clipper, x3, y3);
        return;
    }

    double x01 = (x0 + x1) * 0.5, y01 = (y0 + y1) * 0.5;
    double x12 = (x1 + x2) * 0.5, y12 = (y1 + y2) * 0.5;
    double x23 = (x2 + x3) * 0.5, y23 = (y2 + y3) * 0.5;
    double x012 = (x01 + x12) * 0.5, y012 = (y01 + y12) * 0.5;
    double x123 = (x12 + x23) * 0.5, y123 = (y12 + y23) * 0.5;
    double x0123 = (x012 + x123) * 0.5, y0123 = (y012 + y123) * 0.5;
    ft_clip_cubic_to(clipper, x01, y01, x012, y012, x0123, y0123, depth - 1);
    ft_clip_cubic_to(clipper, x123, y123, x23, y23, x3, y3, depth - 1);
}

static bool ft_points_in_range(const plutovg_point_t* points, int count)
{
    for(int i = 0; i < count; i++) {
        if(!(fabsf(points[i].x) <= (float)PLUTOVG_FT_COORD_LIMIT && fabsf(points[i].y) <= (float)PLUTOVG_FT_COORD_LIMIT)) {
            return false;
        }
    }

    return true;
}

static bool ft_points_sanitize(plutovg_point_t* points, int count)
{
    for(int i = 0; i < count; i++) {
        if(isnan(points[i].x) || isnan(points[i].y))
            return false;
        if(isinf(points[i].x))
            points[i].x = copysignf(FLT_MAX, points[i].x);
        if(isinf(points[i].y)) {
            points[i].y = copysignf(FLT_MAX, points[i].y);
        }
    }

    return true;
}

/*
 * Filling closes every contour implicitly; clip that closing segment too,
 * or FreeType would close the projected contour with a chord across the
 * box. Stroking leaves open contours open.
 */
static plutovg_path_t* ft_path_clip(const plutovg_path_t* path, const plutovg_matrix_t* matrix, bool fill)
{
    ft_clipper_t clipper;
    memset(&clipper, 0, sizeof(clipper));
    clipper.path = plutovg_path_create();

    plutovg_path_iterator_t it;
    plutovg_path_iterator_init(&it, path);

    plutovg_point_t points[3];
    while(plutovg_path_iterator_has_next(&it)) {
        switch(plutovg_path_iterator_next(&it, points)) {
        case PLUTOVG_PATH_COMMAND_MOVE_TO:
            plutovg_matrix_map_points(matrix, points, points, 1);
            if(!ft_points_sanitize(points, 1))
                goto invalid;
            if(fill && clipper.path->elements.size > 0)
                ft_clip_line_to(&clipper, clipper.start_x, clipper.start_y);
            ft_clip_flush(&clipper);
            clipper.x = clipper.start_x = points[0].x;
            clipper.y = clipper.start_y = points[0].y;
            clipper.last_x = ft_clip_clamp(clipper.x);
            clipper.last_y = ft_clip_clamp(clipper.y);
            plutovg_path_move_to(clipper.path, clipper.last_x, clipper.last_y);
            break;
        case PLUTOVG_PATH_COMMAND_LINE_TO:
            plutovg_matrix_map_points(matrix, points, points, 1);
            if(!ft_points_sanitize(points, 1))
                goto invalid;
            ft_clip_line_to(&clipper, points[0].x, points[0].y);
            break;
        case PLUTOVG_PATH_COMMAND_CUBIC_TO:
            plutovg_matrix_map_points(matrix, points, points, 3);
            if(!ft_points_sanitize(points, 3))
                goto invalid;
            ft_clip_cubic_to(&clipper, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, PLUTOVG_FT_CLIP_DEPTH);
            break;
        case PLUTOVG_PATH_COMMAND_CLOSE:
            ft_clip_line_to(&clipper, clipper.start_x, clipper.start_y);
            ft_clip_flush(&clipper);
            plutovg_path_close(clipper.path);
            break;
        }
    }

    if(fill && clipper.path->elements.size > 0)
        ft_clip_line_to(&clipper, clipper.start_x, clipper.start_y);
    ft_clip_flush(&clipper);
    return clipper.path;
invalid:
    plutovg_path_destroy(clipper.path);
    return NULL;
}

static PVG_FT_Outline* ft_outline_convert_stroke(const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_stroke_data_t* stroke_data);

static PVG_FT_Outline* ft_outline_convert_path(const plutovg_path_t* path, const plutovg_matrix_t* matrix, bool fill)
{
    plutovg_path_iterator_t it;
    plutovg_path_iterator_init(&it, path);

    plutovg_point_t points[3];
    PVG_FT_Outline* outline = ft_outline_create(path->num_points, path->num_contours);
    while(plutovg_path_iterator_has_next(&it)) {
        switch(plutovg_path_iterator_next(&it, points)) {
        case PLUTOVG_PATH_COMMAND_MOVE_TO:
            plutovg_matrix_map_points(matrix, points, points, 1);
            if(!ft_points_in_range(points, 1))
                goto clip;
            ft_outline_move_to(outline, points[0].x, points[0].y);
            break;
        case PLUTOVG_PATH_COMMAND_LINE_TO:
            plutovg_matrix_map_points(matrix, points, points, 1);
            if(!ft_points_in_range(points, 1))
                goto clip;
            ft_outline_line_to(outline, points[0].x, points[0].y);
            break;
        case PLUTOVG_PATH_COMMAND_CUBIC_TO:
            plutovg_matrix_map_points(matrix, points, points, 3);
            if(!ft_points_in_range(points, 3))
                goto clip;
            ft_outline_cubic_to(outline, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
            break;
        case PLUTOVG_PATH_COMMAND_CLOSE:
            ft_outline_close(outline);
            break;
        }
    }

    ft_outline_end(outline);
    return outline;
clip:
    ft_outline_destroy(outline);
    plutovg_path_t* clipped = ft_path_clip(path, matrix, fill);
    if(clipped == NULL)
        return ft_outline_create(0, 0);
    plutovg_matrix_t identity;
    plutovg_matrix_init_identity(&identity);
    outline = ft_outline_convert_path(clipped, &identity, fill);
    plutovg_path_destroy(clipped);
    return outline;
}

static PVG_FT_Outline* ft_outline_convert(const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_stroke_data_t* stroke_data)
{
    if(stroke_data) {
        return ft_outline_convert_stroke(path, matrix, stroke_data);
    }

    return ft_outline_convert_path(path, matrix, true);
}

static PVG_FT_Outline* ft_outline_convert_dash(const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_stroke_dash_t* stroke_dash)
{
    if(stroke_dash->array.size == 0)
        return ft_outline_convert_path(path, matrix, false);
    plutovg_path_t* dashed = plutovg_path_clone_dashed(path, stroke_dash->offset, stroke_dash->array.data, stroke_dash->array.size);
    PVG_FT_Outline* outline = ft_outline_convert_path(dashed, matrix, false);
    plutovg_path_destroy(dashed);
    return outline;
}

static PVG_FT_Outline* ft_outline_convert_stroke(const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_stroke_data_t* stroke_data)
{
    double scale_x = sqrt(matrix->a * matrix->a + matrix->b * matrix->b);
    double scale_y = sqrt(matrix->c * matrix->c + matrix->d * matrix->d);

    double scale = hypot(scale_x, scale_y) / PLUTOVG_SQRT2;
    double width = stroke_data->style.width * scale;

    /* Bound how far the stroke outline reaches past the path (see
     * PLUTOVG_FT_COORD_LIMIT). Caps and round joins reach 1.42 * radius,
     * miters radius * miter_limit; the miter limit also feeds
     * PVG_FT_Vector_From_Polar() as 16.16, so keep it below 2^14. Ordinary
     * strokes are far inside both bounds. A stroke wider than 2^20 px
     * renders narrower than specified, which is visible when only its far
     * edge reaches the canvas: a round-capped 1 px segment at x = 10^6 with
     * stroke-width 2.1 * 10^6 should cover a 64 px canvas at the origin but
     * misses it. */
    double radius = width * 0.5;
    if(!(fabs(radius) <= PLUTOVG_FT_STROKE_LIMIT / 2))
        radius = isnan(radius) ? 0.0 : copysign(PLUTOVG_FT_STROKE_LIMIT / 2, radius);
    double miter_limit = stroke_data->style.miter_limit;
    if(!(miter_limit >= 1.0))
        miter_limit = 1.0;
    if(miter_limit > 16384.0)
        miter_limit = 16384.0;
    if(fabs(radius) * miter_limit > PLUTOVG_FT_STROKE_LIMIT)
        miter_limit = plutovg_max(1.0, PLUTOVG_FT_STROKE_LIMIT / fabs(radius));

    PVG_FT_Fixed ftWidth = (PVG_FT_Fixed)(radius * (1 << 6));
    PVG_FT_Fixed ftMiterLimit = (PVG_FT_Fixed)(miter_limit * (1 << 16));

    PVG_FT_Stroker_LineCap ftCap;
    switch(stroke_data->style.cap) {
    case PLUTOVG_LINE_CAP_SQUARE:
        ftCap = PVG_FT_STROKER_LINECAP_SQUARE;
        break;
    case PLUTOVG_LINE_CAP_ROUND:
        ftCap = PVG_FT_STROKER_LINECAP_ROUND;
        break;
    default:
        ftCap = PVG_FT_STROKER_LINECAP_BUTT;
        break;
    }

    PVG_FT_Stroker_LineJoin ftJoin;
    switch(stroke_data->style.join) {
    case PLUTOVG_LINE_JOIN_BEVEL:
        ftJoin = PVG_FT_STROKER_LINEJOIN_BEVEL;
        break;
    case PLUTOVG_LINE_JOIN_ROUND:
        ftJoin = PVG_FT_STROKER_LINEJOIN_ROUND;
        break;
    default:
        ftJoin = PVG_FT_STROKER_LINEJOIN_MITER_FIXED;
        break;
    }

    PVG_FT_Stroker stroker;
    PVG_FT_Stroker_New(&stroker);
    PVG_FT_Stroker_Set(stroker, ftWidth, ftCap, ftJoin, ftMiterLimit);

    PVG_FT_Outline* outline = ft_outline_convert_dash(path, matrix, &stroke_data->dash);
    PVG_FT_Stroker_ParseOutline(stroker, outline);

    PVG_FT_UInt points;
    PVG_FT_UInt contours;
    PVG_FT_Stroker_GetCounts(stroker, &points, &contours);

    PVG_FT_Outline* stroke_outline = ft_outline_create(points, contours);
    PVG_FT_Stroker_Export(stroker, stroke_outline);

    PVG_FT_Stroker_Done(stroker);
    ft_outline_destroy(outline);
    return stroke_outline;
}

static void spans_generation_callback(int count, const PVG_FT_Span* spans, void* user)
{
    plutovg_span_buffer_t* span_buffer = (plutovg_span_buffer_t*)(user);
    plutovg_array_append_data(span_buffer->spans, spans, count);
}

void plutovg_rasterize(plutovg_span_buffer_t* span_buffer, const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_rect_t* clip_rect, const plutovg_stroke_data_t* stroke_data, plutovg_fill_rule_t winding)
{
    PVG_FT_Outline* outline = ft_outline_convert(path, matrix, stroke_data);
    if(stroke_data) {
        outline->flags = PVG_FT_OUTLINE_NONE;
    } else {
        switch(winding) {
        case PLUTOVG_FILL_RULE_EVEN_ODD:
            outline->flags = PVG_FT_OUTLINE_EVEN_ODD_FILL;
            break;
        default:
            outline->flags = PVG_FT_OUTLINE_NONE;
            break;
        }
    }

    PVG_FT_Raster_Params params;
    params.flags = PVG_FT_RASTER_FLAG_DIRECT | PVG_FT_RASTER_FLAG_AA;
    params.gray_spans = spans_generation_callback;
    params.user = span_buffer;
    params.source = outline;
    if(clip_rect) {
        params.flags |= PVG_FT_RASTER_FLAG_CLIP;
        params.clip_box.xMin = (PVG_FT_Pos)clip_rect->x;
        params.clip_box.yMin = (PVG_FT_Pos)clip_rect->y;
        params.clip_box.xMax = (PVG_FT_Pos)(clip_rect->x + clip_rect->w);
        params.clip_box.yMax = (PVG_FT_Pos)(clip_rect->y + clip_rect->h);
    }

    plutovg_span_buffer_reset(span_buffer);
    PVG_FT_Raster_Render(&params);
    ft_outline_destroy(outline);
}

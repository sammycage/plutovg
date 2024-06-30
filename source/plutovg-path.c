#include "plutovg-private.h"

#include <math.h>
#include <assert.h>

void plutovg_path_iterator_init(plutovg_path_iterator_t* it, const plutovg_path_t* path)
{
    it->elements = path->elements.data;
    it->nelements = path->elements.size;
    it->element_index = 0;
}

bool plutovg_path_iterator_has_next(const plutovg_path_iterator_t* it)
{
    return it->element_index < it->nelements;
}

plutovg_path_command_t plutovg_path_iterator_next(plutovg_path_iterator_t* it, plutovg_point_t points[3])
{
    const plutovg_path_element_t* elements = it->elements + it->element_index;
    switch(elements[0].header.command) {
    case PLUTOVG_PATH_COMMAND_MOVE_TO:
    case PLUTOVG_PATH_COMMAND_LINE_TO:
    case PLUTOVG_PATH_COMMAND_CLOSE:
        points[0] = elements[1].point;
        break;
    case PLUTOVG_PATH_COMMAND_CURVE_TO:
        points[0] = elements[1].point;
        points[1] = elements[2].point;
        points[2] = elements[3].point;
        break;
    }

    it->element_index += elements[0].header.length;
    return elements[0].header.command;
}

plutovg_path_t* plutovg_path_create(void)
{
    plutovg_path_t* path = malloc(sizeof(plutovg_path_t));
    path->ref_count = 1;
    path->num_curves = 0;
    path->num_contours = 0;
    path->num_points = 0;
    path->start_point.x = 0;
    path->start_point.y = 0;
    plutovg_array_init(path->elements);
    return path;
}

static plutovg_path_element_t* plutovg_path_add_command(plutovg_path_t* path, plutovg_path_command_t command, int npoints)
{
    const int nelements = path->elements.size;
    const int length = npoints + 1;
    plutovg_array_ensure(path->elements, length);
    plutovg_path_element_t* elements = path->elements.data;
    elements[nelements].header.command = command;
    elements[nelements].header.length = length;
    path->elements.size += length;
    path->num_points += npoints;
    return elements + nelements + 1;
}

void plutovg_path_move_to(plutovg_path_t* path, float x, float y)
{
    plutovg_path_element_t* elements = plutovg_path_add_command(path, PLUTOVG_PATH_COMMAND_MOVE_TO, 1);
    elements[0].point.x = x;
    elements[0].point.y = y;

    path->start_point.x = x;
    path->start_point.y = y;
    path->num_contours += 1;
}

void plutovg_path_line_to(plutovg_path_t* path, float x, float y)
{
    plutovg_path_element_t* elements = plutovg_path_add_command(path, PLUTOVG_PATH_COMMAND_LINE_TO, 1);
    elements[0].point.x = x;
    elements[0].point.y = y;
}

void plutovg_path_curve_to(plutovg_path_t* path, float x1, float y1, float x2, float y2, float x3, float y3)
{
    plutovg_path_element_t* elements = plutovg_path_add_command(path, PLUTOVG_PATH_COMMAND_CURVE_TO, 3);
    elements[0].point.x = x1;
    elements[0].point.y = y1;
    elements[1].point.x = x2;
    elements[1].point.y = y2;
    elements[2].point.x = x3;
    elements[2].point.y = y3;
    path->num_curves += 1;
}

void plutovg_path_close(plutovg_path_t* path)
{
    if(path->elements.size == 0)
        return;
    plutovg_path_element_t* elements = plutovg_path_add_command(path, PLUTOVG_PATH_COMMAND_CLOSE, 1);
    elements[0].point.x = path->start_point.x;
    elements[0].point.y = path->start_point.y;
}

void plutovg_path_get_current_point(plutovg_path_t* path, float* x, float* y)
{
    float xx = 0.f;
    float yy = 0.f;
    if(path->num_points > 0) {
        xx = path->elements.data[path->elements.size - 1].point.x;
        yy = path->elements.data[path->elements.size - 1].point.y;
    }

    if(x) *x = xx;
    if(y) *y = yy;
}

void plutovg_path_reserve(plutovg_path_t* path, int nelements)
{
    plutovg_array_ensure(path->elements, nelements);
}

void plutovg_path_reset(plutovg_path_t* path)
{
    plutovg_array_clear(path->elements);
    path->start_point.x = 0;
    path->start_point.y = 0;
    path->num_contours = 0;
    path->num_points = 0;
    path->num_curves = 0;
}

void plutovg_path_add_rect(plutovg_path_t* path, float x, float y, float w, float h)
{
    plutovg_path_reserve(path, 6 * 2);
    plutovg_path_move_to(path, x, y);
    plutovg_path_line_to(path, x + w, y);
    plutovg_path_line_to(path, x + w, y + h);
    plutovg_path_line_to(path, x, y + h);
    plutovg_path_line_to(path, x, y);
    plutovg_path_close(path);
}

void plutovg_path_add_round_rect(plutovg_path_t* path, float x, float y, float w, float h, float rx, float ry)
{
    rx = plutovg_min(rx, w * 0.5f);
    ry = plutovg_min(ry, h * 0.5f);

    float right = x + w;
    float bottom = y + h;

    float cpx = rx * PLUTOVG_KAPPA;
    float cpy = ry * PLUTOVG_KAPPA;

    plutovg_path_reserve(path, 6 * 2 + 4 * 4);
    plutovg_path_move_to(path, x, y+ry);
    plutovg_path_curve_to(path, x, y+ry-cpy, x+rx-cpx, y, x+rx, y);
    plutovg_path_line_to(path, right-rx, y);
    plutovg_path_curve_to(path, right-rx+cpx, y, right, y+ry-cpy, right, y+ry);
    plutovg_path_line_to(path, right, bottom-ry);
    plutovg_path_curve_to(path, right, bottom-ry+cpy, right-rx+cpx, bottom, right-rx, bottom);
    plutovg_path_line_to(path, x+rx, bottom);
    plutovg_path_curve_to(path, x+rx-cpx, bottom, x, bottom-ry+cpy, x, bottom-ry);
    plutovg_path_line_to(path, x, y+ry);
    plutovg_path_close(path);
}

void plutovg_path_add_ellipse(plutovg_path_t* path, float cx, float cy, float rx, float ry)
{
    float left = cx - rx;
    float top = cy - ry;
    float right = cx + rx;
    float bottom = cy + ry;

    float cpx = rx * PLUTOVG_KAPPA;
    float cpy = ry * PLUTOVG_KAPPA;

    plutovg_path_reserve(path, 2 * 2 + 4 * 4);
    plutovg_path_move_to(path, cx, top);
    plutovg_path_curve_to(path, cx+cpx, top, right, cy-cpy, right, cy);
    plutovg_path_curve_to(path, right, cy+cpy, cx+cpx, bottom, cx, bottom);
    plutovg_path_curve_to(path, cx-cpx, bottom, left, cy+cpy, left, cy);
    plutovg_path_curve_to(path, left, cy-cpy, cx-cpx, top, cx, top);
    plutovg_path_close(path);
}

void plutovg_path_add_circle(plutovg_path_t* path, float cx, float cy, float r)
{
    plutovg_path_add_ellipse(path, cx, cy, r, r);
}

void plutovg_path_add_arc(plutovg_path_t* path, float cx, float cy, float r, float a0, float a1, bool ccw)
{
    float da = a1 - a0;
    if(fabsf(da) > PLUTOVG_TWO_PI) {
        da = PLUTOVG_TWO_PI;
    } else if(da != 0.f && ccw != (da < 0.f)) {
        da += PLUTOVG_TWO_PI * (ccw ? -1 : 1);
    }

    int seg_n = (int)(ceilf(fabsf(da) / PLUTOVG_HALF_PI));
    float a = a0;
    float ax = cx + cosf(a) * r;
    float ay = cy + sinf(a) * r;

    float seg_a = da / seg_n;
    float d = (seg_a / PLUTOVG_HALF_PI) * PLUTOVG_KAPPA * r;
    float dx = -sinf(a) * d;
    float dy = cosf(a) * d;

    plutovg_path_reserve(path, 2 + 4 * seg_n);
    if(path->elements.size == 0) {
        plutovg_path_move_to(path, ax, ay);
    } else {
        plutovg_path_line_to(path, ax, ay);
    }

    for(int i = 0; i < seg_n; i++) {
        float cp1x = ax + dx;
        float cp1y = ay + dy;

        a += seg_a;
        ax = cx + cosf(a) * r;
        ay = cy + sinf(a) * r;

        dx = -sinf(a) * d;
        dy = cosf(a) * d;

        float cp2x = ax - dx;
        float cp2y = ay - dy;
        plutovg_path_curve_to(path, cp1x, cp1y, cp2x, cp2y, ax, ay);
    }
}

void plutovg_path_transform(plutovg_path_t* path, const plutovg_matrix_t* matrix)
{
    plutovg_path_element_t* elements = path->elements.data;
    for(int i = 0; i < path->elements.size; i += elements[i].header.length) {
        switch(elements[i].header.command) {
        case PLUTOVG_PATH_COMMAND_MOVE_TO:
        case PLUTOVG_PATH_COMMAND_LINE_TO:
        case PLUTOVG_PATH_COMMAND_CLOSE:
            plutovg_matrix_map_point(matrix, &elements[i + 1].point, &elements[i + 1].point);
            break;
        case PLUTOVG_PATH_COMMAND_CURVE_TO:
            plutovg_matrix_map_point(matrix, &elements[i + 1].point, &elements[i + 1].point);
            plutovg_matrix_map_point(matrix, &elements[i + 2].point, &elements[i + 2].point);
            plutovg_matrix_map_point(matrix, &elements[i + 3].point, &elements[i + 3].point);
            break;
        }
    }
}

void plutovg_path_add_path(plutovg_path_t* path, const plutovg_path_t* source, const plutovg_matrix_t* matrix)
{
    if(matrix == NULL) {
        plutovg_array_append(path->elements, source->elements);
        path->num_curves += source->num_curves;
        path->num_contours += source->num_contours;
        path->num_points += source->num_points;
        path->start_point = source->start_point;
        return;
    }

    plutovg_path_iterator_t it;
    plutovg_path_iterator_init(&it, source);

    plutovg_point_t points[3];
    plutovg_array_ensure(path->elements, source->elements.size);
    while(plutovg_path_iterator_has_next(&it)) {
        switch(plutovg_path_iterator_next(&it, points)) {
        case PLUTOVG_PATH_COMMAND_MOVE_TO:
            plutovg_matrix_map_point(matrix, &points[0], &points[0]);
            plutovg_path_move_to(path, points[0].x, points[0].y);
            break;
        case PLUTOVG_PATH_COMMAND_LINE_TO:
            plutovg_matrix_map_point(matrix, &points[0], &points[0]);
            plutovg_path_line_to(path, points[0].x, points[0].y);
            break;
        case PLUTOVG_PATH_COMMAND_CURVE_TO:
            plutovg_matrix_map_point(matrix, &points[0], &points[0]);
            plutovg_matrix_map_point(matrix, &points[1], &points[1]);
            plutovg_matrix_map_point(matrix, &points[2], &points[2]);
            plutovg_path_curve_to(path, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
            break;
        case PLUTOVG_PATH_COMMAND_CLOSE:
            plutovg_path_close(path);
            break;
        }
    }
}

int plutovg_path_get_elements(const plutovg_path_t* path, const plutovg_path_element_t** elements)
{
    if(elements)
        *elements = path->elements.data;
    return path->elements.size;
}

plutovg_path_t* plutovg_path_reference(plutovg_path_t* path)
{
    if(path == NULL)
        return NULL;
    ++path->ref_count;
    return path;
}

void plutovg_path_destroy(plutovg_path_t* path)
{
    if(path == NULL)
        return;
    if(--path->ref_count == 0) {
        plutovg_array_destroy(path->elements);
        free(path);
    }
}

int plutovg_path_get_reference_count(const plutovg_path_t* path)
{
    if(path)
        return path->ref_count;
    return 0;
}

void plutovg_path_traverse(const plutovg_path_t* path, plutovg_path_traverse_func_t traverse_func, void* closure)
{
    plutovg_path_iterator_t it;
    plutovg_path_iterator_init(&it, path);

    plutovg_point_t points[3];
    while(plutovg_path_iterator_has_next(&it)) {
        switch(plutovg_path_iterator_next(&it, points)) {
        case PLUTOVG_PATH_COMMAND_MOVE_TO:
            traverse_func(closure, PLUTOVG_PATH_COMMAND_MOVE_TO, points, 1);
            break;
        case PLUTOVG_PATH_COMMAND_LINE_TO:
            traverse_func(closure, PLUTOVG_PATH_COMMAND_LINE_TO, points, 1);
            break;
        case PLUTOVG_PATH_COMMAND_CURVE_TO:
            traverse_func(closure, PLUTOVG_PATH_COMMAND_CURVE_TO, points, 3);
            break;
        case PLUTOVG_PATH_COMMAND_CLOSE:
            traverse_func(closure, PLUTOVG_PATH_COMMAND_CLOSE, points, 1);
            break;
        }
    }
}

typedef struct {
    float x1; float y1;
    float x2; float y2;
    float x3; float y3;
    float x4; float y4;
} bezier_t;

static void split_bezier(const bezier_t* b, bezier_t* first, bezier_t* second)
{
    float c = (b->x2 + b->x3) * 0.5;
    first->x2 = (b->x1 + b->x2) * 0.5;
    second->x3 = (b->x3 + b->x4) * 0.5;
    first->x1 = b->x1;
    second->x4 = b->x4;
    first->x3 = (first->x2 + c) * 0.5;
    second->x2 = (second->x3 + c) * 0.5;
    first->x4 = second->x1 = (first->x3 + second->x2) * 0.5;

    c = (b->y2 + b->y3) * 0.5;
    first->y2 = (b->y1 + b->y2) * 0.5;
    second->y3 = (b->y3 + b->y4) * 0.5;
    first->y1 = b->y1;
    second->y4 = b->y4;
    first->y3 = (first->y2 + c) * 0.5;
    second->y2 = (second->y3 + c) * 0.5;
    first->y4 = second->y1 = (first->y3 + second->y2) * 0.5;
}

void plutovg_path_traverse_flatten(const plutovg_path_t* path, plutovg_path_traverse_func_t traverse_func, void* closure)
{
    if(path->num_curves == 0) {
        return plutovg_path_traverse(path, traverse_func, closure);
    }

    const float threshold = 0.25f;

    plutovg_path_iterator_t it;
    plutovg_path_iterator_init(&it, path);

    bezier_t beziers[32];
    plutovg_point_t points[3];
    plutovg_point_t current_point = {0, 0};
    while(plutovg_path_iterator_has_next(&it)) {
        plutovg_path_command_t command = plutovg_path_iterator_next(&it, points);
        switch(command) {
        case PLUTOVG_PATH_COMMAND_MOVE_TO:
        case PLUTOVG_PATH_COMMAND_LINE_TO:
        case PLUTOVG_PATH_COMMAND_CLOSE:
            traverse_func(closure, command, points, 1);
            current_point = points[0];
            break;
        case PLUTOVG_PATH_COMMAND_CURVE_TO:
            beziers[0].x1 = current_point.x;
            beziers[0].y1 = current_point.y;
            beziers[0].x2 = points[0].x;
            beziers[0].y2 = points[0].y;
            beziers[0].x3 = points[1].x;
            beziers[0].y3 = points[1].y;
            beziers[0].x4 = points[2].x;
            beziers[0].y4 = points[2].y;
            bezier_t* b = beziers;
            while(b >= beziers) {
                float y4y1 = b->y4 - b->y1;
                float x4x1 = b->x4 - b->x1;
                float l = fabsf(x4x1) + fabsf(y4y1);
                float d;
                if(l > 1.f) {
                    d = fabsf((x4x1)*(b->y1 - b->y2) - (y4y1)*(b->x1 - b->x2)) + fabsf((x4x1)*(b->y1 - b->y3) - (y4y1)*(b->x1 - b->x3));
                } else {
                    d = fabsf(b->x1 - b->x2) + fabsf(b->y1 - b->y2) + fabsf(b->x1 - b->x3) + fabsf(b->y1 - b->y3);
                    l = 1.f;
                }

                if(d < threshold*l || b == beziers + 31) {
                    plutovg_point_t point = {b->x4, b->y4};
                    traverse_func(closure, PLUTOVG_PATH_COMMAND_LINE_TO, &point, 1);
                    --b;
                } else {
                    split_bezier(b, b + 1, b);
                    ++b;
                }
            }

            current_point = points[2];
            break;
        }
    }
}

typedef struct {
    const float* dashes; int ndashes;
    float start_phase; float phase;
    int start_index; int index;
    bool start_toggle; bool toggle;
    plutovg_point_t current_point;
    plutovg_path_traverse_func_t traverse_func;
    void* closure;
} dasher_t;

static void dash_traverse_func(void* closure, plutovg_path_command_t command, const plutovg_point_t* points, int npoints)
{
    dasher_t* dasher = (dasher_t*)(closure);
    if(command == PLUTOVG_PATH_COMMAND_MOVE_TO) {
        if(dasher->start_toggle)
            dasher->traverse_func(dasher->closure, PLUTOVG_PATH_COMMAND_MOVE_TO, points, npoints);
        dasher->current_point = points[0];
        dasher->phase = dasher->start_phase;
        dasher->index = dasher->start_index;
        dasher->toggle = dasher->start_toggle;
        return;
    }

    assert(command == PLUTOVG_PATH_COMMAND_LINE_TO || command == PLUTOVG_PATH_COMMAND_CLOSE);
    plutovg_point_t p0 = dasher->current_point;
    plutovg_point_t p1 = points[0];
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float dist0 = sqrtf(dx*dx + dy*dy);
    float dist1 = 0.f;
    while(dist0 - dist1 > dasher->dashes[dasher->index % dasher->ndashes] - dasher->phase) {
        dist1 += dasher->dashes[dasher->index % dasher->ndashes] - dasher->phase;
        float a = dist1 / dist0;
        plutovg_point_t p = {p0.x + a * dx, p0.y + a * dy};
        if(dasher->toggle) {
            dasher->traverse_func(dasher->closure, PLUTOVG_PATH_COMMAND_LINE_TO, &p, 1);
        } else {
            dasher->traverse_func(dasher->closure, PLUTOVG_PATH_COMMAND_MOVE_TO, &p, 1);
        }

        dasher->phase = 0.f;
        dasher->toggle = !dasher->toggle;
        dasher->index++;
    }

    if(dasher->toggle) {
        dasher->traverse_func(dasher->closure, PLUTOVG_PATH_COMMAND_LINE_TO, &p1, 1);
    }

    dasher->phase += dist0 - dist1;
    dasher->current_point = p1;
}

void plutovg_path_traverse_dashed(const plutovg_path_t* path, float offset, const float* dashes, int ndashes, plutovg_path_traverse_func_t traverse_func, void* closure)
{
    float dash_sum = 0.f;
    for(int i = 0; i < ndashes; ++i)
        dash_sum += dashes[i];
    if(ndashes % 2 == 1)
        dash_sum *= 2.f;
    if(dash_sum == 0.f) {
        return plutovg_path_traverse(path, traverse_func, closure);
    }

    dasher_t dasher;
    dasher.dashes = dashes;
    dasher.ndashes = ndashes;
    dasher.start_phase = fmodf(offset, dash_sum);
    if(dasher.start_phase < 0.f)
        dasher.start_phase += dash_sum;
    dasher.start_index = 0;
    dasher.start_toggle = true;
    while(dasher.start_phase >= dasher.dashes[dasher.start_index % dasher.ndashes]) {
        dasher.start_phase -= dashes[dasher.start_index % dasher.ndashes];
        dasher.start_toggle = !dasher.start_toggle;
        dasher.start_index++;
    }

    dasher.phase = dasher.start_phase;
    dasher.index = dasher.start_index;
    dasher.toggle = dasher.start_toggle;
    dasher.current_point.x = 0;
    dasher.current_point.y = 0;
    dasher.traverse_func = traverse_func;
    dasher.closure = closure;
    return plutovg_path_traverse_flatten(path, dash_traverse_func, &dasher);
}

typedef struct {
    plutovg_point_t current_point;
    bool is_first_point;
    float length;
    float x1;
    float y1;
    float x2;
    float y2;
} extents_calculator_t;

static void extents_traverse_func(void* closure, plutovg_path_command_t command, const plutovg_point_t* points, int npoints)
{
    extents_calculator_t* calculator = (extents_calculator_t*)(closure);
    if(calculator->is_first_point) {
        calculator->is_first_point = false;
        calculator->current_point = points[0];
        calculator->x1 = points[0].x;
        calculator->y1 = points[0].y;
        calculator->x2 = points[0].x;
        calculator->y2 = points[0].y;
        calculator->length = 0;
        return;
    }

    assert(command == PLUTOVG_PATH_COMMAND_MOVE_TO || command == PLUTOVG_PATH_COMMAND_LINE_TO || command == PLUTOVG_PATH_COMMAND_CLOSE);
    if(command == PLUTOVG_PATH_COMMAND_LINE_TO || command == PLUTOVG_PATH_COMMAND_CLOSE) {
        calculator->length += hypotf(points[0].x - calculator->current_point.x, points[0].y - calculator->current_point.y);
    }

    calculator->x1 = plutovg_min(calculator->x1, points[0].x);
    calculator->y1 = plutovg_min(calculator->y1, points[0].y);
    calculator->x2 = plutovg_max(calculator->x2, points[0].x);
    calculator->y2 = plutovg_max(calculator->y2, points[0].y);
    calculator->current_point = points[0];
}

float plutovg_path_extents(const plutovg_path_t* path, plutovg_rect_t* extents)
{
    extents_calculator_t calculator = {{0, 0}, true, 0, 0, 0, 0, 0};
    plutovg_path_traverse_flatten(path, extents_traverse_func, &calculator);
    extents->x = calculator.x1;
    extents->y = calculator.y1;
    extents->w = calculator.x2 - calculator.x1;
    extents->h = calculator.y2 - calculator.y1;
    return calculator.length;
}

float plutovg_path_length(const plutovg_path_t* path)
{
    extents_calculator_t calculator = {{0, 0}, true, 0, 0, 0, 0, 0};
    plutovg_path_traverse_flatten(path, extents_traverse_func, &calculator);
    return calculator.length;
}

plutovg_path_t* plutovg_path_clone(const plutovg_path_t* path)
{
    plutovg_path_t* clone = plutovg_path_create();
    plutovg_array_append(clone->elements, path->elements);
    clone->start_point = path->start_point;
    clone->num_contours = path->num_contours;
    clone->num_points = path->num_points;
    clone->num_curves = path->num_curves;
    return clone;
}

static void clone_traverse_func(void* closure, plutovg_path_command_t command, const plutovg_point_t* points, int npoints)
{
    plutovg_path_t* path = (plutovg_path_t*)(closure);
    switch(command) {
    case PLUTOVG_PATH_COMMAND_MOVE_TO:
        plutovg_path_move_to(path, points[0].x, points[0].y);
        break;
    case PLUTOVG_PATH_COMMAND_LINE_TO:
        plutovg_path_line_to(path, points[0].x, points[0].y);
        break;
    case PLUTOVG_PATH_COMMAND_CURVE_TO:
        plutovg_path_curve_to(path, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
        break;
    case PLUTOVG_PATH_COMMAND_CLOSE:
        plutovg_path_close(path);
        break;
    }
}

plutovg_path_t* plutovg_path_clone_flatten(const plutovg_path_t* path)
{
    plutovg_path_t* clone = plutovg_path_create();
    plutovg_path_reserve(clone, path->elements.size);
    plutovg_path_traverse_flatten(path, clone_traverse_func, clone);
    return clone;
}

plutovg_path_t* plutovg_path_clone_dashed(const plutovg_path_t* path, float offset, const float* dashes, int ndashes)
{
    plutovg_path_t* clone = plutovg_path_create();
    plutovg_path_reserve(clone, path->elements.size);
    plutovg_path_traverse_dashed(path, offset, dashes, ndashes, clone_traverse_func, clone);
    return clone;
}

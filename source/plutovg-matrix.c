#include "plutovg.h"

#include <math.h>

void plutovg_matrix_init(plutovg_matrix_t* matrix, float a, float b, float c, float d, float e, float f)
{
    matrix->a = a; matrix->b = b;
    matrix->c = c; matrix->d = d;
    matrix->e = e; matrix->f = f;
}

void plutovg_matrix_init_identity(plutovg_matrix_t* matrix)
{
    matrix->a = 1; matrix->b = 0;
    matrix->c = 0; matrix->d = 1;
    matrix->e = 0; matrix->f = 0;
}

void plutovg_matrix_init_translate(plutovg_matrix_t* matrix, float x, float y)
{
    plutovg_matrix_init(matrix, 1, 0, 0, 1, x, y);
}

void plutovg_matrix_init_scale(plutovg_matrix_t* matrix, float x, float y)
{
    plutovg_matrix_init(matrix, x, 0, 0, y, 0, 0);
}

void plutovg_matrix_init_shear(plutovg_matrix_t* matrix, float x, float y)
{
    plutovg_matrix_init(matrix, 1, tanf(y), tanf(x), 1, 0, 0);
}

void plutovg_matrix_init_rotate(plutovg_matrix_t* matrix, float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);
    plutovg_matrix_init(matrix, c, s, -s, c, 0, 0);
}

void plutovg_matrix_translate(plutovg_matrix_t* matrix, float x, float y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_translate(&m, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_scale(plutovg_matrix_t* matrix, float x, float y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_scale(&m, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_shear(plutovg_matrix_t* matrix, float x, float y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_shear(&m, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_rotate(plutovg_matrix_t* matrix, float radians)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_rotate(&m, radians);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_multiply(plutovg_matrix_t* matrix, const plutovg_matrix_t* left, const plutovg_matrix_t* right)
{
    float a = left->a * right->a + left->b * right->c;
    float b = left->a * right->b + left->b * right->d;
    float c = left->c * right->a + left->d * right->c;
    float d = left->c * right->b + left->d * right->d;
    float e = left->e * right->a + left->f * right->c + right->e;
    float f = left->e * right->b + left->f * right->d + right->f;
    plutovg_matrix_init(matrix, a, b, c, d, e, f);
}

bool plutovg_matrix_invert(const plutovg_matrix_t* matrix, plutovg_matrix_t* inverse)
{
    float det = (matrix->a * matrix->d - matrix->b * matrix->c);
    if(det == 0.0)
        return false;
    float inv_det = 1.0 / det;
    float a = matrix->a * inv_det;
    float b = matrix->b * inv_det;
    float c = matrix->c * inv_det;
    float d = matrix->d * inv_det;
    float e = (matrix->c * matrix->f - matrix->d * matrix->e) * inv_det;
    float f = (matrix->b * matrix->e - matrix->a * matrix->f) * inv_det;
    plutovg_matrix_init(inverse, d, -b, -c, a, e, f);
    return true;
}

void plutovg_matrix_map(const plutovg_matrix_t* matrix, float x, float y, float* xx, float* yy)
{
    *xx = x * matrix->a + y * matrix->c + matrix->e;
    *yy = x * matrix->b + y * matrix->d + matrix->f;
}

void plutovg_matrix_map_point(const plutovg_matrix_t* matrix, const plutovg_point_t* src, plutovg_point_t* dst)
{
    plutovg_matrix_map(matrix, src->x, src->y, &dst->x, &dst->y);
}

void plutovg_matrix_map_rect(const plutovg_matrix_t* matrix, const plutovg_rect_t* src, plutovg_rect_t* dst)
{
    plutovg_point_t p[4];
    p[0].x = src->x;
    p[0].y = src->y;
    p[1].x = src->x + src->w;
    p[1].y = src->y;
    p[2].x = src->x + src->w;
    p[2].y = src->y + src->h;
    p[3].x = src->x;
    p[3].y = src->y + src->h;

    plutovg_matrix_map_point(matrix, &p[0], &p[0]);
    plutovg_matrix_map_point(matrix, &p[1], &p[1]);
    plutovg_matrix_map_point(matrix, &p[2], &p[2]);
    plutovg_matrix_map_point(matrix, &p[3], &p[3]);

    float l = p[0].x;
    float t = p[0].y;
    float r = p[0].x;
    float b = p[0].y;

    for(int i = 1; i < 4; i++) {
        if(p[i].x < l) l = p[i].x;
        if(p[i].x > r) r = p[i].x;
        if(p[i].y < t) t = p[i].y;
        if(p[i].y > b) b = p[i].y;
    }

    dst->x = l;
    dst->y = t;
    dst->w = r - l;
    dst->h = b - t;
}

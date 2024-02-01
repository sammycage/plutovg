#ifndef PLUTOVG_H
#define PLUTOVG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @note plutovg_surface_t format is ARGB32_Premultiplied.
 */
typedef struct plutovg_surface plutovg_surface_t;

plutovg_surface_t* plutovg_surface_create(int width, int height);
plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride);
plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface);
void plutovg_surface_destroy(plutovg_surface_t* surface);
int plutovg_surface_get_reference_count(const plutovg_surface_t* surface);
unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface);
int plutovg_surface_get_width(const plutovg_surface_t* surface);
int plutovg_surface_get_height(const plutovg_surface_t* surface);
int plutovg_surface_get_stride(const plutovg_surface_t* surface);
void plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename);

typedef struct plutovg_point plutovg_point_t;

struct plutovg_point {
    double x;
    double y;
};

typedef struct plutovg_rect plutovg_rect_t;

struct plutovg_rect {
    double x;
    double y;
    double w;
    double h;
};

void plutovg_rect_init(plutovg_rect_t* rect, double x, double y, double w, double h);
void plutovg_rect_init_empty(plutovg_rect_t* rect);
void plutovg_rect_init_invalid(plutovg_rect_t* rect);
int plutovg_rect_empty(const plutovg_rect_t* rect);
int plutovg_rect_invalid(const plutovg_rect_t* rect);
void plutovg_rect_unite(plutovg_rect_t* rect, const plutovg_rect_t* source);
void plutovg_rect_intersect(plutovg_rect_t* rect, const plutovg_rect_t* source);

typedef struct plutovg_matrix plutovg_matrix_t;

struct plutovg_matrix {
    double m00; double m10;
    double m01; double m11;
    double m02; double m12;
};

void plutovg_matrix_init(plutovg_matrix_t* matrix, double m00, double m10, double m01, double m11, double m02, double m12);
void plutovg_matrix_init_identity(plutovg_matrix_t* matrix);
void plutovg_matrix_init_translate(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_init_scale(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_init_shear(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_init_rotate(plutovg_matrix_t* matrix, double radians);
void plutovg_matrix_init_rotate_translate(plutovg_matrix_t* matrix, double radians, double x, double y);
void plutovg_matrix_translate(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_scale(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_shear(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_rotate(plutovg_matrix_t* matrix, double radians);
void plutovg_matrix_rotate_translate(plutovg_matrix_t* matrix, double radians, double x, double y);
void plutovg_matrix_multiply(plutovg_matrix_t* matrix, const plutovg_matrix_t* a, const plutovg_matrix_t* b);
int plutovg_matrix_invert(plutovg_matrix_t* matrix);
void plutovg_matrix_map(const plutovg_matrix_t* matrix, double x, double y, double* _x, double* _y);
void plutovg_matrix_map_point(const plutovg_matrix_t* matrix, const plutovg_point_t* src, plutovg_point_t* dst);
void plutovg_matrix_map_rect(const plutovg_matrix_t* matrix, const plutovg_rect_t* src, plutovg_rect_t* dst);

typedef char plutovg_path_element_t;

enum {
    plutovg_path_element_move_to,
    plutovg_path_element_line_to,
    plutovg_path_element_cubic_to,
    plutovg_path_element_close
};

typedef struct plutovg_path plutovg_path_t;

plutovg_path_t* plutovg_path_create(void);
plutovg_path_t* plutovg_path_reference(plutovg_path_t* path);
void plutovg_path_destroy(plutovg_path_t* path);
int plutovg_path_get_reference_count(const plutovg_path_t* path);
void plutovg_path_move_to(plutovg_path_t* path, double x, double y);
void plutovg_path_line_to(plutovg_path_t* path, double x, double y);
void plutovg_path_quad_to(plutovg_path_t* path, double x1, double y1, double x2, double y2);
void plutovg_path_cubic_to(plutovg_path_t* path, double x1, double y1, double x2, double y2, double x3, double y3);
void plutovg_path_arc_to(plutovg_path_t* path, double x1, double y1, double x2, double y2, double radius);
void plutovg_path_close(plutovg_path_t* path);
void plutovg_path_rel_move_to(plutovg_path_t* path, double dx, double dy);
void plutovg_path_rel_line_to(plutovg_path_t* path, double dx, double dy);
void plutovg_path_rel_quad_to(plutovg_path_t* path, double dx1, double dy1, double dx2, double dy2);
void plutovg_path_rel_cubic_to(plutovg_path_t* path, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);
void plutovg_path_rel_arc_to(plutovg_path_t* path, double dx1, double dy1, double dx2, double dy2, double radius);
void plutovg_path_add_rect(plutovg_path_t* path, double x, double y, double w, double h);
void plutovg_path_add_round_rect(plutovg_path_t* path, double x, double y, double w, double h, double rx, double ry);
void plutovg_path_add_ellipse(plutovg_path_t* path, double cx, double cy, double rx, double ry);
void plutovg_path_add_circle(plutovg_path_t* path, double cx, double cy, double r);
void plutovg_path_add_arc(plutovg_path_t* path, double cx, double cy, double r, double a0, double a1, int ccw);
void plutovg_path_add_path(plutovg_path_t* path, const plutovg_path_t* source, const plutovg_matrix_t* matrix);
void plutovg_path_transform(plutovg_path_t* path, const plutovg_matrix_t* matrix);
void plutovg_path_get_current_point(const plutovg_path_t* path, double* x, double* y);
int plutovg_path_get_element_count(const plutovg_path_t* path);
plutovg_path_element_t* plutovg_path_get_elements(const plutovg_path_t* path);
int plutovg_path_get_point_count(const plutovg_path_t* path);
plutovg_point_t* plutovg_path_get_points(const plutovg_path_t* path);
void plutovg_path_clear(plutovg_path_t* path);
int plutovg_path_empty(const plutovg_path_t* path);
plutovg_path_t* plutovg_path_clone(const plutovg_path_t* path);
plutovg_path_t* plutovg_path_clone_flat(const plutovg_path_t* path);

typedef struct plutovg_color plutovg_color_t;

struct plutovg_color {
    double r;
    double g;
    double b;
    double a;
};

void plutovg_color_init_rgb(plutovg_color_t* color, double r, double g, double b);
void plutovg_color_init_rgba(plutovg_color_t* color, double r, double g, double b, double a);

typedef int plutovg_gradient_type_t;

enum {
    plutovg_gradient_type_linear,
    plutovg_gradient_type_radial
};

typedef int plutovg_spread_method_t;

enum {
    plutovg_spread_method_pad,
    plutovg_spread_method_reflect,
    plutovg_spread_method_repeat
};

typedef struct plutovg_gradient_stop plutovg_gradient_stop_t;

struct plutovg_gradient_stop {
    double offset;
    plutovg_color_t color;
};

typedef int plutovg_texture_type_t;

enum {
    plutovg_texture_type_plain,
    plutovg_texture_type_tiled
};

typedef int plutovg_paint_type_t;

enum {
    plutovg_paint_type_color,
    plutovg_paint_type_gradient,
    plutovg_paint_type_texture
};

typedef int plutovg_line_cap_t;

enum {
    plutovg_line_cap_butt,
    plutovg_line_cap_round,
    plutovg_line_cap_square
};

typedef int plutovg_line_join_t;

enum {
    plutovg_line_join_miter,
    plutovg_line_join_round,
    plutovg_line_join_bevel
};

typedef int plutovg_fill_rule_t;

enum {
    plutovg_fill_rule_non_zero,
    plutovg_fill_rule_even_odd
};

typedef int plutovg_operator_t;

enum {
    plutovg_operator_src,
    plutovg_operator_src_over,
    plutovg_operator_dst_in,
    plutovg_operator_dst_out
};

typedef struct plutovg plutovg_t;

plutovg_t* plutovg_create(plutovg_surface_t* surface);
plutovg_t* plutovg_reference(plutovg_t* pluto);
void plutovg_destroy(plutovg_t* pluto);
int plutovg_get_reference_count(const plutovg_t* pluto);
void plutovg_save(plutovg_t* pluto);
void plutovg_restore(plutovg_t* pluto);

void plutovg_set_paint_type(const plutovg_t* pluto, plutovg_paint_type_t type);
plutovg_paint_type_t plutovg_get_paint_type(const plutovg_t* pluto);

void plutovg_set_rgb(plutovg_t* pluto, double r, double g, double b);
void plutovg_set_rgba(plutovg_t* pluto, double r, double g, double b, double a);
void plutovg_set_color(plutovg_t* pluto, const plutovg_color_t* color);
const plutovg_color_t* plutovg_get_color(const plutovg_t* pluto);

void plutovg_set_linear_gradient(plutovg_t* pluto, double x1, double y1, double x2, double y2);
void plutovg_set_radial_gradient(plutovg_t* pluto, double cx, double cy, double cr, double fx, double fy, double fr);

void plutovg_set_gradient_type(const plutovg_t* pluto, plutovg_gradient_type_t type);
plutovg_gradient_type_t plutovg_get_gradient_type(const plutovg_t* pluto);

void plutovg_set_gradient_spread(plutovg_t* pluto, plutovg_spread_method_t spread);
plutovg_spread_method_t plutovg_get_gradient_spread(const plutovg_t* pluto);

void plutovg_set_gradient_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix);
void plutovg_reset_gradient_matrix(plutovg_t* pluto);
const plutovg_matrix_t* plutovg_get_gradient_matrix(const plutovg_t* pluto);

void plutovg_set_gradient_opacity(plutovg_t* pluto, double opacity);
double plutovg_get_gradient_opacity(const plutovg_t* pluto);

void plutovg_add_gradient_stop_rgb(plutovg_t* pluto, double offset, double r, double g, double b);
void plutovg_add_gradient_stop_rgba(plutovg_t* pluto, double offset, double r, double g, double b, double a);
void plutovg_add_gradient_stop_color(plutovg_t* pluto, double offset, const plutovg_color_t* color);
void plutovg_add_gradient_stop(plutovg_t* pluto, const plutovg_gradient_stop_t* stop);
void plutovg_clear_gradient_stops(plutovg_t* pluto);
int plutovg_get_gradient_stop_count(const plutovg_t* pluto);
const plutovg_gradient_stop_t* plutovg_get_gradient_stop_data(const plutovg_t* pluto);

void plutovg_set_linear_gradient_values(plutovg_t* pluto, double x1, double y1, double x2, double y2);
void plutovg_set_radial_gradient_values(plutovg_t* pluto, double cx, double cy, double cr, double fx, double fy, double fr);
void plutovg_get_linear_gradient_values(const plutovg_t* pluto, double* x1, double* y1, double* x2, double* y2);
void plutovg_get_radial_gradient_values(const plutovg_t* pluto, double* cx, double* cy, double* cr, double* fx, double* fy, double* fr);

void plutovg_set_texture(plutovg_t* pluto, plutovg_surface_t* surface, plutovg_texture_type_t type, double x, double y);

void plutovg_set_texture_surface(plutovg_t* pluto, plutovg_surface_t* surface);
plutovg_surface_t* plutovg_get_texture_surface(const plutovg_t* pluto);

void plutovg_set_texture_type(plutovg_t* pluto, plutovg_texture_type_t type);
plutovg_texture_type_t plutovg_get_texture_type(const plutovg_t* pluto);

void plutovg_set_texture_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix);
void plutovg_reset_texture_matrix(plutovg_t* pluto, double x, double y);
const plutovg_matrix_t* plutovg_get_texture_matrix(const plutovg_t* pluto);

void plutovg_set_texture_opacity(plutovg_t* pluto, double opacity);
double plutovg_get_texture_opacity(const plutovg_t* pluto);

void plutovg_set_operator(plutovg_t* pluto, plutovg_operator_t op);
void plutovg_set_opacity(plutovg_t* pluto, double opacity);
void plutovg_set_fill_rule(plutovg_t* pluto, plutovg_fill_rule_t winding);

plutovg_operator_t plutovg_get_operator(const plutovg_t* pluto);
double plutovg_get_opacity(const plutovg_t* pluto);
plutovg_fill_rule_t plutovg_get_fill_rule(const plutovg_t* pluto);

void plutovg_set_line_width(plutovg_t* pluto, double width);
void plutovg_set_line_cap(plutovg_t* pluto, plutovg_line_cap_t cap);
void plutovg_set_line_join(plutovg_t* pluto, plutovg_line_join_t join);
void plutovg_set_miter_limit(plutovg_t* pluto, double limit);

double plutovg_get_line_width(const plutovg_t* pluto);
plutovg_line_cap_t plutovg_get_line_cap(const plutovg_t* pluto);
plutovg_line_join_t plutovg_get_line_join(const plutovg_t* pluto);
double plutovg_get_miter_limit(const plutovg_t* pluto);

void plutovg_set_dash(plutovg_t* pluto, double offset, const double* data, int size);
void plutovg_set_dash_offset(plutovg_t* pluto, double offset);
void plutovg_set_dash_data(plutovg_t* pluto, const double* data, int size);
void plutovg_add_dash_data(plutovg_t* pluto, double value);
void plutovg_clear_dash_data(plutovg_t* pluto);

double plutovg_get_dash_offset(const plutovg_t* pluto);
const double* plutovg_get_dash_data(const plutovg_t* pluto);
int plutovg_get_dash_data_count(const plutovg_t* pluto);

void plutovg_translate(plutovg_t* pluto, double x, double y);
void plutovg_scale(plutovg_t* pluto, double x, double y);
void plutovg_rotate(plutovg_t* pluto, double radians);
void plutovg_transform(plutovg_t* pluto, const plutovg_matrix_t* matrix);
void plutovg_set_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix);
void plutovg_identity_matrix(plutovg_t* pluto);
void plutovg_get_matrix(const plutovg_t* pluto, plutovg_matrix_t* matrix);

void plutovg_move_to(plutovg_t* pluto, double x, double y);
void plutovg_line_to(plutovg_t* pluto, double x, double y);
void plutovg_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2);
void plutovg_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3);
void plutovg_rel_move_to(plutovg_t* pluto, double dx, double dy);
void plutovg_rel_line_to(plutovg_t* pluto, double dx, double dy);
void plutovg_rel_quad_to(plutovg_t* pluto, double dx1, double dy1, double dx2, double dy2);
void plutovg_rel_cubic_to(plutovg_t* pluto, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);

void plutovg_rect(plutovg_t* pluto, double x, double y, double w, double h);
void plutovg_pixel(plutovg_t* pluto, int x, int y);
void plutovg_image(plutovg_t* pluto, int x, int y, unsigned char* data, int w, int h, int c);
void plutovg_round_rect(plutovg_t* pluto, double x, double y, double w, double h, double rx, double ry);
void plutovg_ellipse(plutovg_t* pluto, double cx, double cy, double rx, double ry);
void plutovg_circle(plutovg_t* pluto, double cx, double cy, double r);
void plutovg_arc(plutovg_t* path, double cx, double cy, double r, double a0, double a1, int ccw);

void plutovg_add_path(plutovg_t* pluto, const plutovg_path_t* path);
void plutovg_new_path(plutovg_t* pluto);
void plutovg_close_path(plutovg_t* pluto);

void plutovg_get_current_point(const plutovg_t* pluto, double* x, double* y);
plutovg_path_t* plutovg_get_path(const plutovg_t* pluto);

void plutovg_fill(plutovg_t* pluto);
void plutovg_stroke(plutovg_t* pluto);
void plutovg_clip(plutovg_t* pluto);
void plutovg_paint(plutovg_t* pluto);

void plutovg_fill_preserve(plutovg_t* pluto);
void plutovg_stroke_preserve(plutovg_t* pluto);
void plutovg_clip_preserve(plutovg_t* pluto);

void plutovg_fill_extents(plutovg_t* pluto, plutovg_rect_t* rect);
void plutovg_stroke_extents(plutovg_t* pluto, plutovg_rect_t* rect);
void plutovg_clip_extents(plutovg_t* pluto, plutovg_rect_t* rect);
void plutovg_reset_clip(plutovg_t* pluto);

#ifdef __cplusplus
}
#endif

#endif // PLUTOVG_H

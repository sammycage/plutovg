#ifndef PLUTOVG_H
#define PLUTOVG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(PLUTOVG_BUILD_STATIC) && (defined(_WIN32) || defined(__CYGWIN__))
#define PLUTOVG_EXPORT __declspec(dllexport)
#define PLUTOVG_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#define PLUTOVG_EXPORT __attribute__((__visibility__("default")))
#define PLUTOVG_IMPORT
#else
#define PLUTOVG_EXPORT
#define PLUTOVG_IMPORT
#endif

#ifdef PLUTOVG_BUILD
#define PLUTOVG_API PLUTOVG_EXPORT
#else
#define PLUTOVG_API PLUTOVG_IMPORT
#endif

#define PLUTOVG_VERSION_MAJOR 0
#define PLUTOVG_VERSION_MINOR 0
#define PLUTOVG_VERSION_MICRO 1

#define PLUTOVG_VERSION_ENCODE(major, minor, micro) (((major) * 10000) + ((minor) * 100) + ((micro) * 1))
#define PLUTOVG_VERSION PLUTOVG_VERSION_ENCODE(PLUTOVG_VERSION_MAJOR, PLUTOVG_VERSION_MINOR, PLUTOVG_VERSION_MICRO)

#define PLUTOVG_VERSION_XSTRINGIZE(major, minor, micro) #major"."#minor"."#micro
#define PLUTOVG_VERSION_STRINGIZE(major, minor, micro) PLUTOVG_VERSION_XSTRINGIZE(major, minor, micro)
#define PLUTOVG_VERSION_STRING PLUTOVG_VERSION_STRINGIZE(PLUTOVG_VERSION_MAJOR, PLUTOVG_VERSION_MINOR, PLUTOVG_VERSION_MICRO)

/**
 * @brief Gets the version of the plutovg library.
 * @return An integer representing the version of the plutovg library.
 */
PLUTOVG_API int plutovg_version(void);

/**
 * @brief Gets the version of the plutovg library as a string.
 * @return A string representing the version of the plutovg library.
 */
PLUTOVG_API const char* plutovg_version_string(void);

/**
 * @brief A function pointer type for a cleanup callback.
 * @param closure A pointer to the resource to be cleaned up.
 */
typedef void (*plutovg_destroy_func_t)(void* closure);

/**
 * @brief A function pointer type for a write callback.
 * @param closure A pointer to user-defined data or context.
 * @param data A pointer to the data to be written.
 * @param size The size of the data in bytes.
 */
typedef void (*plutovg_write_func_t)(void* closure, void* data, int size);

/**
 * @brief A structure representing a point in 2D space.
 */
typedef struct plutovg_point {
    float x; /**< The x-coordinate of the point. */
    float y; /**< The y-coordinate of the point. */
} plutovg_point_t;

/**
 * @brief A structure representing a rectangle in 2D space.
 */
typedef struct plutovg_rect {
    float x; /**< The x-coordinate of the top-left corner of the rectangle. */
    float y; /**< The y-coordinate of the top-left corner of the rectangle. */
    float w; /**< The width of the rectangle. */
    float h; /**< The height of the rectangle. */
} plutovg_rect_t;

/**
 * @brief A structure representing a 2D transformation matrix.
 */
typedef struct plutovg_matrix {
    float a; /**< The horizontal scaling factor. */
    float b; /**< The vertical shearing factor. */
    float c; /**< The horizontal shearing factor. */
    float d; /**< The vertical scaling factor. */
    float e; /**< The horizontal translation offset. */
    float f; /**< The vertical translation offset. */
} plutovg_matrix_t;

/**
 * @brief Initializes a 2D transformation matrix.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be initialized.
 * @param a The horizontal scaling factor.
 * @param b The vertical shearing factor.
 * @param c The horizontal shearing factor.
 * @param d The vertical scaling factor.
 * @param e The horizontal translation offset.
 * @param f The vertical translation offset.
 */
PLUTOVG_API void plutovg_matrix_init(plutovg_matrix_t* matrix, float a, float b, float c, float d, float e, float f);

/**
 * @brief Initializes a 2D transformation matrix to the identity matrix.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be initialized.
 */
PLUTOVG_API void plutovg_matrix_init_identity(plutovg_matrix_t* matrix);

/**
 * @brief Initializes a 2D transformation matrix for translation.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be initialized.
 * @param tx The translation offset in the x-direction.
 * @param ty The translation offset in the y-direction.
 */
PLUTOVG_API void plutovg_matrix_init_translate(plutovg_matrix_t* matrix, float tx, float ty);

/**
 * @brief Initializes a 2D transformation matrix for scaling.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be initialized.
 * @param sx The scaling factor in the x-direction.
 * @param sy The scaling factor in the y-direction.
 */
PLUTOVG_API void plutovg_matrix_init_scale(plutovg_matrix_t* matrix, float sx, float sy);

/**
 * @brief Initializes a 2D transformation matrix for shearing.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be initialized.
 * @param shx The shearing factor in the x-direction.
 * @param shy The shearing factor in the y-direction.
 */
PLUTOVG_API void plutovg_matrix_init_shear(plutovg_matrix_t* matrix, float shx, float shy);

/**
 * @brief Initializes a 2D transformation matrix for rotation.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be initialized.
 * @param angle The rotation angle in radians.
 */
PLUTOVG_API void plutovg_matrix_init_rotate(plutovg_matrix_t* matrix, float angle);

/**
 * @brief Adds a translation with offsets `tx` and `ty` to the matrix.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be modified.
 * @param tx The translation offset in the x-direction.
 * @param ty The translation offset in the y-direction.
 */
PLUTOVG_API void plutovg_matrix_translate(plutovg_matrix_t* matrix, float tx, float ty);

/**
 * @brief Scales the matrix by factors `sx` and `sy`
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be modified.
 * @param sx The scaling factor in the x-direction.
 * @param sy The scaling factor in the y-direction.
 */
PLUTOVG_API void plutovg_matrix_scale(plutovg_matrix_t* matrix, float sx, float sy);

/**
 * @brief Shears the matrix by factors `shx` and `shy`.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be modified.
 * @param shx The shearing factor in the x-direction.
 * @param shy The shearing factor in the y-direction.
 */
PLUTOVG_API void plutovg_matrix_shear(plutovg_matrix_t* matrix, float shx, float shy);

/**
 * @brief Rotates the matrix by the specified angle (in radians).
 * @param matrix A pointer to the `plutovg_matrix_t` structure to be modified.
 * @param angle The rotation angle in radians.
 */
PLUTOVG_API void plutovg_matrix_rotate(plutovg_matrix_t* matrix, float angle);

/**
 * @brief Multiplies `left` and `right` matrices and stores the result in `matrix`.
 * @param matrix A pointer to the `plutovg_matrix_t` structure to store the result.
 * @param left A pointer to the first `plutovg_matrix_t` matrix.
 * @param right A pointer to the second `plutovg_matrix_t` matrix.
 */
PLUTOVG_API void plutovg_matrix_multiply(plutovg_matrix_t* matrix, const plutovg_matrix_t* left, const plutovg_matrix_t* right);

/**
 * @brief Calculates the inverse of `matrix` and stores it in `inverse`.
 * @note If `inverse` is `NULL`, the function only checks if the matrix is invertible.
 * @param matrix A pointer to the `plutovg_matrix_t` matrix to invert.
 * @param inverse A pointer to the `plutovg_matrix_t` structure to store the result, or `NULL`.
 * @return `true` if the matrix is invertible; `false` otherwise.
 */
PLUTOVG_API bool plutovg_matrix_invert(const plutovg_matrix_t* matrix, plutovg_matrix_t* inverse);

/**
 * @brief Transforms the point `(x, y)` using `matrix` and stores the result in `(xx, yy)`.
 * @param matrix A pointer to the `plutovg_matrix_t` transformation matrix.
 * @param x The x-coordinate of the point to transform.
 * @param y The y-coordinate of the point to transform.
 * @param xx A pointer to store the transformed x-coordinate.
 * @param yy A pointer to store the transformed y-coordinate.
 */
PLUTOVG_API void plutovg_matrix_map(const plutovg_matrix_t* matrix, float x, float y, float* xx, float* yy);

/**
 * @brief Transforms the `src` point using `matrix` and stores the result in `dst`.
 * @param matrix A pointer to the `plutovg_matrix_t` transformation matrix.
 * @param src A pointer to the `plutovg_point_t` point to transform.
 * @param dst A pointer to the `plutovg_point_t` to store the transformed point.
 */
PLUTOVG_API void plutovg_matrix_map_point(const plutovg_matrix_t* matrix, const plutovg_point_t* src, plutovg_point_t* dst);

/**
 * @brief Transforms an array of `src` points using `matrix` and stores the results in `dst`.
 * @param matrix A pointer to the `plutovg_matrix_t` transformation matrix.
 * @param src A pointer to the array of `plutovg_point_t` points to transform.
 * @param dst A pointer to the array of `plutovg_point_t` to store the transformed points.
 * @param count The number of points to transform.
 */
PLUTOVG_API void plutovg_matrix_map_points(const plutovg_matrix_t* matrix, const plutovg_point_t* src, plutovg_point_t* dst, int count);

/**
 * @brief Transforms the `src` rectangle using `matrix` and stores the result in `dst`.
 * @param matrix A pointer to the `plutovg_matrix_t` transformation matrix.
 * @param src A pointer to the `plutovg_rect_t` rectangle to transform.
 * @param dst A pointer to the `plutovg_rect_t` to store the transformed rectangle.
 */
PLUTOVG_API void plutovg_matrix_map_rect(const plutovg_matrix_t* matrix, const plutovg_rect_t* src, plutovg_rect_t* dst);

typedef struct plutovg_path plutovg_path_t;

typedef enum plutovg_path_command {
    PLUTOVG_PATH_COMMAND_MOVE_TO,
    PLUTOVG_PATH_COMMAND_LINE_TO,
    PLUTOVG_PATH_COMMAND_CUBIC_TO,
    PLUTOVG_PATH_COMMAND_CLOSE
} plutovg_path_command_t;

typedef union plutovg_path_element {
    struct {
        plutovg_path_command_t command;
        int length;
    } header;
    plutovg_point_t point;
} plutovg_path_element_t;

typedef struct plutovg_path_iterator {
    const plutovg_path_element_t* elements;
    int size;
    int index;
} plutovg_path_iterator_t;

PLUTOVG_API void plutovg_path_iterator_init(plutovg_path_iterator_t* it, const plutovg_path_t* path);
PLUTOVG_API bool plutovg_path_iterator_has_next(const plutovg_path_iterator_t* it);
PLUTOVG_API plutovg_path_command_t plutovg_path_iterator_next(plutovg_path_iterator_t* it, plutovg_point_t points[3]);

PLUTOVG_API plutovg_path_t* plutovg_path_create(void);
PLUTOVG_API void plutovg_path_move_to(plutovg_path_t* path, float x, float y);
PLUTOVG_API void plutovg_path_line_to(plutovg_path_t* path, float x, float y);
PLUTOVG_API void plutovg_path_quad_to(plutovg_path_t* path, float x1, float y1, float x2, float y2);
PLUTOVG_API void plutovg_path_cubic_to(plutovg_path_t* path, float x1, float y1, float x2, float y2, float x3, float y3);
PLUTOVG_API void plutovg_path_arc_to(plutovg_path_t* path, float rx, float ry, float angle, bool large_arc_flag, bool sweep_flag, float x, float y);
PLUTOVG_API void plutovg_path_close(plutovg_path_t* path);
PLUTOVG_API void plutovg_path_get_current_point(plutovg_path_t* path, float* x, float* y);
PLUTOVG_API void plutovg_path_reserve(plutovg_path_t* path, int count);
PLUTOVG_API void plutovg_path_reset(plutovg_path_t* path);
PLUTOVG_API void plutovg_path_add_rect(plutovg_path_t* path, float x, float y, float w, float h);
PLUTOVG_API void plutovg_path_add_round_rect(plutovg_path_t* path, float x, float y, float w, float h, float rx, float ry);
PLUTOVG_API void plutovg_path_add_ellipse(plutovg_path_t* path, float cx, float cy, float rx, float ry);
PLUTOVG_API void plutovg_path_add_circle(plutovg_path_t* path, float cx, float cy, float r);
PLUTOVG_API void plutovg_path_add_arc(plutovg_path_t* path, float cx, float cy, float r, float a0, float a1, bool ccw);
PLUTOVG_API void plutovg_path_add_path(plutovg_path_t* path, const plutovg_path_t* source, const plutovg_matrix_t* matrix);
PLUTOVG_API void plutovg_path_transform(plutovg_path_t* path, const plutovg_matrix_t* matrix);

PLUTOVG_API int plutovg_path_get_elements(const plutovg_path_t* path, const plutovg_path_element_t** elements);
PLUTOVG_API plutovg_path_t* plutovg_path_reference(plutovg_path_t* path);
PLUTOVG_API void plutovg_path_destroy(plutovg_path_t* path);
PLUTOVG_API int plutovg_path_get_reference_count(const plutovg_path_t* path);

typedef void (*plutovg_path_traverse_func_t)(void* closure, plutovg_path_command_t command, const plutovg_point_t* points, int npoints);

PLUTOVG_API void plutovg_path_traverse(const plutovg_path_t* path, plutovg_path_traverse_func_t traverse_func, void* closure);
PLUTOVG_API void plutovg_path_traverse_flatten(const plutovg_path_t* path, plutovg_path_traverse_func_t traverse_func, void* closure);
PLUTOVG_API void plutovg_path_traverse_dashed(const plutovg_path_t* path, float offset, const float* dashes, int ndashes, plutovg_path_traverse_func_t traverse_func, void* closure);

PLUTOVG_API float plutovg_path_extents(const plutovg_path_t* path, plutovg_rect_t* extents);
PLUTOVG_API float plutovg_path_length(const plutovg_path_t* path);

PLUTOVG_API plutovg_path_t* plutovg_path_clone(const plutovg_path_t* path);
PLUTOVG_API plutovg_path_t* plutovg_path_clone_flatten(const plutovg_path_t* path);
PLUTOVG_API plutovg_path_t* plutovg_path_clone_dashed(const plutovg_path_t* path, float offset, const float* dashes, int ndashes);

PLUTOVG_API bool plutovg_path_parse(plutovg_path_t* path, const char* data, int length);

typedef enum plutovg_text_encoding {
    PLUTOVG_TEXT_ENCODING_UTF8,
    PLUTOVG_TEXT_ENCODING_UTF16,
    PLUTOVG_TEXT_ENCODING_UTF32,
    PLUTOVG_TEXT_ENCODING_LATIN1
} plutovg_text_encoding_t;

typedef struct plutovg_text_iterator {
    const void* text;
    int length;
    plutovg_text_encoding_t encoding;
    int index;
} plutovg_text_iterator_t;

typedef unsigned int plutovg_codepoint_t;

PLUTOVG_API void plutovg_text_iterator_init(plutovg_text_iterator_t* it, const void* text, int length, plutovg_text_encoding_t encoding);
PLUTOVG_API bool plutovg_text_iterator_has_next(const plutovg_text_iterator_t* it);
PLUTOVG_API plutovg_codepoint_t plutovg_text_iterator_next(plutovg_text_iterator_t* it);

typedef struct plutovg_font_face plutovg_font_face_t;

PLUTOVG_API plutovg_font_face_t* plutovg_font_face_load_from_file(const char* filename, int ttcindex);
PLUTOVG_API plutovg_font_face_t* plutovg_font_face_load_from_data(const void* data, unsigned int length, int ttcindex, plutovg_destroy_func_t destroy_func, void* closure);

PLUTOVG_API plutovg_font_face_t* plutovg_font_face_reference(plutovg_font_face_t* face);
PLUTOVG_API void plutovg_font_face_destroy(plutovg_font_face_t* face);
PLUTOVG_API int plutovg_font_face_get_reference_count(const plutovg_font_face_t* face);

PLUTOVG_API void plutovg_font_face_get_metrics(const plutovg_font_face_t* face, float size, float* ascent, float* descent, float* line_gap, plutovg_rect_t* extents);
PLUTOVG_API void plutovg_font_face_get_glyph_metrics(const plutovg_font_face_t* face, float size, plutovg_codepoint_t codepoint, float* advance_width, float* left_side_bearing, plutovg_rect_t* extents);

PLUTOVG_API float plutovg_font_face_get_glyph_path(const plutovg_font_face_t* face, float size, float x, float y, plutovg_codepoint_t codepoint, plutovg_path_t* path);
PLUTOVG_API float plutovg_font_face_traverse_glyph_path(const plutovg_font_face_t* face, float size, float x, float y, plutovg_codepoint_t codepoint, plutovg_path_traverse_func_t traverse_func, void* closure);

PLUTOVG_API float plutovg_font_face_text_extents(const plutovg_font_face_t* face, float size, const void* text, int length, plutovg_text_encoding_t encoding, plutovg_rect_t* extents);

/**
 * @note plutovg_surface_t format is ARGB32_Premultiplied.
 */
typedef struct plutovg_surface plutovg_surface_t;

PLUTOVG_API plutovg_surface_t* plutovg_surface_create(int width, int height);
PLUTOVG_API plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride);

PLUTOVG_API plutovg_surface_t* plutovg_surface_load_from_image_file(const char* filename);
PLUTOVG_API plutovg_surface_t* plutovg_surface_load_from_image_data(const void* data, int length);
PLUTOVG_API plutovg_surface_t* plutovg_surface_load_from_image_base64(const char* data, int length);

PLUTOVG_API plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface);
PLUTOVG_API void plutovg_surface_destroy(plutovg_surface_t* surface);
PLUTOVG_API int plutovg_surface_get_reference_count(const plutovg_surface_t* surface);

PLUTOVG_API unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface);
PLUTOVG_API int plutovg_surface_get_width(const plutovg_surface_t* surface);
PLUTOVG_API int plutovg_surface_get_height(const plutovg_surface_t* surface);
PLUTOVG_API int plutovg_surface_get_stride(const plutovg_surface_t* surface);

PLUTOVG_API bool plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename);
PLUTOVG_API bool plutovg_surface_write_to_jpg(const plutovg_surface_t* surface, const char* filename, int quality);

PLUTOVG_API bool plutovg_surface_write_to_png_stream(const plutovg_surface_t* surface, plutovg_write_func_t write_func, void* closure);
PLUTOVG_API bool plutovg_surface_write_to_jpg_stream(const plutovg_surface_t* surface, plutovg_write_func_t write_func, void* closure, int quality);

PLUTOVG_API void plutovg_convert_argb_to_rgba(unsigned char* dst, const unsigned char* src, int width, int height, int stride);
PLUTOVG_API void plutovg_convert_rgba_to_argb(unsigned char* dst, const unsigned char* src, int width, int height, int stride);

typedef struct plutovg_color {
    float r;
    float g;
    float b;
    float a;
} plutovg_color_t;

typedef enum {
    PLUTOVG_TEXTURE_TYPE_PLAIN,
    PLUTOVG_TEXTURE_TYPE_TILED
} plutovg_texture_type_t;

typedef enum {
    PLUTOVG_SPREAD_METHOD_PAD,
    PLUTOVG_SPREAD_METHOD_REFLECT,
    PLUTOVG_SPREAD_METHOD_REPEAT
} plutovg_spread_method_t;

typedef struct plutovg_gradient_stop {
    float offset;
    plutovg_color_t color;
} plutovg_gradient_stop_t;

typedef struct plutovg_paint plutovg_paint_t;

PLUTOVG_API plutovg_paint_t* plutovg_paint_create_rgb(float r, float g, float b);
PLUTOVG_API plutovg_paint_t* plutovg_paint_create_rgba(float r, float g, float b, float a);
PLUTOVG_API plutovg_paint_t* plutovg_paint_create_color(const plutovg_color_t* color);
PLUTOVG_API plutovg_paint_t* plutovg_paint_create_linear_gradient(float x1, float y1, float x2, float y2,
    plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix);
PLUTOVG_API plutovg_paint_t* plutovg_paint_create_radial_gradient(float cx, float cy, float cr, float fx, float fy, float fr,
    plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix);
PLUTOVG_API plutovg_paint_t* plutovg_paint_create_texture(plutovg_surface_t* surface, plutovg_texture_type_t type, float opacity, const plutovg_matrix_t* matrix);

PLUTOVG_API plutovg_paint_t* plutovg_paint_reference(plutovg_paint_t* paint);
PLUTOVG_API void plutovg_paint_destroy(plutovg_paint_t* paint);
PLUTOVG_API int plutovg_paint_get_reference_count(const plutovg_paint_t* paint);

typedef enum {
    PLUTOVG_FILL_RULE_NON_ZERO,
    PLUTOVG_FILL_RULE_EVEN_ODD
} plutovg_fill_rule_t;

typedef enum {
    PLUTOVG_OPERATOR_SRC,
    PLUTOVG_OPERATOR_SRC_OVER,
    PLUTOVG_OPERATOR_DST_IN,
    PLUTOVG_OPERATOR_DST_OUT
} plutovg_operator_t;

typedef enum {
    PLUTOVG_LINE_CAP_BUTT,
    PLUTOVG_LINE_CAP_ROUND,
    PLUTOVG_LINE_CAP_SQUARE
} plutovg_line_cap_t;

typedef enum {
    PLUTOVG_LINE_JOIN_MITER,
    PLUTOVG_LINE_JOIN_ROUND,
    PLUTOVG_LINE_JOIN_BEVEL
} plutovg_line_join_t;

typedef struct plutovg_canvas plutovg_canvas_t;

PLUTOVG_API plutovg_canvas_t* plutovg_canvas_create(plutovg_surface_t* surface);

PLUTOVG_API plutovg_canvas_t* plutovg_canvas_reference(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_destroy(plutovg_canvas_t* canvas);
PLUTOVG_API int plutovg_canvas_get_reference_count(const plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_save(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_restore(plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_set_rgb(plutovg_canvas_t* canvas, float r, float g, float b);
PLUTOVG_API void plutovg_canvas_set_rgba(plutovg_canvas_t* canvas, float r, float g, float b, float a);
PLUTOVG_API void plutovg_canvas_set_color(plutovg_canvas_t* canvas, const plutovg_color_t* color);
PLUTOVG_API void plutovg_canvas_set_linear_gradient(plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2,
    plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix);
PLUTOVG_API void plutovg_canvas_set_radial_gradient(plutovg_canvas_t* canvas, float cx, float cy, float cr, float fx, float fy, float fr,
    plutovg_spread_method_t spread, const plutovg_gradient_stop_t* stops, int nstops, const plutovg_matrix_t* matrix);
PLUTOVG_API void plutovg_canvas_set_texture(plutovg_canvas_t* canvas, plutovg_surface_t* surface, plutovg_texture_type_t type, float opacity, const plutovg_matrix_t* matrix);

PLUTOVG_API void plutovg_canvas_set_paint(plutovg_canvas_t* canvas, plutovg_paint_t* paint);
PLUTOVG_API plutovg_paint_t* plutovg_canvas_get_paint(const plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_set_font(plutovg_canvas_t* canvas, plutovg_font_face_t* face, float size);
PLUTOVG_API void plutovg_canvas_set_font_face(plutovg_canvas_t* canvas, plutovg_font_face_t* face);
PLUTOVG_API void plutovg_canvas_set_font_size(plutovg_canvas_t* canvas, float size);

PLUTOVG_API plutovg_font_face_t* plutovg_canvas_get_font_face(const plutovg_canvas_t* canvas);
PLUTOVG_API float plutovg_canvas_get_font_size(const plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_set_fill_rule(plutovg_canvas_t* canvas, plutovg_fill_rule_t winding);
PLUTOVG_API void plutovg_canvas_set_operator(plutovg_canvas_t* canvas, plutovg_operator_t op);
PLUTOVG_API void plutovg_canvas_set_opacity(plutovg_canvas_t* canvas, float opacity);

PLUTOVG_API plutovg_fill_rule_t plutovg_canvas_get_fill_rule(const plutovg_canvas_t* canvas);
PLUTOVG_API plutovg_operator_t plutovg_canvas_get_operator(const plutovg_canvas_t* canvas);
PLUTOVG_API float plutovg_canvas_get_opacity(const plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_set_line_width(plutovg_canvas_t* canvas, float line_width);
PLUTOVG_API void plutovg_canvas_set_line_cap(plutovg_canvas_t* canvas, plutovg_line_cap_t line_cap);
PLUTOVG_API void plutovg_canvas_set_line_join(plutovg_canvas_t* canvas, plutovg_line_join_t line_join);
PLUTOVG_API void plutovg_canvas_set_miter_limit(plutovg_canvas_t* canvas, float miter_limit);

PLUTOVG_API float plutovg_canvas_get_line_width(const plutovg_canvas_t* canvas);
PLUTOVG_API plutovg_line_cap_t plutovg_canvas_get_line_cap(const plutovg_canvas_t* canvas);
PLUTOVG_API plutovg_line_join_t plutovg_canvas_get_line_join(const plutovg_canvas_t* canvas);
PLUTOVG_API float plutovg_canvas_get_miter_limit(const plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_set_dash(plutovg_canvas_t* canvas, float offset, const float* dashes, int ndashes);
PLUTOVG_API void plutovg_canvas_set_dash_offset(plutovg_canvas_t* canvas, float offset);
PLUTOVG_API void plutovg_canvas_set_dash_array(plutovg_canvas_t* canvas, const float* dashes, int ndashes);

PLUTOVG_API float plutovg_canvas_get_dash_offset(const plutovg_canvas_t* canvas);
PLUTOVG_API int plutovg_canvas_get_dash_array(const plutovg_canvas_t* canvas, const float** dashes);

/**
 * @brief Translates the current transformation matrix by offsets `tx` and `ty`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param tx The translation offset in the x-direction.
 * @param ty The translation offset in the y-direction.
 */
PLUTOVG_API void plutovg_canvas_translate(plutovg_canvas_t* canvas, float tx, float ty);

/**
 * @brief Scales the current transformation matrix by factors `sx` and `sy`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param sx The scaling factor in the x-direction.
 * @param sy The scaling factor in the y-direction.
 */
PLUTOVG_API void plutovg_canvas_scale(plutovg_canvas_t* canvas, float sx, float sy);

/**
 * @brief Shears the current transformation matrix by factors `shx` and `shy`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param shx The shearing factor in the x-direction.
 * @param shy The shearing factor in the y-direction.
 */
PLUTOVG_API void plutovg_canvas_shear(plutovg_canvas_t* canvas, float shx, float shy);

/**
 * @brief Rotates the current transformation matrix by the specified angle (in radians).
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param angle The rotation angle in radians.
 */
PLUTOVG_API void plutovg_canvas_rotate(plutovg_canvas_t* canvas, float angle);

/**
 * @brief Multiplies the current transformation matrix with the specified `matrix`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param matrix A pointer to the `plutovg_matrix_t` transformation matrix.
 */
PLUTOVG_API void plutovg_canvas_transform(plutovg_canvas_t* canvas, const plutovg_matrix_t* matrix);

/**
 * @brief Resets the current transformation matrix to the identity matrix.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 */
PLUTOVG_API void plutovg_canvas_reset_matrix(plutovg_canvas_t* canvas);

/**
 * @brief Resets the current transformation matrix to the specified `matrix`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param matrix A pointer to the `plutovg_matrix_t` transformation matrix.
 */
PLUTOVG_API void plutovg_canvas_set_matrix(plutovg_canvas_t* canvas, const plutovg_matrix_t* matrix);

/**
 * @brief Stores the current transformation matrix in `matrix`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param A pointer to the `plutovg_matrix_t` to store the matrix.
 */
PLUTOVG_API void plutovg_canvas_get_matrix(const plutovg_canvas_t* canvas, plutovg_matrix_t* matrix);

/**
 * @brief Transforms the point `(x, y)` using the current transformation matrix and stores the result in `(xx, yy)`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param x The x-coordinate of the point to transform.
 * @param y The y-coordinate of the point to transform.
 * @param xx A pointer to store the transformed x-coordinate.
 * @param yy A pointer to store the transformed y-coordinate.
 */
PLUTOVG_API void plutovg_canvas_map(const plutovg_canvas_t* canvas, float x, float y, float* xx, float* yy);

/**
 * @brief Transforms the `src` point using the current transformation matrix and stores the result in `dst`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param src A pointer to the `plutovg_point_t` point to transform.
 * @param dst A pointer to the `plutovg_point_t` to store the transformed point.
 */
PLUTOVG_API void plutovg_canvas_map_point(const plutovg_canvas_t* canvas, const plutovg_point_t* src, plutovg_point_t* dst);

/**
 * @brief Transforms the `src` rectangle using the current transformation matrix and stores the result in `dst`.
 * @param canvas A pointer to the `plutovg_canvas_t` canvas.
 * @param src A pointer to the `plutovg_rect_t` rectangle to transform.
 * @param dst A pointer to the `plutovg_rect_t` to store the transformed rectangle.
 */
PLUTOVG_API void plutovg_canvas_map_rect(const plutovg_canvas_t* canvas, const plutovg_rect_t* src, plutovg_rect_t* dst);

PLUTOVG_API void plutovg_canvas_move_to(plutovg_canvas_t* canvas, float x, float y);
PLUTOVG_API void plutovg_canvas_line_to(plutovg_canvas_t* canvas, float x, float y);
PLUTOVG_API void plutovg_canvas_quad_to(plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2);
PLUTOVG_API void plutovg_canvas_cubic_to(plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2, float x3, float y3);
PLUTOVG_API void plutovg_canvas_arc_to(plutovg_canvas_t* canvas, float rx, float ry, float angle, bool large_arc_flag, bool sweep_flag, float x, float y);

PLUTOVG_API void plutovg_canvas_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h);
PLUTOVG_API void plutovg_canvas_round_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h, float rx, float ry);
PLUTOVG_API void plutovg_canvas_ellipse(plutovg_canvas_t* canvas, float cx, float cy, float rx, float ry);
PLUTOVG_API void plutovg_canvas_circle(plutovg_canvas_t* canvas, float cx, float cy, float r);
PLUTOVG_API void plutovg_canvas_arc(plutovg_canvas_t* canvas, float cx, float cy, float r, float a0, float a1, bool ccw);

PLUTOVG_API void plutovg_canvas_add_path(plutovg_canvas_t* canvas, const plutovg_path_t* path);
PLUTOVG_API void plutovg_canvas_new_path(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_close_path(plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_get_current_point(const plutovg_canvas_t* canvas, float* x, float* y);
PLUTOVG_API plutovg_path_t* plutovg_canvas_get_path(const plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_fill_extents(const plutovg_canvas_t* canvas, plutovg_rect_t* extents);
PLUTOVG_API void plutovg_canvas_stroke_extents(const plutovg_canvas_t* canvas, plutovg_rect_t* extents);
PLUTOVG_API void plutovg_canvas_clip_extents(const plutovg_canvas_t* canvas, plutovg_rect_t* extents);

PLUTOVG_API void plutovg_canvas_fill(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_stroke(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_clip(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_paint(plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_fill_preserve(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_stroke_preserve(plutovg_canvas_t* canvas);
PLUTOVG_API void plutovg_canvas_clip_preserve(plutovg_canvas_t* canvas);

PLUTOVG_API void plutovg_canvas_fill_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h);
PLUTOVG_API void plutovg_canvas_fill_path(plutovg_canvas_t* canvas, const plutovg_path_t* path);

PLUTOVG_API void plutovg_canvas_stroke_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h);
PLUTOVG_API void plutovg_canvas_stroke_path(plutovg_canvas_t* canvas, const plutovg_path_t* path);

PLUTOVG_API void plutovg_canvas_clip_rect(plutovg_canvas_t* canvas, float x, float y, float w, float h);
PLUTOVG_API void plutovg_canvas_clip_path(plutovg_canvas_t* canvas, const plutovg_path_t* path);

PLUTOVG_API float plutovg_canvas_add_glyph(plutovg_canvas_t* canvas, plutovg_codepoint_t codepoint, float x, float y);
PLUTOVG_API float plutovg_canvas_add_text(plutovg_canvas_t* canvas, const void* text, int length, plutovg_text_encoding_t encoding, float x, float y);

PLUTOVG_API float plutovg_canvas_fill_text(plutovg_canvas_t* canvas, const void* text, int length, plutovg_text_encoding_t encoding, float x, float y);
PLUTOVG_API float plutovg_canvas_stroke_text(plutovg_canvas_t* canvas, const void* text, int length, plutovg_text_encoding_t encoding, float x, float y);
PLUTOVG_API float plutovg_canvas_clip_text(plutovg_canvas_t* canvas, const void* text, int length, plutovg_text_encoding_t encoding, float x, float y);

PLUTOVG_API void plutovg_canvas_font_metrics(plutovg_canvas_t* canvas, float* ascent, float* descent, float* line_gap, plutovg_rect_t* extents);
PLUTOVG_API void plutovg_canvas_glyph_metrics(plutovg_canvas_t* canvas, plutovg_codepoint_t codepoint, float* advance_width, float* left_side_bearing, plutovg_rect_t* extents);
PLUTOVG_API float plutovg_canvas_text_extents(plutovg_canvas_t* canvas, const void* text, int length, plutovg_text_encoding_t encoding, plutovg_rect_t* extents);

#ifdef __cplusplus
}
#endif

#endif // PLUTOVG_H

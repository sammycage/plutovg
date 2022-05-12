#include <plutovg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * Converts an array of four bytes at a given offset into an integer
 * @param src: Source of bytes
 * @param offset: Where to begin to get bytes from
 * @param size: Size of source array
 * @return An integer
 */
int bytes_to_int(uint8_t** src, size_t* size) {
    int value = 0;
    if (*size >= sizeof(int)) {
        value = (*src)[0] | ((*src)[1] << 8) | ((*src)[2] << 16) | ((*src)[3] << 24);
        *size -= sizeof(int);
        *src += sizeof(int);
    }
    return value;
}

double bytes_to_double(uint8_t** src, size_t* size) {
    double d = 0.0f;
    if (*size >= sizeof(double)) {
        memcpy(&d, src, sizeof(d));
        *size -= sizeof(double);
        *src += sizeof(double);
    }
    return d;
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Fuzz surface functions
    uint8_t** s_data = (uint8_t **) &data;

    const int width = bytes_to_int(s_data, &size);
    const int height = bytes_to_int(s_data, &size);

    plutovg_surface_t* surface = plutovg_surface_create(width, height);
    plutovg_t* pluto = plutovg_create(surface);

    double center_x = width * 0.5;
    double center_y = height * 0.5;
    double face_radius = bytes_to_double(s_data, &size);
    double eye_radius = bytes_to_double(s_data, &size);
    double mouth_radius = bytes_to_double(s_data, &size);
    double eye_offset_x = bytes_to_double(s_data, &size);
    double eye_offset_y = bytes_to_double(s_data, &size);
    double eye_x = center_x - eye_offset_x;
    double eye_y = center_y - eye_offset_y;

    const double pi = 3.14159265358979323846;

    plutovg_save(pluto);
    plutovg_arc(pluto, center_x, center_y, face_radius, 0, 2 * pi, 0);
    plutovg_set_source_rgb(pluto, 1, 1, 0);
    plutovg_fill_preserve(pluto);
    plutovg_set_source_rgb(pluto, 0, 0, 0);
    plutovg_set_line_width(pluto, 5);
    plutovg_stroke(pluto);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_arc(pluto, eye_x, eye_y, eye_radius, 0, 2 * pi, 0);
    plutovg_arc(pluto, center_x + eye_offset_x, eye_y, eye_radius, 0, 2 * pi, 0);
    plutovg_fill(pluto);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_arc(pluto, center_x, center_y, mouth_radius, 0, pi, 0);
    plutovg_set_line_width(pluto, 5);
    plutovg_stroke(pluto);
    plutovg_restore(pluto);

//    plutovg_surface_write_to_png(surface, "smiley.png");
    plutovg_surface_destroy(surface);
    plutovg_destroy(pluto);

    return 0;
}
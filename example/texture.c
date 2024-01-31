#include "plutovg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(void) {
    int width, height, channels;
    unsigned char *image_data1 = stbi_load("assets/texture1.png", &width, &height, &channels, 0);
    unsigned char *image_data2 = stbi_load("assets/texture2.png", &width, &height, &channels, 0);
    unsigned char *image_data3 = stbi_load("assets/texture3.png", &width, &height, &channels, 0);
    unsigned char *image_data4 = stbi_load("assets/texture4.png", &width, &height, &channels, 0);

    if (!image_data1 || !image_data2 || !image_data3 || !image_data4) {
        fprintf(stderr, "Error loading textures: %s\n", stbi_failure_reason());
        return 1;
    }

    // PlutoVG
    plutovg_surface_t *surface = plutovg_surface_create(width, height);
    plutovg_t *pluto = plutovg_create(surface);

    plutovg_save(pluto);
    plutovg_image(pluto, -width / 2, -height / 2, image_data1, width, height, channels);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_image(pluto, width / 2, -height / 2, image_data2, width, height, channels);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_image(pluto, -width / 2, height / 2, image_data3, width, height, channels);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_image(pluto, width / 2, height / 2, image_data4, width, height, channels);
    plutovg_restore(pluto);

    stbi_image_free(image_data1);
    stbi_image_free(image_data2);
    stbi_image_free(image_data3);
    stbi_image_free(image_data4);

    const double center_x = width * 0.5;
    const double center_y = height * 0.5;
    const double face_radius = width * 0.25;
    const double eye_radius = face_radius * 0.1;
    const double mouth_radius = face_radius * 0.7;
    const double eye_offset_x = face_radius * 0.35;
    const double eye_offset_y = face_radius * 0.3;
    const double eye_x = center_x - eye_offset_x;
    const double eye_y = center_y - eye_offset_y;

    const double pi = 3.14159265358979323846;

    plutovg_save(pluto);
    plutovg_arc(pluto, center_x, center_y, face_radius, 0, 2 * pi, 0);
    plutovg_set_rgb(pluto, 1, 1, 0);
    plutovg_fill_preserve(pluto);
    plutovg_set_rgb(pluto, 0, 0, 0);
    plutovg_set_line_width(pluto, 10);
    plutovg_stroke(pluto);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_arc(pluto, eye_x, eye_y, eye_radius, 0, 2 * pi, 0);
    plutovg_arc(pluto, center_x + eye_offset_x, eye_y, eye_radius, 0, 2 * pi, 0);
    plutovg_fill(pluto);
    plutovg_restore(pluto);

    plutovg_save(pluto);
    plutovg_arc(pluto, center_x, center_y, mouth_radius, 0, pi, 0);
    plutovg_set_line_width(pluto, 10);
    plutovg_stroke(pluto);
    plutovg_restore(pluto);

    plutovg_surface_write_to_png(surface, "texture.png");
    plutovg_surface_destroy(surface);
    plutovg_destroy(pluto);

    return 0;
}

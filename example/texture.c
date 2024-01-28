#include "plutovg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(void) {
    const char *image_path = "/Users/ivan/Desktop/texture.png";

    int width, height, channels;
    unsigned char *image_data = stbi_load(image_path, &width, &height, &channels, 0);

    if (!image_data) {
        fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
        return 1;
    }

    printf("Image width: %d\n", width);
    printf("Image height: %d\n", height);
    printf("Number of channels: %d\n", channels);

    // PlutoVG
    plutovg_surface_t *surface = plutovg_surface_create(width, height);
    plutovg_t *pluto = plutovg_create(surface);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            plutovg_save(pluto);
            plutovg_pixel(pluto, j, i);

            unsigned char *pixelOffset = image_data + (j + width * i) * channels;
            const double r = pixelOffset[0] / 255.0;
            const double g = pixelOffset[1] / 255.0;
            const double b = pixelOffset[2] / 255.0;
            const double a = channels >= 4 ? pixelOffset[3] / 255.0 : 1.0;

            plutovg_set_rgba(pluto, r, g, b, a);
            plutovg_fill(pluto);
            plutovg_restore(pluto);
        }
    }

    stbi_image_free(image_data);

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

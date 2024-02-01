#include "plutovg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NUM_TEXTURES 4

int main(void) {
    // PlutoVG
    const int canvas_width = 900;
    const int canvas_height = 900;
    plutovg_surface_t *surface = plutovg_surface_create(canvas_width, canvas_height);
    plutovg_t *pluto = plutovg_create(surface);

    // Draw raster textures in 4 corners
    int positions[NUM_TEXTURES][2] = {{-1, -1},
                                      {1,  -1},
                                      {-1, 1},
                                      {1,  1}};
    for (int i = 0; i < NUM_TEXTURES; ++i)
    {
        char filename[PATH_MAX];
        sprintf(filename, "assets/texture%d.png", i + 1);

        int width, height, channels;
        unsigned char *image_data = stbi_load(filename, &width, &height, &channels, 0);

        if (!image_data) {
            fprintf(stderr, "Error loading textures: %s\n", stbi_failure_reason());
            return 1;
        }

        const int x = positions[i][0] * width / 2;
        const int y = positions[i][1] * height / 2;

        plutovg_save(pluto);
        plutovg_image(pluto, x, y, image_data, width, height, channels);
        plutovg_restore(pluto);

        stbi_image_free(image_data);
    }

    // Draw vector shapes on top of the textures
    const double center_x = canvas_width * 0.5;
    const double center_y = canvas_height * 0.5;
    const double face_radius = canvas_width * 0.25;
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

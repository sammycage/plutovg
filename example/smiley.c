#include "plutovg.h"

int main(void)
{
    const int width = 150;
    const int height = 150;

    plutovg_surface_t* surface = plutovg_surface_create(width, height);
    plutovg_t* pluto = plutovg_create(surface);

    double center_x = width * 0.5;
    double center_y = height * 0.5;
    double face_radius = 70;
    double eye_radius = 10;
    double mouth_radius = 50;
    double eye_offset_x = 25;
    double eye_offset_y = 20;
    double eye_x = center_x - eye_offset_x;
    double eye_y = center_y - eye_offset_y;

    const double pi = 3.14159265358979323846;

    plutovg_save(pluto);
    plutovg_arc(pluto, center_x, center_y, face_radius, 0, 2 * pi, 0);
    plutovg_set_rgb(pluto, 1, 1, 0);
    plutovg_fill_preserve(pluto);
    plutovg_set_rgb(pluto, 0, 0, 0);
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

    plutovg_surface_write_to_png(surface, "smiley.png");
    plutovg_surface_destroy(surface);
    plutovg_destroy(pluto);

    return 0;
}

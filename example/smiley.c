#include "plutovg.h"

#include <math.h>

int main()
{
    const int width = 150;
    const int height = 150;

    plutovg_surface_t* surface = plutovg_surface_create(width, height);
    plutovg_t* pluto = plutovg_create(surface);

    double center_x = width / 2;
    double center_y = height / 2;
    double radius = 70;
    double eye_radius = 10;
    double eye_offset_x = 25;
    double eye_offset_y = 20;
    double eye_x = center_x - eye_offset_x;
    double eye_y = center_y - eye_offset_y;

    plutovg_arc(pluto, center_x, center_y, radius, 0, 2 * M_PI, 0);
    plutovg_set_source_rgb(pluto, 1, 1, 0);
    plutovg_fill_preserve(pluto);
    plutovg_set_line_width(pluto, 5);
    plutovg_set_source_rgb(pluto, 0, 0, 0);
    plutovg_stroke(pluto);

    plutovg_arc(pluto, eye_x, eye_y, eye_radius, 0, 2 * M_PI, 0);
    plutovg_arc(pluto, center_x + eye_offset_x, eye_y, eye_radius, 0, 2 * M_PI, 0);
    plutovg_set_source_rgb(pluto, 0, 0, 0);
    plutovg_fill(pluto);

    plutovg_arc(pluto, center_x, center_y, 50, 0, M_PI, 0);
    plutovg_set_source_rgb(pluto, 0, 0, 0);
    plutovg_stroke(pluto);

    plutovg_surface_write_to_png(surface, "smiley.png");
    plutovg_surface_destroy(surface);
    plutovg_destroy(pluto);

    return 0;
}

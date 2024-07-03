[![Releases](https://img.shields.io/badge/Version-0.0.1-orange.svg)](https://github.com/sammycage/plutovg/releases)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/sammycage/plutovg/blob/main/LICENSE)
[![Build Status](https://github.com/sammycage/plutovg/actions/workflows/main.yml/badge.svg)](https://github.com/sammycage/plutovg/actions)
[![CodeFactor](https://www.codefactor.io/repository/github/sammycage/plutovg/badge)](https://www.codefactor.io/repository/github/sammycage/plutovg)

# PlutoVG
PlutoVG is a standalone 2D vector graphics library in C.

## Features
- Path Filling, Stroking and Dashing
- Soild, Gradient and Texture Paints
- Fonts and Texts
- Clipping and Compositing
- Transformations
- Images

## Example
```c
#include <plutovg.h>

int main(void)
{
    const int width = 150;
    const int height = 150;

    plutovg_surface_t* surface = plutovg_surface_create(width, height);
    plutovg_canvas_t* canvas = plutovg_canvas_create(surface);

    float center_x = width * 0.5;
    float center_y = height * 0.5;
    float face_radius = 70;
    float eye_radius = 10;
    float mouth_radius = 50;
    float eye_offset_x = 25;
    float eye_offset_y = 20;
    float eye_x = center_x - eye_offset_x;
    float eye_y = center_y - eye_offset_y;

    const float pi = 3.14159265358979323846;

    plutovg_canvas_save(canvas);
    plutovg_canvas_arc(canvas, center_x, center_y, face_radius, 0, 2 * pi, 0);
    plutovg_canvas_set_rgb(canvas, 1, 1, 0);
    plutovg_canvas_fill_preserve(canvas);
    plutovg_canvas_set_rgb(canvas, 0, 0, 0);
    plutovg_canvas_set_line_width(canvas, 5);
    plutovg_canvas_stroke(canvas);
    plutovg_canvas_restore(canvas);

    plutovg_canvas_save(canvas);
    plutovg_canvas_arc(canvas, eye_x, eye_y, eye_radius, 0, 2 * pi, 0);
    plutovg_canvas_arc(canvas, center_x + eye_offset_x, eye_y, eye_radius, 0, 2 * pi, 0);
    plutovg_canvas_set_rgb(canvas, 0, 0, 0);
    plutovg_canvas_fill(canvas);
    plutovg_canvas_restore(canvas);

    plutovg_canvas_save(canvas);
    plutovg_canvas_arc(canvas, center_x, center_y, mouth_radius, 0, pi, 0);
    plutovg_canvas_set_rgb(canvas, 0, 0, 0);
    plutovg_canvas_set_line_width(canvas, 5);
    plutovg_canvas_stroke(canvas);
    plutovg_canvas_restore(canvas);

    plutovg_surface_write_to_png(surface, "smiley.png");
    plutovg_surface_destroy(surface);
    plutovg_canvas_destroy(canvas);
    return 0;
}
```

output :

![smiley.png](smiley.png)

## Installation

Ensure you have [Meson](http://mesonbuild.com) and [Ninja](http://ninja-build.org) installed.

```bash
git clone https://github.com/sammycage/plutovg.git
cd plutovg
meson setup build
ninja -C build
ninja -C build install
```

## Projects using PlutoVG
- [LunaSVG](https://github.com/sammycage/lunasvg)
- [PlutoSVG](https://github.com/sammycage/plutosvg)

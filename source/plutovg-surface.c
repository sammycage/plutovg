#include "plutovg-private.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

static plutovg_surface_t* plutovg_surface_create_uninitialized(int width, int height)
{
    plutovg_surface_t* surface = malloc(width * height * 4 + sizeof(plutovg_surface_t));
    if(surface == NULL)
        return NULL;
    surface->ref_count = 1;
    surface->width = width;
    surface->height = height;
    surface->stride = width * 4;
    surface->data = (unsigned char*)(surface + 1);
    return surface;
}

plutovg_surface_t* plutovg_surface_create(int width, int height)
{
    plutovg_surface_t* surface = plutovg_surface_create_uninitialized(width, height);
    if(surface == NULL)
        return NULL;
    memset(surface->data, 0, surface->height * surface->stride);
    return surface;
}

plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    surface->ref_count = 1;
    surface->width = width;
    surface->height = height;
    surface->stride = stride;
    surface->data = data;
    return surface;
}

static plutovg_surface_t* plutovg_surface_load_from_image(stbi_uc* image, int width, int height)
{
    plutovg_surface_t* surface = plutovg_surface_create_uninitialized(width, height);
    if(surface)
        plutovg_convert_rgba_to_argb(surface->data, image, surface->width, surface->height, surface->stride);
    stbi_image_free(image);
    return surface;
}

plutovg_surface_t* plutovg_surface_load_from_image_file(const char* filename)
{
    int width, height, channels;
    stbi_uc* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    if(image == NULL)
        return NULL;
    return plutovg_surface_load_from_image(image, width, height);
}

plutovg_surface_t* plutovg_surface_load_from_image_data(const void* data, unsigned int length)
{
    int width, height, channels;
    stbi_uc* image = stbi_load_from_memory(data, length, &width, &height, &channels, STBI_rgb_alpha);
    if(image == NULL)
        return NULL;
    return plutovg_surface_load_from_image(image, width, height);
}

plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface)
{
    if(surface == NULL)
        return NULL;
    ++surface->ref_count;
    return surface;
}

void plutovg_surface_destroy(plutovg_surface_t* surface)
{
    if(surface == NULL)
        return;
    if(--surface->ref_count == 0) {
        free(surface);
    }
}

int plutovg_surface_get_reference_count(const plutovg_surface_t* surface)
{
    if(surface)
        return surface->ref_count;
    return 0;
}

unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface)
{
    return surface->data;
}

int plutovg_surface_get_width(const plutovg_surface_t* surface)
{
    return surface->width;
}

int plutovg_surface_get_height(const plutovg_surface_t* surface)
{
    return surface->height;
}

int plutovg_surface_get_stride(const plutovg_surface_t* surface)
{
    return surface->stride;
}

static void plutovg_surface_write_begin(const plutovg_surface_t* surface)
{
    plutovg_convert_argb_to_rgba(surface->data, surface->data, surface->width, surface->height, surface->stride);
}

static void plutovg_surface_write_end(const plutovg_surface_t* surface)
{
    plutovg_convert_rgba_to_argb(surface->data, surface->data, surface->width, surface->height, surface->stride);
}

bool plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename)
{
    plutovg_surface_write_begin(surface);
    int success = stbi_write_png(filename, surface->width, surface->height, 4, surface->data, surface->stride);
    plutovg_surface_write_end(surface);
    return success;
}

bool plutovg_surface_write_to_jpg(const plutovg_surface_t* surface, const char* filename, int quality)
{
    plutovg_surface_write_begin(surface);
    int success = stbi_write_jpg(filename, surface->width, surface->height, 4, surface->data, quality);
    plutovg_surface_write_end(surface);
    return success;
}

bool plutovg_surface_write_to_png_stream(const plutovg_surface_t* surface, plutovg_write_func_t write_func, void* closure)
{
    plutovg_surface_write_begin(surface);
    int success = stbi_write_png_to_func(write_func, closure, surface->width, surface->height, 4, surface->data, surface->stride);
    plutovg_surface_write_end(surface);
    return success;
}

bool plutovg_surface_write_to_jpg_stream(const plutovg_surface_t* surface, plutovg_write_func_t write_func, void* closure, int quality)
{
    plutovg_surface_write_begin(surface);
    int success = stbi_write_jpg_to_func(write_func, closure, surface->width, surface->height, 4, surface->data, quality);
    plutovg_surface_write_end(surface);
    return success;
}

void plutovg_convert_argb_to_rgba(unsigned char* dst, const unsigned char* src, int width, int height, int stride)
{
    for(int y = 0; y < height; y++) {
        uint32_t* dst_row = (uint32_t*)(dst + stride * y);
        uint32_t* src_row = (uint32_t*)(src + stride * y);
        for(int x = 0; x < width; x++) {
            uint32_t pixel = src_row[x];
            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = (pixel >> 0) & 0xFF;
            if(a != 0 && a != 255) {
                r = (r * 255) / a;
                g = (g * 255) / a;
                b = (b * 255) / a;
            }

            dst_row[x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

void plutovg_convert_rgba_to_argb(unsigned char* dst, const unsigned char* src, int width, int height, int stride)
{
    for(int y = 0; y < height; y++) {
        uint32_t* dst_row = (uint32_t*)(dst + stride * y);
        uint32_t* src_row = (uint32_t*)(src + stride * y);
        for(int x = 0; x < width; x++) {
            uint32_t pixel = src_row[x];
            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t r = (pixel >> 0) & 0xFF;
            if(a != 0 && a != 255) {
                r = (r * a) / 255;
                g = (g * a) / 255;
                b = (b * a) / 255;
            }

            dst_row[x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
}

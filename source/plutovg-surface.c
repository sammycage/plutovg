#include "plutovg-private.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

static plutovg_surface_t* plutovg_surface_create_uninitialized(int width, int height)
{
    plutovg_surface_t* surface = malloc(width * height * 4 + sizeof(plutovg_surface_t));
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

static void plutovg_surface_premultiply_rgba(const plutovg_surface_t* surface)
{
    unsigned char* data = plutovg_surface_get_data(surface);
    int width = plutovg_surface_get_width(surface);
    int height = plutovg_surface_get_height(surface);
    int stride = plutovg_surface_get_stride(surface);
    for(int y = 0; y < height; y++) {
        uint32_t* p = (uint32_t*)(data + stride * y);
        for(int x = 0; x < width; x++) {
            uint32_t a = (p[x] >> 24) & 0xFF;
            uint32_t b = (p[x] >> 16) & 0xFF;
            uint32_t g = (p[x] >> 8) & 0xFF;
            uint32_t r = (p[x] >> 0) & 0xFF;

            uint32_t pr = (r * a) / 255;
            uint32_t pg = (g * a) / 255;
            uint32_t pb = (b * a) / 255;
            p[x] = (a << 24) | (pr << 16) | (pg << 8) | pb;
        }
    }
}

static void plutovg_surface_unpremultiply_argb(const plutovg_surface_t* surface)
{
    unsigned char* data = plutovg_surface_get_data(surface);
    int width = plutovg_surface_get_width(surface);
    int height = plutovg_surface_get_height(surface);
    int stride = plutovg_surface_get_stride(surface);
    for(int y = 0; y < height; y++) {
        uint32_t* p = (uint32_t*)(data + stride * y);
        for(int x = 0; x < width; x++) {
            uint32_t a = (p[x] >> 24) & 0xFF;
            if(a == 0)
                continue;
            uint32_t pr = (p[x] >> 16) & 0xFF;
            uint32_t pg = (p[x] >> 8) & 0xFF;
            uint32_t pb = (p[x] >> 0) & 0xFF;

            uint32_t r = (pr * 255) / a;
            uint32_t g = (pg * 255) / a;
            uint32_t b = (pb * 255) / a;
            p[x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

static plutovg_surface_t* plutovg_surface_load_from_image(stbi_uc* image, int width, int height)
{
    plutovg_surface_t* surface = plutovg_surface_create_uninitialized(width, height);
    memcpy(surface->data, image, surface->height * surface->stride);
    plutovg_surface_premultiply_rgba(surface);
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

bool plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename)
{
    plutovg_surface_unpremultiply_argb(surface);
    int success = stbi_write_png(filename, surface->width, surface->height, 4, surface->data, surface->stride);
    plutovg_surface_premultiply_rgba(surface);
    return success;
}

bool plutovg_surface_write_to_jpg(const plutovg_surface_t* surface, const char* filename, int quality)
{
    plutovg_surface_unpremultiply_argb(surface);
    int success = stbi_write_jpg(filename, surface->width, surface->height, 4, surface->data, quality);
    plutovg_surface_premultiply_rgba(surface);
    return success;
}

bool plutovg_surface_write_to_png_stream(const plutovg_surface_t* surface, plutovg_stream_write_func_t write_func, void* closure)
{
    plutovg_surface_unpremultiply_argb(surface);
    int success = stbi_write_png_to_func(write_func, closure, surface->width, surface->height, 4, surface->data, surface->stride);
    plutovg_surface_premultiply_rgba(surface);
    return success;
}

bool plutovg_surface_write_to_jpg_stream(const plutovg_surface_t* surface, plutovg_stream_write_func_t write_func, void* closure, int quality)
{
    plutovg_surface_unpremultiply_argb(surface);
    int success = stbi_write_jpg_to_func(write_func, closure, surface->width, surface->height, 4, surface->data, quality);
    plutovg_surface_premultiply_rgba(surface);
    return success;
}

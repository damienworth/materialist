#ifndef MATERIALIST_FONT_HPP
#define MATERIALIST_FONT_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BBOX_H
#include FT_OUTLINE_H

namespace font {

struct rectangle {
    float left;
    float top;
    float right;
    float buttom;
};

struct cell_info {
    uint32_t point_offset;
    uint32_t offset;
    uint32_t count_x;
    uint32_t count_y;
};

struct glyph_instance {
    rectangle rect;
    uint32_t  index;
    float     sharpness;
};

struct host_glyph_info {
    rectangle bbox;
    float     advance;
};

struct device_glyph_info {
    rectangle bbox;
    cell_info info;
};

uint32_t align_uint32(uint32_t value, uint32_t alignment) noexcept;
void     load_font(vulkan::context& ctx, const char* font_face) noexcept;

} // namespace font

#endif // MATERIALIST_FONT_HPP


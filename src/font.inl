#define FT_CHECK(f, msg)                                                  \
    {                                                                     \
        auto error = f;                                                   \
        if (error) { ERROR("(FREETYPE) {}: error code {}", msg, error); } \
    }

namespace font {

void
load_font(vulkan::context& ctx, const char* font_face) noexcept
{
    FT_Library library;
    FT_CHECK(FT_Init_FreeType(&library), "failed to initialize freetype");

    FT_Face face;
    FT_CHECK(
        FT_New_Face(library, font_face, 0, &face),
        "failed to create new font face");
    FT_CHECK(
        FT_Set_Char_Size(face, 0, 1000 * 64, 96, 96),
        "failed to set character size");

    uint32_t total_points = 0;
    uint32_t total_cells  = 0;

    for (uint32_t i = 0; i < NUMBER_OF_GLYPHS; i++) {
        char c = ' ' + i;
        printf("%c", c);

        fd_Outline*       o   = &r->outlines[i];
        fd_HostGlyphInfo* hgi = &r->glyph_infos[i];

        FT_UInt glyph_index = FT_Get_Char_Index(face, c);
        FT_CHECK(
            FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING),
            "failed to load glyph");

        fd_outline_convert(&face->glyph->outline, o, c);

        hgi->bbox    = o->bbox;
        hgi->advance = face->glyph->metrics.horiAdvance / 64.0f;

        total_points += o->num_of_points;
        total_cells += o->cell_count_x * o->cell_count_y;
    }

    r->glyph_info_size   = sizeof(fd_DeviceGlyphInfo) * NUMBER_OF_GLYPHS;
    r->glyph_cells_size  = sizeof(uint32_t) * total_cells;
    r->glyph_points_size = sizeof(vec2) * total_points;

    uint32_t alignment =
        r->device_properties.limits.minStorageBufferOffsetAlignment;
    r->glyph_info_offset  = 0;
    r->glyph_cells_offset = align_uint32(r->glyph_info_size, alignment);
    r->glyph_points_offset =
        align_uint32(r->glyph_info_size + r->glyph_cells_size, alignment);
    r->glyph_data_size = r->glyph_points_offset + r->glyph_points_size;

    r->glyph_data = malloc(r->glyph_data_size);

    fd_DeviceGlyphInfo* device_glyph_infos =
        (fd_DeviceGlyphInfo*)((char*)r->glyph_data + r->glyph_info_offset);
    uint32_t* cells = (uint32_t*)((char*)r->glyph_data + r->glyph_cells_offset);
    vec2*     points = (vec2*)((char*)r->glyph_data + r->glyph_points_offset);

    uint32_t point_offset = 0;
    uint32_t cell_offset  = 0;

    for (uint32_t i = 0; i < NUMBER_OF_GLYPHS; i++) {
        fd_Outline*         o   = &r->outlines[i];
        fd_DeviceGlyphInfo* dgi = &device_glyph_infos[i];

        dgi->cell_info.cell_count_x = o->cell_count_x;
        dgi->cell_info.cell_count_y = o->cell_count_y;
        dgi->cell_info.point_offset = point_offset;
        dgi->cell_info.cell_offset  = cell_offset;
        dgi->bbox                   = o->bbox;

        uint32_t cell_count = o->cell_count_x * o->cell_count_y;
        memcpy(cells + cell_offset, o->cells, sizeof(uint32_t) * cell_count);
        memcpy(
            points + point_offset, o->points, sizeof(vec2) * o->num_of_points);

        // fd_outline_u16_points(o, &dgi->cbox, points + point_offset);

        point_offset += o->num_of_points;
        cell_offset += cell_count;
    }

    assert(point_offset == total_points);
    assert(cell_offset == total_cells);

    for (uint32_t i = 0; i < NUMBER_OF_GLYPHS; i++)
        fd_outline_destroy(&r->outlines[i]);

    FT_CHECK(FT_Done_Face(face), "failed to destroy font face");
    FT_CHECK(FT_Done_FreeType(library), "failed to deinitialize freetype");
}


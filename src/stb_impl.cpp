// Single compilation unit for stb implementations
// This ensures they are compiled exactly once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// stb_rect_pack must come BEFORE stb_truetype when both are used,
// because stb_truetype has its own internal rect packer that conflicts.
// By defining STB_RECT_PACK_IMPLEMENTATION first, stb_truetype will
// use the standalone version.
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

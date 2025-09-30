#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define QOI_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_write.h>
// If qoi header exists in include path, include it; harmless otherwise if removed later.
#ifdef __has_include
#  if __has_include(<qoi.h>)
#    include <qoi.h>
#  endif
#endif

#include "../include/GlobalsPCH.h"
// This source file exists solely so the build system can compile a single
// translation unit for the precompiled header specified by premake.

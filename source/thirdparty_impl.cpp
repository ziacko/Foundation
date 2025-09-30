// Single translation unit for third-party single-header library implementations.
// This prevents ODR/linker errors when using a precompiled header.

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

// Nothing else should go here.

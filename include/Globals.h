// Thin wrapper kept for backward compatibility. Real content moved to GlobalsPCH.h (the precompiled header).
#pragma once
#include "GlobalsPCH.h"

// If code elsewhere relied on IMPLEMENTATION defines being visible (it shouldn't),
// provide a diagnostic to help migrate.
#ifdef STB_IMAGE_IMPLEMENTATION
#error "STB_IMAGE_IMPLEMENTATION should not be defined outside thirdparty_impl.cpp"
#endif




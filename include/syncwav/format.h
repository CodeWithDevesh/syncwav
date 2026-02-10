#pragma once
#include "export.h"

namespace swav {

SWAV_API enum class AudioFormat {
  F32, // 32-bit float
  S16, // 16-bit signed int
  S32, // 32-bit signed int
  U8   // 8-bit unsigned int
};

SWAV_API inline const char *toCString(AudioFormat fmt) {
  using AF = swav::AudioFormat;
  switch (fmt) {
  case AF::F32:
    return "float_32";
  case AF::S16:
    return "int_16";
  case AF::S32:
    return "int_32";
  case AF::U8:
    return "uint_8";
  default:
    return "unknown";
  }
}

} // namespace swav

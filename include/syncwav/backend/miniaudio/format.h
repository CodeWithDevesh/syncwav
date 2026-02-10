#pragma once
#include "../../format.h"
#include "miniaudio.h"

namespace swav {
inline ma_format toMiniaudioFormat(swav::AudioFormat fmt) {
  using AF = swav::AudioFormat;
  switch (fmt) {
  case AF::F32:
    return ma_format_f32;
  case AF::S16:
    return ma_format_s16;
  case AF::S32:
    return ma_format_s32;
  case AF::U8:
    return ma_format_u8;
  default:
    return ma_format_unknown;
  }
}
} // namespace swav

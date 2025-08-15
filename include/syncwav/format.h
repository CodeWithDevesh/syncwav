#pragma once
#include "export.h"
#include "miniaudio.h"
extern "C" {
#include <libavutil/samplefmt.h>
}

namespace swav {
enum class SWAV_API AudioFormat {
  F32, // 32-bit float
  S16, // 16-bit signed int
  S32, // 32-bit signed int
  U8   // 8-bit unsigned int
};

SWAV_API inline AVSampleFormat toFFmpegFormat(swav::AudioFormat fmt) {
  using AF = swav::AudioFormat;
  switch (fmt) {
  case AF::F32:
    return AV_SAMPLE_FMT_FLT;
  case AF::S16:
    return AV_SAMPLE_FMT_S16;
  case AF::S32:
    return AV_SAMPLE_FMT_S32;
  case AF::U8:
    return AV_SAMPLE_FMT_U8;
  default:
    return AV_SAMPLE_FMT_NONE;
  }
}

SWAV_API inline ma_format toMiniaudioFormat(swav::AudioFormat fmt) {
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

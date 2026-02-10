#pragma once
#include "../../format.h"
extern "C" {
#include <libavutil/samplefmt.h>
}

namespace swav {

inline AVSampleFormat toFFmpegFormat(swav::AudioFormat fmt) {
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
} // namespace swav

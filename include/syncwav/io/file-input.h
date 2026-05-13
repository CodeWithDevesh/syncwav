#pragma once
#include "../export.h"
#include "input.h"
#include <atomic>
#include <thread>

#ifdef SWAV_USE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}
#endif

namespace swav {
class SWAV_API FileAudioInput : public Input {
public:
  FileAudioInput(Context &context, const char *filePath);
  ~FileAudioInput();
  void start() override;
  void stop() override;

private:
  void run();

private:
  std::thread thread;
  std::atomic<bool> running;

#ifdef SWAV_USE_FFMPEG
  int streamIndex;
  AVFormatContext *fmtCtx;
  AVCodecContext *codecCtx;
  SwrContext *swrCtx;
  AVPacket *packet;
  AVFrame *frame;
#endif
};
} // namespace swav

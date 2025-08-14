#pragma once
#include "../export.h"
#include "sinks.h"
#include <thread>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace swav {
class SWAV_API FileAudioInput : public Input {
public:
  FileAudioInput(const char *filePath);
  ~FileAudioInput();
  void start() override;
  void stop() override;

private:
  static void run(std::atomic<bool> *running, AVFormatContext *fmtCtx,
                  AVCodecContext *codecCtx, SwrContext *swrCtx,
                  AVPacket *packet, AVFrame *frame, int streamIndex);

private:
  std::thread *thread;
  int streamIndex;

  AVFormatContext *fmtCtx;
  AVCodecContext *codecCtx;
  SwrContext *swrCtx;
  AVPacket *packet;
  AVFrame *frame;
  std::atomic<bool> *running;
};
} // namespace swav

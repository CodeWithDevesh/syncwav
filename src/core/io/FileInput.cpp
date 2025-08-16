#include "syncwav/format.h"
#include <chrono>
#include <stdexcept>
#include <syncwav/io/FileInput.h>
#include <syncwav/log.h>
#include <thread>

namespace swav {

FileAudioInput::FileAudioInput(Context &context, const char *filePath)
    : Input("File Input", context) {
  fmtCtx = nullptr;
  swrCtx = nullptr;
  if (avformat_open_input(&fmtCtx, filePath, nullptr, nullptr) < 0)
    throw std::runtime_error("Failed to open the file");
  if (avformat_find_stream_info(fmtCtx, nullptr) < 0)
    throw std::runtime_error("Failed to find stream info");

  streamIndex =
      av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  AVStream *audioStream = fmtCtx->streams[streamIndex];

  AVCodecParameters *codecpar = audioStream->codecpar;
  const AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);
  codecCtx = avcodec_alloc_context3(decoder);
  avcodec_parameters_to_context(codecCtx, codecpar);
  avcodec_open2(codecCtx, decoder, nullptr);

  AVChannelLayout stereoLayout;
  av_channel_layout_default(&stereoLayout, context.channels);

  swr_alloc_set_opts2(&swrCtx, &stereoLayout, toFFmpegFormat(context.format),
                      context.sampleRate, &codecCtx->ch_layout,
                      codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr);

  swr_init(swrCtx);

  av_channel_layout_uninit(&stereoLayout);

  packet = av_packet_alloc();
  frame = av_frame_alloc();
  running = new std::atomic<bool>(false);
}

FileAudioInput::~FileAudioInput() {
  stop();
  av_frame_free(&frame);
  av_packet_free(&packet);
  swr_free(&swrCtx);
  avcodec_free_context(&codecCtx);
  avformat_close_input(&fmtCtx);
  delete (running);
}

void FileAudioInput::start() {
  if (*running)
    return;
  Input::start();
  *running = true;
  thread = new std::thread(run, running, fmtCtx, codecCtx, swrCtx, packet,
                           frame, streamIndex, std::ref(context));
}

void FileAudioInput::stop() {
  if (!*running)
    return;
  Input::stop();
  *running = false;
  thread->join();
  delete (thread);
}

void FileAudioInput::run(std::atomic<bool> *running, AVFormatContext *fmtCtx,
                         AVCodecContext *codecCtx, SwrContext *swrCtx,
                         AVPacket *packet, AVFrame *frame, int streamIndex,
                         Context &context) {
  while (*running && av_read_frame(fmtCtx, packet) >= 0) {
    if (packet->stream_index == streamIndex) {
      avcodec_send_packet(codecCtx, packet);
      while (*running && avcodec_receive_frame(codecCtx, frame) == 0) {
        uint8_t **outData = nullptr;
        int outLinesize;
        int outNbSamples = av_rescale_rnd(
            swr_get_delay(swrCtx, codecCtx->sample_rate) + frame->nb_samples,
            48000, codecCtx->sample_rate, AV_ROUND_UP);

        av_samples_alloc_array_and_samples(&outData, &outLinesize,
                                           context.channels, outNbSamples,
                                           toFFmpegFormat(context.format), 0);

        int samplesConverted = swr_convert(
            swrCtx, outData, outNbSamples,
            (const uint8_t **)frame->extended_data, frame->nb_samples);

        int bufferSize = av_samples_get_buffer_size(
            &outLinesize, context.channels, samplesConverted,
            toFFmpegFormat(context.format), 0);

        for (auto output : context.outputs) {
          while (output->availableWrite() < samplesConverted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (!*running)
              break;
          }
          if (!*running)
            break;
          output->write(outData[0], samplesConverted);
        }
        av_freep(&outData[0]);
        av_freep(&outData);
      }
    }
    av_packet_unref(packet);
  }
}
} // namespace swav

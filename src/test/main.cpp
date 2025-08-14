#include "syncwav/utils.h"
#include <cstring>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}
#include <syncwav/core.h>
#include <syncwav/queue.h>

struct Packet {
  float *data;
  int frames;
  int offset = 0;
  ~Packet() { free(data); };
};

void loopback(ma_device *pDevice, void *pOutput, const void *pInput,
              ma_uint32 frameCount) {
  float* output = static_cast<float*> (pOutput);
  auto queue =
      static_cast<swav::Queue<std::shared_ptr<Packet>> *>(pDevice->pUserData);
  while (frameCount > 0) {
    auto currentPacket = queue->front();
    if (currentPacket == nullptr) {
      memset(output, 0, ma_get_bytes_per_frame(ma_format_f32, 2) * frameCount);
      frameCount = 0;
      break;
    }

    Packet *packet = currentPacket->get();
    ma_uint32 framesAvl = packet->frames - packet->offset;
    ma_uint32 framesToWrite = std::min(framesAvl, frameCount);

    memcpy(output, packet->data + (packet->offset * pDevice->playback.channels),
           ma_get_bytes_per_frame(ma_format_f32, pDevice->playback.channels) *
               framesToWrite);
    frameCount -= framesToWrite;
    packet->offset += framesToWrite;
    if(packet->offset == packet->frames)
      queue->pop();
    output += framesToWrite * 2;
  }
  (void)pInput;
}

int main() {

  swav::Queue<std::shared_ptr<Packet>> queue(30);

  ma_device *device = new ma_device();
  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.pDeviceID = NULL;
  config.playback.format = ma_format_f32;
  config.playback.channels = 2;
  config.sampleRate = 48000;
  config.dataCallback = loopback;
  config.pUserData = &queue;

  ma_result result = ma_device_init(NULL, &config, device);
  if (result != MA_SUCCESS) {
    swav::log::e("Error while initializing local output device");
    std::exit(-1);
  }
  swav::log::i("Local Output successfully initialized");

  ma_device_start(device);

  const char *filename = "test.mp3";
  AVFormatContext *fmt_ctx = nullptr;
  if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr) < 0)
    return -1;
  if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    return -1;

  int stream_index =
      av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  AVStream *audio_stream = fmt_ctx->streams[stream_index];

  AVCodecParameters *codecpar = audio_stream->codecpar;
  const AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);
  AVCodecContext *codec_ctx = avcodec_alloc_context3(decoder);
  avcodec_parameters_to_context(codec_ctx, codecpar);
  avcodec_open2(codec_ctx, decoder, nullptr);

  SwrContext *swr_ctx = swr_alloc();
  AVChannelLayout stereoLayout;
  av_channel_layout_default(&stereoLayout, 2);

  swr_alloc_set_opts2(&swr_ctx, &stereoLayout, AV_SAMPLE_FMT_FLT, 48000,
                      &codec_ctx->ch_layout, codec_ctx->sample_fmt,
                      codec_ctx->sample_rate, 0, nullptr);
  swr_init(swr_ctx);

  AVPacket *pkt = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();

  while (av_read_frame(fmt_ctx, pkt) >= 0) {
    if (pkt->stream_index == stream_index) {
      avcodec_send_packet(codec_ctx, pkt);
      while (avcodec_receive_frame(codec_ctx, frame) == 0) {
        uint8_t *out_data[2];
        int out_linesize;
        int out_nb_samples = av_rescale_rnd(
            swr_get_delay(swr_ctx, codec_ctx->sample_rate) + frame->nb_samples,
            48000, codec_ctx->sample_rate, AV_ROUND_UP);

        av_samples_alloc(out_data, &out_linesize, 2, out_nb_samples,
                         AV_SAMPLE_FMT_FLT, 0);

        int samples_converted = swr_convert(
            swr_ctx, out_data, out_nb_samples,
            (const uint8_t **)frame->extended_data, frame->nb_samples);

        int buffer_size = av_samples_get_buffer_size(
            &out_linesize, 2, samples_converted, AV_SAMPLE_FMT_FLT, 0);
        swav::log::i("Buffer size is {}", buffer_size);
        swav::log::i("frames is {}", samples_converted);

        auto packet = std::make_shared<Packet>();
        packet->data = (float*) malloc(buffer_size);
        packet->frames = samples_converted;

        memcpy(packet->data, out_data[0], buffer_size);

        queue.push(packet);

        av_freep(&out_data[0]);
      }
    }
    av_packet_unref(pkt);
  }

  av_frame_free(&frame);
  av_packet_free(&pkt);
  swr_free(&swr_ctx);
  avcodec_free_context(&codec_ctx);
  avformat_close_input(&fmt_ctx);

  ma_device_stop(device);
  ma_device_uninit(device);

  return 0;
}

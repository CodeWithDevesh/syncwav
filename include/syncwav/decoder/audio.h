#pragma once
// #include "../log.h"
// #include "syncwav/io/sinks.h"
// #include <libavcodec/avcodec.h>
// #include <libavformat/avformat.h>
// #include <libavutil/opt.h>
// #include <libswresample/swresample.h>
// #include <memory>
//
// namespace swav {
// class Decoder {
// public:
//   int init(const char *file) {
//     if (initialized) {
//       return -1;
//     }
//
//     if (avformat_open_input(&fmtCtx, file, nullptr, nullptr) < 0)
//       return -1;
//     if (avformat_find_stream_info(fmtCtx, nullptr) < 0)
//       return -1;
//
//     streamIndex =
//         av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
//     AVStream *audioStream = fmtCtx->streams[streamIndex];
//
//     AVCodecParameters *codecpar = audioStream->codecpar;
//     const AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);
//     codecCtx = avcodec_alloc_context3(decoder);
//     avcodec_parameters_to_context(codecCtx, codecpar);
//     avcodec_open2(codecCtx, decoder, nullptr);
//
//     AVChannelLayout stereoLayout;
//     av_channel_layout_default(&stereoLayout, 2);
//
//     swr_alloc_set_opts2(&swrCtx, &stereoLayout, AV_SAMPLE_FMT_FLT, 48000,
//                         &codecCtx->ch_layout, codecCtx->sample_fmt,
//                         codecCtx->sample_rate, 0, nullptr);
//
//     swr_init(swrCtx);
//
//     av_channel_layout_uninit(&stereoLayout);
//
//     packet = av_packet_alloc();
//     frame = av_frame_alloc();
//
//     initialized = true;
//     return 1;
//   }
//
//   int uninit() {
//     av_frame_free(&frame);
//     av_packet_free(&packet);
//     swr_free(&swrCtx);
//     avcodec_free_context(&codecCtx);
//     avformat_close_input(&fmtCtx);
//     return 1;
//   }
//
//   bool decode(AudioPacket &audioPacket) {
//     while (av_read_frame(fmtCtx, packet) >= 0) {
//       if (packet->stream_index == streamIndex) {
//         avcodec_send_packet(codecCtx, packet);
//         while (avcodec_receive_frame(codecCtx, frame) == 0) {
//           uint8_t *out_data[2];
//           int out_linesize;
//           int out_nb_samples = av_rescale_rnd(
//               swr_get_delay(swrCtx, codecCtx->sample_rate) + frame->nb_samples,
//               48000, codecCtx->sample_rate, AV_ROUND_UP);
//
//           av_samples_alloc(out_data, &out_linesize, 2, out_nb_samples,
//                            AV_SAMPLE_FMT_FLT, 0);
//
//           int samples_converted = swr_convert(
//               swrCtx, out_data, out_nb_samples,
//               (const uint8_t **)frame->extended_data, frame->nb_samples);
//
//           int buffer_size = av_samples_get_buffer_size(
//               &out_linesize, 2, samples_converted, AV_SAMPLE_FMT_FLT, 0);
//           log::i("Buffer size is {}", buffer_size);
//           log::i("frames is {}", samples_converted);
//
//           audioPacket.data = std::make_shared<void>(malloc(buffer_size),
//                                                     [](void *p) { free(p); });
//           audioPacket.frames = samples_converted;
//           audioPacket.bytesPerFrame = buffer_size / samples_converted;
//
//           memcpy(audioPacket.data.get(), out_data[0], buffer_size);
//
//           av_freep(&out_data[0]);
//           av_packet_unref(packet);
//           return true;
//         }
//       }
//       av_packet_unref(packet);
//     }
//     return false;
//   }
//
// private:
//   bool initialized = false;
//
//   const char *file;
//   int streamIndex;
//
//   AVFormatContext *fmtCtx;
//   AVCodecContext *codecCtx;
//   SwrContext *swrCtx;
//   AVPacket *packet;
//   AVFrame *frame;
// };
// } // namespace swav

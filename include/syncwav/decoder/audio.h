#pragma once
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

namespace swav {
	class Decoder {
	public:
		int init(const char* file) {
			if (initialized) {
				return -1;
			}

			if (avformat_open_input(&fmtCtx, file, nullptr, nullptr) < 0) return -1;
			if (avformat_find_stream_info(fmtCtx, nullptr) < 0) return -1;

			int streamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
			AVStream* audioStream = fmtCtx->streams[streamIndex];

			AVCodecParameters* codecpar = audioStream->codecpar;
			const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
			codecCtx = avcodec_alloc_context3(decoder);
			avcodec_parameters_to_context(codecCtx, codecpar);
			avcodec_open2(codecCtx, decoder, nullptr);

			AVChannelLayout stereoLayout;
			av_channel_layout_default(&stereoLayout, 2);

			swr_alloc_set_opts2(&swrCtx, &stereoLayout, AV_SAMPLE_FMT_FLT, 48000, &codecCtx->ch_layout, codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr);

			swr_init(swrCtx);

			av_channel_layout_uninit(&stereoLayout);

			AVPacket* pkt = av_packet_alloc();
			AVFrame* frame = av_frame_alloc();

			initialized = true;
			return 1;
		}



	private:
		bool initialized = false;

		const char* file;

		AVFormatContext* fmtCtx;
		AVCodecContext* codecCtx;
		SwrContext* swrCtx;
		AVPacket* packet;
		AVFrame* frame;
	};
}
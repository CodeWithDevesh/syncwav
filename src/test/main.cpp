extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
}
#include <syncwav/core.h>

int main() {
	const char* filename = "test.mp3";
	AVFormatContext* fmt_ctx = nullptr;
	if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr) < 0) return -1;
	if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) return -1;

	int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	AVStream* audio_stream = fmt_ctx->streams[stream_index];

	AVCodecParameters* codecpar = audio_stream->codecpar;
	const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
	AVCodecContext* codec_ctx = avcodec_alloc_context3(decoder);
	avcodec_parameters_to_context(codec_ctx, codecpar);
	avcodec_open2(codec_ctx, decoder, nullptr);

	SwrContext* swr_ctx = swr_alloc();
	AVChannelLayout stereoLayout;
	av_channel_layout_default(&stereoLayout, 2);

	swr_alloc_set_opts2(&swr_ctx, &stereoLayout, AV_SAMPLE_FMT_FLT, 48000, &codec_ctx->ch_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate, 0, nullptr);
	swr_init(swr_ctx);

	AVPacket* pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	while (av_read_frame(fmt_ctx, pkt) >= 0) {
		if (pkt->stream_index == stream_index) {
			avcodec_send_packet(codec_ctx, pkt);
			while (avcodec_receive_frame(codec_ctx, frame) == 0) {
				uint8_t* out_data[2];
				int out_linesize;
				int out_nb_samples = av_rescale_rnd(
					swr_get_delay(swr_ctx, codec_ctx->sample_rate) + frame->nb_samples,
					48000, codec_ctx->sample_rate, AV_ROUND_UP);

				av_samples_alloc(out_data, &out_linesize, 2, out_nb_samples, AV_SAMPLE_FMT_FLT, 0);

				int samples_converted = swr_convert(swr_ctx, out_data, out_nb_samples,
					(const uint8_t**)frame->extended_data,
					frame->nb_samples);

				int buffer_size = av_samples_get_buffer_size(&out_linesize, 2, samples_converted,
					AV_SAMPLE_FMT_FLT, 0);
				swav::log::i("Buffer size is {}", buffer_size);

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

	return 0;
}

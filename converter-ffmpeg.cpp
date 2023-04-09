extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

int convertAVItoMP4(const char* input_filename, const char* output_filename) {
    int ret = 0;
    AVFormatContext *in_fmt_ctx = nullptr, *out_fmt_ctx = nullptr;
    AVCodec *video_codec = nullptr;
    AVStream *in_stream = nullptr, *out_stream = nullptr;
    AVCodecContext *in_codec_ctx = nullptr, *out_codec_ctx = nullptr;
    AVPacket pkt = { 0 };
    int stream_idx = 0;

    av_register_all();
    avcodec_register_all();

    if (avformat_open_input(&in_fmt_ctx, input_filename, nullptr, nullptr) < 0) {
        fprintf(stderr, "Error opening input file\n");
        return -1;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        fprintf(stderr, "Error finding stream information\n");
        return -1;
    }
    if (avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, output_filename) < 0) {
        fprintf(stderr, "Error allocating output context\n");
        return -1;
    }

    video_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!video_codec) {
        fprintf(stderr, "Error finding H.264 encoder\n");
        return -1;
    }

    in_stream = in_fmt_ctx->streams[stream_idx];
    in_codec_ctx = in_stream->codec;
    out_codec_ctx = avcodec_alloc_context3(video_codec);
    out_codec_ctx->codec_id = AV_CODEC_ID_H264;
    out_codec_ctx->width = in_codec_ctx->width;
    out_codec_ctx->height = in_codec_ctx->height;
    out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    out_codec_ctx->bit_rate = 1000000;

    if (avcodec_open2(out_codec_ctx, video_codec, nullptr) < 0) {
        fprintf(stderr, "Error opening codec\n");
        return -1;
    }

    out_stream = avformat_new_stream(out_fmt_ctx, video_codec);
    if (!out_stream) {
        fprintf(stderr, "Error creating output stream\n");
        return -1;
    }

    if (avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx) < 0) {
        fprintf(stderr, "Error setting codec parameters\n");
        return -1;
    }

    av_dump_format(out_fmt_ctx, 0, output_filename, 1);

    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&out_fmt_ctx->pb, output_filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Error opening output file\n");
            return -1;
        }
    }

    if (avformat_write_header(out_fmt_ctx, nullptr) < 0) {
        fprintf(stderr, "Error writing header\n");
        return -1;
    }

 
    while (av_read_frame(in_fmt_ctx, &pkt) >= 0) {
        AVFrame *frame = av_frame_alloc();
        int frame_decoded = 0;
        if (pkt.stream_index == stream_idx) {
            avcodec_send_packet(in_codec_ctx, &pkt);
            frame_decoded = avcodec_receive_frame(in_codec_ctx, frame);
            if (frame_decoded == AVERROR(EAGAIN) || frame_decoded == AVERROR_EOF) {
                av_frame_free(&frame);
                continue;
            } else if (frame_decoded < 0) {
                printf("Error decoding video frame\n");
                break;
            }

            /* Perform color correction on frame */
            color_correct(frame);

            /* Encode the frame and write to output */
            avcodec_send_frame(out_codec_ctx, frame);
            while (avcodec_receive_packet(out_codec_ctx, &pkt_out) == 0) {
                pkt_out.stream_index = out_stream->index;
                av_packet_rescale_ts(&pkt_out, out_codec_ctx->time_base, out_stream->time_base);
                av_interleaved_write_frame(out_fmt_ctx, &pkt_out);
                av_packet_unref(&pkt_out);
            }

            av_frame_free(&frame);
        }
        av_packet_unref(&pkt);
}


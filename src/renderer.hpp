#pragma once

#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/packet.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

using namespace godot;

class Renderer : public Resource {
	GDCLASS(Renderer, Resource);

private:
	static const int byte_per_pixel = 4;
	static constexpr AVChannelLayout chlayout_stereo = AV_CHANNEL_LAYOUT_STEREO;
	AVFormatContext *av_format_ctx = nullptr;
	const AVOutputFormat *av_out_format = nullptr;
	struct SwsContext *sws_ctx = nullptr;
	struct SwrContext *swr_ctx = nullptr;
	AVCodecContext *av_codec_ctx_video = nullptr, *av_codec_ctx_audio = nullptr;
	const AVCodec *av_codec_video = nullptr, *av_codec_audio = nullptr;
	FILE *output_file = nullptr;
	AVStream *av_stream_video, *av_stream_audio;
	AVPacket *av_packet_video = nullptr, *av_packet_audio = nullptr;
	AVFrame *av_frame_video = nullptr, *av_frame_audio = nullptr;
	char error_str[AV_ERROR_MAX_STRING_SIZE];
	int i = 0, x = 0, y = 0, response = 0;

	/* Render requirements */
	String file_path = "";
	AVCodecID av_codec_id_video, av_codec_id_audio;
	Vector2i resolution = Vector2i(1920, 1080);
	int framerate = 30, bit_rate = 400000, gop_size = 0;
	bool render_audio = false;

public:
	enum RENDERER_AUDIO_CODEC {
		A_MP3 = AV_CODEC_ID_MP3,
		A_AAC = AV_CODEC_ID_AAC,
		A_OPUS = AV_CODEC_ID_OPUS,
		A_VORBIS = AV_CODEC_ID_VORBIS,
		A_FLAC = AV_CODEC_ID_FLAC,
		A_PCM_UNCOMPRESSED = AV_CODEC_ID_PCM_S16LE,
		A_AC3 = AV_CODEC_ID_AC3,
		A_EAC3 = AV_CODEC_ID_EAC3,
		A_WAV = AV_CODEC_ID_WAVPACK,
		A_MP2 = AV_CODEC_ID_MP2,
	};
	enum RENDERER_VIDEO_CODEC {
		V_H264 = AV_CODEC_ID_H264,
		V_HEVC = AV_CODEC_ID_HEVC, // H265
		V_VP9 = AV_CODEC_ID_VP9,
		V_MPEG4 = AV_CODEC_ID_MPEG4,
		V_MPEG2 = AV_CODEC_ID_MPEG2VIDEO,
		V_MPEG1 = AV_CODEC_ID_MPEG1VIDEO,
		V_AV1 = AV_CODEC_ID_AV1,
		V_VP8 = AV_CODEC_ID_VP8,
		V_AMV = AV_CODEC_ID_AMV,
		V_GOPRO_CINEFORM = AV_CODEC_ID_CFHD,
		V_CINEPAK = AV_CODEC_ID_CINEPAK,
		V_DIRAC = AV_CODEC_ID_DIRAC,
		V_FLV1 = AV_CODEC_ID_FLV1,
		V_GIF = AV_CODEC_ID_GIF,
		V_H261 = AV_CODEC_ID_H261,
		V_H263 = AV_CODEC_ID_H263,
		V_H263p = AV_CODEC_ID_H263P,
		V_THEORA = AV_CODEC_ID_THEORA,
		V_WEBP = AV_CODEC_ID_WEBP,
		V_DNXHD = AV_CODEC_ID_DNXHD,
		V_MJPEG = AV_CODEC_ID_MJPEG,
		V_PRORES = AV_CODEC_ID_PRORES,
		V_RAWVIDEO = AV_CODEC_ID_RAWVIDEO,
		V_YUV4 = AV_CODEC_ID_YUV4,
	};
	enum RENDERER_SUBTITLE_CODEC {
		S_ASS = AV_CODEC_ID_ASS,
		S_MOV_TEXT = AV_CODEC_ID_MOV_TEXT,
		S_SUBRIP = AV_CODEC_ID_SUBRIP,
		S_TEXT = AV_CODEC_ID_TEXT,
		S_TTML = AV_CODEC_ID_TTML,
		S_WEBVTT = AV_CODEC_ID_WEBVTT,
		S_XSUB = AV_CODEC_ID_XSUB,
	};

	~Renderer();

	static Dictionary get_supported_codecs();
	static bool is_video_codec_supported(RENDERER_VIDEO_CODEC a_codec);
	static bool is_audio_codec_supported(RENDERER_AUDIO_CODEC a_codec);

	inline void set_output_file_path(String a_file_path) { file_path = a_file_path; }
	inline String get_output_file_path(String a_file_path) { return file_path; }

	inline void set_video_codec(RENDERER_VIDEO_CODEC a_video_codec) { av_codec_id_video = static_cast<AVCodecID>(a_video_codec); }
	inline RENDERER_VIDEO_CODEC get_video_codec() { return static_cast<RENDERER_VIDEO_CODEC>(av_codec_id_video); }

	inline void set_audio_codec(RENDERER_AUDIO_CODEC a_audio_codec) { av_codec_id_audio = static_cast<AVCodecID>(a_audio_codec); }
	inline RENDERER_AUDIO_CODEC get_audio_codec() { return static_cast<RENDERER_AUDIO_CODEC>(av_codec_id_audio); }

	inline void set_resolution(Vector2i a_resolution) { resolution = a_resolution; }
	inline Vector2i get_resolution() { return resolution; }

	inline void set_framerate(int a_framerate) { framerate = a_framerate; }
	inline int get_framerate() { return framerate; }

	inline void set_bit_rate(int a_bit_rate) { bit_rate = a_bit_rate; }
	inline int get_bit_rate() { return bit_rate; }

	inline void set_gop_size(int a_gop_size) { gop_size = a_gop_size; }
	inline int get_gop_size() { return gop_size; }

	inline void set_render_audio(bool a_value) { render_audio = a_value; }
	inline bool get_render_audio() { return render_audio; }

	inline char *get_av_error() { return av_make_error_string(error_str, AV_ERROR_MAX_STRING_SIZE, response); }

	bool ready_check();

	int open();
	int send_frame(Ref<Image> a_frame_image);
	int send_audio(Ref<AudioStreamWAV> a_wav);
	int close();

protected:
	static inline void _bind_methods() {
		/* AUDIO CODEC ENUMS */
		BIND_ENUM_CONSTANT(A_MP3);
		BIND_ENUM_CONSTANT(A_AAC);
		BIND_ENUM_CONSTANT(A_OPUS);
		BIND_ENUM_CONSTANT(A_VORBIS);
		BIND_ENUM_CONSTANT(A_FLAC);
		BIND_ENUM_CONSTANT(A_PCM_UNCOMPRESSED);
		BIND_ENUM_CONSTANT(A_AC3);
		BIND_ENUM_CONSTANT(A_EAC3);
		BIND_ENUM_CONSTANT(A_WAV);
		BIND_ENUM_CONSTANT(A_MP2);

		/* VIDEO CODEC ENUMS */
		BIND_ENUM_CONSTANT(V_H264);
		BIND_ENUM_CONSTANT(V_HEVC);
		BIND_ENUM_CONSTANT(V_VP9);
		BIND_ENUM_CONSTANT(V_MPEG4);
		BIND_ENUM_CONSTANT(V_MPEG2);
		BIND_ENUM_CONSTANT(V_MPEG1);
		BIND_ENUM_CONSTANT(V_AV1);
		BIND_ENUM_CONSTANT(V_VP8);
		BIND_ENUM_CONSTANT(V_AMV);
		BIND_ENUM_CONSTANT(V_GOPRO_CINEFORM);
		BIND_ENUM_CONSTANT(V_CINEPAK);
		BIND_ENUM_CONSTANT(V_DIRAC);
		BIND_ENUM_CONSTANT(V_FLV1);
		BIND_ENUM_CONSTANT(V_GIF);
		BIND_ENUM_CONSTANT(V_H261);
		BIND_ENUM_CONSTANT(V_H263);
		BIND_ENUM_CONSTANT(V_H263p);
		BIND_ENUM_CONSTANT(V_THEORA);
		BIND_ENUM_CONSTANT(V_WEBP);
		BIND_ENUM_CONSTANT(V_DNXHD);
		BIND_ENUM_CONSTANT(V_MJPEG);
		BIND_ENUM_CONSTANT(V_PRORES);
		BIND_ENUM_CONSTANT(V_RAWVIDEO);
		BIND_ENUM_CONSTANT(V_YUV4);

		/* SUBTITLE CODEC ENUMS */
		BIND_ENUM_CONSTANT(S_ASS);
		BIND_ENUM_CONSTANT(S_MOV_TEXT);
		BIND_ENUM_CONSTANT(S_SUBRIP);
		BIND_ENUM_CONSTANT(S_TEXT);
		BIND_ENUM_CONSTANT(S_TTML);
		BIND_ENUM_CONSTANT(S_WEBVTT);
		BIND_ENUM_CONSTANT(S_XSUB);

		ClassDB::bind_static_method("Renderer", D_METHOD("get_supported_codecs"), &Renderer::get_supported_codecs);
		ClassDB::bind_static_method("Renderer", D_METHOD("is_video_codec_supported", "a_video_codec"), &Renderer::is_video_codec_supported);
		ClassDB::bind_static_method("Renderer", D_METHOD("is_audio_codec_supported", "a_audio_codec"), &Renderer::is_audio_codec_supported);

		ClassDB::bind_method(D_METHOD("set_output_file_path", "a_file_path"), &Renderer::set_output_file_path);
		ClassDB::bind_method(D_METHOD("get_output_file_path"), &Renderer::get_output_file_path);

		ClassDB::bind_method(D_METHOD("set_video_codec", "a_video_codec"), &Renderer::set_video_codec);
		ClassDB::bind_method(D_METHOD("get_video_codec"), &Renderer::get_video_codec);

		ClassDB::bind_method(D_METHOD("set_audio_codec", "a_audio_codec"), &Renderer::set_audio_codec);
		ClassDB::bind_method(D_METHOD("get_audio_codec"), &Renderer::get_audio_codec);

		ClassDB::bind_method(D_METHOD("set_resolution", "a_resolution"), &Renderer::set_resolution);
		ClassDB::bind_method(D_METHOD("get_resolution"), &Renderer::get_resolution);

		ClassDB::bind_method(D_METHOD("set_framerate", "a_framerate"), &Renderer::set_framerate);
		ClassDB::bind_method(D_METHOD("get_framerate"), &Renderer::get_framerate);

		ClassDB::bind_method(D_METHOD("set_bit_rate", "a_bit_rate"), &Renderer::set_bit_rate);
		ClassDB::bind_method(D_METHOD("get_bit_rate"), &Renderer::get_bit_rate);

		ClassDB::bind_method(D_METHOD("set_gop_size", "a_gop_size"), &Renderer::set_gop_size);
		ClassDB::bind_method(D_METHOD("get_gop_size"), &Renderer::get_gop_size);

		ClassDB::bind_method(D_METHOD("set_render_audio", "a_value"), &Renderer::set_render_audio);
		ClassDB::bind_method(D_METHOD("get_render_audio"), &Renderer::get_render_audio);

		ClassDB::bind_method(D_METHOD("ready_check"), &Renderer::ready_check);

		ClassDB::bind_method(D_METHOD("open"), &Renderer::open);
		ClassDB::bind_method(D_METHOD("send_frame", "a_frame_image"), &Renderer::send_frame);
		ClassDB::bind_method(D_METHOD("send_audio", "a_wav"), &Renderer::send_audio);
		ClassDB::bind_method(D_METHOD("close"), &Renderer::close);
	}
};

VARIANT_ENUM_CAST(Renderer::RENDERER_VIDEO_CODEC);
VARIANT_ENUM_CAST(Renderer::RENDERER_AUDIO_CODEC);
VARIANT_ENUM_CAST(Renderer::RENDERER_SUBTITLE_CODEC);

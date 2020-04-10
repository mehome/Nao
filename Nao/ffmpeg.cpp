#include "ffmpeg.h"

#include "utils.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace detail {
    static int read(void* opaque, uint8_t* buf, int buf_size) {
        binary_istream* stream = static_cast<binary_istream*>(opaque);
        if (!stream->good()) {
            return -1;
        }

        if (stream->eof()) {
            return AVERROR_EOF;
        }

        stream->read(buf, buf_size);

        return utils::narrow<int>(stream->gcount());
    }

    static int64_t seek(void* opaque, int64_t offset, int whence) {
        binary_istream* stream = static_cast<binary_istream*>(opaque);

        if (whence == AVSEEK_SIZE) {
            auto cur = stream->tellg();
            stream->seekg(0, std::ios::end);
            auto size = stream->tellg();
            stream->seekg(cur, std::ios::beg);

            return size;
        }

        stream->seekg(offset, whence);
        return stream->tellg();
    }
}

namespace ffmpeg {
    inline namespace generic {
        frame::frame() : _frame { av_frame_alloc() } {
            ASSERT(_frame);
        }

        frame::~frame() {
            av_frame_free(&_frame);
        }

        int64_t frame::samples() const {
            return _frame->nb_samples;
        }

        int64_t frame::channels() const {
            return _frame->channels;
        }

        const char* frame::data(size_t index) const {
            return reinterpret_cast<char*>(_frame->buf[index]->data);
        }

        size_t frame::size(size_t index) const {
            return _frame->buf[index]->size;
        }

        frame::operator AVFrame* () const noexcept {
            return _frame;
        }

        packet::packet() : _packet { av_packet_alloc() } {
            ASSERT(_packet);
        }

        packet::~packet() {
            av_packet_free(&_packet);
        }

        int packet::stream_index() const {
            return _packet->stream_index;
        }

        packet::operator AVPacket* () const noexcept {
            return _packet;
        }
    }

    namespace avio {
        io_context::io_context(const istream_ptr& stream)
            : _stream { stream }
            , _ctx { avio_alloc_context(static_cast<unsigned char*>(av_malloc(buffer_size)), buffer_size,
                0, _stream.get(), detail::read, nullptr, detail::seek) } {

        }

        io_context::~io_context() {
            av_freep(&_ctx->buffer);
            avio_context_free(&_ctx);
        }

        AVIOContext* io_context::ctx() const {
            return _ctx;
        }
    }

    namespace avformat {
        stream::stream(AVStream* stream) : _stream { stream }{
            
        }

        AVMediaType stream::type() const {
            return _stream->codecpar->codec_type;
        }

        AVCodecID stream::id() const {
            return _stream->codecpar->codec_id;
        }

        int stream::index() const {
            return _stream->index;
        }

        int64_t stream::sample_rate() const {
            return _stream->codecpar->sample_rate;
        }

        int64_t stream::channels() const {
            return _stream->codecpar->channels;
        }

        std::chrono::nanoseconds stream::duration() const {
            return std::chrono::nanoseconds { static_cast<int64_t>(round(1e9 * _stream->duration * time_base())) };
        }

        double stream::time_base() const {
            return _stream->time_base.num / static_cast<double>(_stream->time_base.den);
        }

        AVCodecParameters* stream::params() const {
            return _stream->codecpar;
        }

        stream::operator bool() const {
            return _stream;
        }

        context::context(const istream_ptr& stream, const std::string& path)
            : _ctx { avformat_alloc_context() }
            , _stream { stream }, _ioctx { stream } {

            ASSERT(_ctx);
            _ctx->pb = _ioctx.ctx();
            _ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

            ASSERT(avformat_open_input(&_ctx, path.c_str(), nullptr, nullptr) == 0);

            _streams = std::vector<avformat::stream> { _ctx->streams, _ctx->streams + _ctx->nb_streams };
        }

        context::~context() {
            avformat_close_input(&_ctx);
            avformat_free_context(_ctx);
        }

        AVFormatContext* context::ctx() const {
            return _ctx;
        }

        size_t context::stream_count() const {
            return _ctx->nb_streams;
        }

        const std::vector<stream>& context::streams() const {
            return _streams;
        }

        bool context::read_frame(packet& pkt) const {
            return av_read_frame(_ctx, pkt) == 0;
        }

        bool context::read_frame(packet& pkt, int index) const {
            do {
                if (!read_frame(pkt)) {
                    return false;
                }
            } while (pkt.stream_index() != index);

            return true;
        }

        bool context::seek(std::chrono::nanoseconds pos, int index) {
            return av_seek_frame(_ctx, index,
                pos / std::chrono::nanoseconds { static_cast<int64_t>(_streams[index].time_base() * 1e9) },
                AVSEEK_FLAG_ANY) >= 0;
        }

    }

    namespace avcodec {
        codec::codec(AVCodecID id) : _codec { avcodec_find_decoder(id) } {
            ASSERT(_codec);
        }

        AVCodec* codec::ctx() const {
            return _codec;
        }

        std::string codec::name() const {
            return _codec->name;
        }

        std::string codec::long_name() const {
            return _codec->long_name;
        }


        context::context(const codec& codec) : _ctx { avcodec_alloc_context3(codec.ctx()) } {
            ASSERT(_ctx);
        }

        context::context() : _ctx { avcodec_alloc_context3(nullptr) } {
            ASSERT(_ctx);
        }

        context::context(context&& other) noexcept : _ctx { other._ctx } {
            other._ctx = nullptr;
        }

        context& context::operator=(context&& other) noexcept{
            _ctx = other._ctx;
            other._ctx = nullptr;
            return *this;
        }


        context::~context() {
            avcodec_free_context(&_ctx);
        }

        bool context::parameters_to_context(const avformat::stream& stream) const {
            return avcodec_parameters_to_context(_ctx, stream.params()) >= 0;
        }

        bool context::open(const codec& codec) const {
            return avcodec_open2(_ctx, codec.ctx(), nullptr) == 0;
        }

        AVSampleFormat context::sample_format() const {
            return _ctx->sample_fmt;
        }

        int context::decode(const packet& pkt, frame& frame) const {
            int res = avcodec_send_packet(_ctx, pkt);
            if (res != 0) {
                return res;
            }

            return avcodec_receive_frame(_ctx, frame);
        }

        AVCodecContext* context::ctx() const {
            return _ctx;
        }
    }
}
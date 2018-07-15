/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <qcoreapplication.h>

#include "ff_tools.h"

#include "ff_decoder.h"

class FFDecoderContext
{
public:
    AVFormatContext *format_context=nullptr;
    AVStream *stream=nullptr;
    AVCodec *codec=nullptr;
    AVCodecContext *codec_context=nullptr;
    AVFrame *frame=nullptr;
    SwrContext *convert_context=nullptr;
    AVPacket packet;
    int64_t duration=0;
    int64_t pos=0;
    bool ended=false;

    class Bitrate {
        std::list <uint64_t> accum;
        size_t window_size=128;

    public:
        bool isWindowSizeSet() const {
            return window_size!=0;
        }

        void setWindowSize(size_t size) {
            window_size=size;
        }

        void push(uint64_t value) {
            accum.push_back(value);

            while(accum.size()>window_size)
                accum.pop_front();
        }

        void reset() {
            window_size=0;
            accum.clear();
        }

        uint64_t calc() const {
            if(accum.empty())
                return 0;

            return std::accumulate(accum.begin(), accum.end(), 0.)/(double)accum.size();
        }

    } bitrate;
};

FFDecoder::FFDecoder(QObject *parent)
    : QObject(parent)
{
    context=new FFDecoderContext();

    av_init_packet(&context->packet);

    context->frame=av_frame_alloc();
}

FFDecoder::~FFDecoder()
{
    close();

    av_frame_unref(context->frame);

    delete context;
}

bool FFDecoder::open(const QString &filename)
{
    // qInfo() << "FFDecoder::open:" << filename;

    close();

    context->format_context=avformat_alloc_context();

    int ret;


    ret=avformat_open_input(&context->format_context, filename.toUtf8().data(), 0, 0);

    if(ret<0) {
        qWarning() << "FFDecoder::open: avformat_open_input err" << ffErrorString(ret);
        goto error_exit;
    }


    ret=avformat_find_stream_info(context->format_context, nullptr);

    if(ret<0) {
        qCritical() << "FFDecoder::open: avformat_find_stream_info err" << ffErrorString(ret);
        goto error_exit;
    }


    for(unsigned int i=0; i<context->format_context->nb_streams; ++i) {
        if(context->format_context->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
            context->stream=context->format_context->streams[i];

            if(context->stream->duration!=AV_NOPTS_VALUE) {
                context->duration=context->stream->duration*av_q2d(context->stream->time_base)*1000;

            } else if(context->format_context->duration!=AV_NOPTS_VALUE) {
                qWarning() << "FFDecoder::open: duration from format context";
                context->duration=context->format_context->duration*av_q2d(AV_TIME_BASE_Q)*1000;

            } else {
                qWarning() << "FFDecoder::open: no duration";
                context->duration=0;
            }

            break;
        }
    }

    if(!context->stream) {
        goto error_exit;
    }

    if(context->stream->codecpar->codec_id==AV_CODEC_ID_OPUS)
        context->codec=avcodec_find_decoder_by_name("libopus");

    if(context->stream->codecpar->codec_id==AV_CODEC_ID_VORBIS)
        context->codec=avcodec_find_decoder_by_name("libvorbis");

    if(context->stream->codecpar->codec_id==AV_CODEC_ID_AAC)
        context->codec=avcodec_find_decoder_by_name("libfdk_aac");

    if(!context->codec) {
        qWarning() << "decoder_by_name not found";
        context->codec=avcodec_find_decoder(context->stream->codecpar->codec_id);
    }

    if(!context->codec) {
        qCritical() << "FFDecoder::open: avcodec_find_decoder err";
        goto error_exit;
    }


    context->codec_context=avcodec_alloc_context3(context->codec);


    ret=avcodec_parameters_to_context(context->codec_context, context->stream->codecpar);

    if(ret<0) {
        qCritical() << "FFDecoder::open: avcodec_parameters_to_context err" << ffErrorString(ret);
        goto error_exit;
    }


    ret=avcodec_open2(context->codec_context, context->codec, nullptr);

    if(ret<0) {
        qCritical() << "FFDecoder::open: avcodec_open2 err" << ffErrorString(ret);
        goto error_exit;
    }


    context->convert_context=swr_alloc();

    if(!context->convert_context) {
        qCritical() << "FFDecoder::open: swr_alloc err";
        goto error_exit;
    }

    // av_opt_set_channel_layout(context->convert_context, "in_channel_layout", context->codec_context->channel_layout, 0);
    av_opt_set_channel_layout(context->convert_context, "in_channel_layout", av_get_default_channel_layout(context->codec_context->channels), 0);

    av_opt_set_channel_layout(context->convert_context, "out_channel_layout", AV_CH_LAYOUT_STEREO,  0);
    av_opt_set_int(context->convert_context,"in_sample_rate", context->codec_context->sample_rate, 0);
    av_opt_set_int(context->convert_context, "out_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(context->convert_context, "in_sample_fmt", context->codec_context->sample_fmt, 0);
    av_opt_set_sample_fmt(context->convert_context, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);


    ret=swr_init(context->convert_context);

    if(ret<0) {
        qCritical() << "FFDecoder::open: swr_init err" << ffErrorString(ret);
        goto error_exit;
    }


    if(strcmp(context->format_context->iformat->name, avcodec_get_name(context->codec->id))==0)
        stats.format=QString(context->format_context->iformat->name);

    else
        stats.format=QString("%1/%2").arg(context->format_context->iformat->name).arg(avcodec_get_name(context->codec->id));

    stats.duration=context->duration;
    stats.samplerate=context->codec_context->sample_rate;


    emit openOk();

    // qInfo() << "FFDecoder::open: ok";

    return true;

error_exit:

    emit openError();

    close();

    return false;
}

void FFDecoder::close()
{
    if(context->codec_context) {
        avcodec_free_context(&context->codec_context);
        context->codec_context=nullptr;
    }

    if(context->format_context) {
        avformat_close_input(&context->format_context);
        avformat_free_context(context->format_context);
        context->format_context=nullptr;
    }

    if(context->convert_context) {
        swr_free(&context->convert_context);
        context->convert_context=nullptr;
    }

    context->codec=nullptr;
    context->stream=nullptr;

    context->bitrate.reset();

    context->duration=0;
    context->pos=0;
    context->ended=false;

    stats.last_update_time=0;
}

int64_t FFDecoder::pos() const
{
    return context->pos;
}

int64_t FFDecoder::duration() const
{
    return context->duration;
}

QByteArray FFDecoder::read()
{
    QByteArray ba;

    while(true) {
        if(!context->format_context)
            return ba;

        if(av_read_frame(context->format_context, &context->packet)>=0) {
            if(context->packet.stream_index==context->stream->index) {
                if(avcodec_send_packet(context->codec_context, &context->packet)==0) {
                    double packet_bitrate=context->packet.size*(1000/(av_q2d(context->stream->time_base)*context->packet.duration*1000))*8;
                    context->bitrate.push(packet_bitrate);

                    if(av_gettime() - stats.last_update_time>=1000*1000) {
                        stats.pos=context->pos;
                        stats.bitrate=context->bitrate.calc();
                        stats.last_update_time=av_gettime();

                        emit statsChanged(stats);
                    }

                    if(!context->bitrate.isWindowSizeSet())
                        context->bitrate.setWindowSize((1000./(av_q2d(context->stream->time_base)*context->packet.duration*1000.))*3.); // window size for 3 sec

                    while(true) {
                        int ret=avcodec_receive_frame(context->codec_context, context->frame);

                        if(ret<0 && ret!=AVERROR(EAGAIN)) {
                            qCritical() << "avcodec_receive_frame err" << ffErrorString(ret);
                        }

                        if(ret<0) {
                            break;
                        }

                        if(ret>=0) {
                            uint8_t *out_samples;

                            int out_num_samples=av_rescale_rnd(swr_get_delay(context->convert_context, context->codec_context->sample_rate)
                                                               + context->frame->nb_samples, 48000, context->codec_context->sample_rate, AV_ROUND_UP);

                            av_samples_alloc(&out_samples, nullptr, 2, out_num_samples, AV_SAMPLE_FMT_S16, 1);

                            out_num_samples=swr_convert(context->convert_context, &out_samples, out_num_samples,
                                                        (const uint8_t**)&context->frame->extended_data[0], context->frame->nb_samples);

                            QByteArray ba_readed((char*)out_samples, av_samples_get_buffer_size(nullptr, 2, out_num_samples, AV_SAMPLE_FMT_S16, 1));


                            av_freep(&out_samples);

                            av_packet_unref(&context->packet);


                            if(context->frame->pts!=AV_NOPTS_VALUE) {
                                context->pos=av_q2d(context->stream->time_base)*context->frame->pts*1000;
                            }


                            ba+=ba_readed;
                        }
                    }
                }

                av_packet_unref(&context->packet);
                break;

            } else {
                av_packet_unref(&context->packet);
            }

        } else {
            if(!context->ended) {
                qInfo() << "FFDecoder::read: stream ended";
                emit streamEnded();
            }

            context->ended=true;
            break;
            qApp->processEvents();
        }
    }

    return ba;
}

void FFDecoder::seek(int64_t pos)
{
    if(!context->format_context)
        return;

    int ret=av_seek_frame(context->format_context, context->stream->index, pos/1000./av_q2d(context->stream->time_base), AVSEEK_FLAG_BACKWARD);

    if(ret<0) {
        qCritical() << "FFDecoder::seek: av_seek_frame err";
        return;
    }

    avcodec_flush_buffers(context->codec_context);

    stats.last_update_time=0;
}

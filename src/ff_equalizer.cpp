/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include "ff_equalizer.h"

struct FFEqualizerContext
{
    std::atomic <bool> enabled;

    int sample_rate=0;
    int format=0;
    int channels=0;

    EQParams params;

    AVFilterGraph *filter_graph=nullptr;
    AVFilterContext *abuffer_ctx=nullptr;
    AVFilterContext *abuffersink_ctx=nullptr;

    AVFrame *frame;

    std::vector <AVFilterContext*> eq;
};

FFEqualizer::FFEqualizer()
    : d(new FFEqualizerContext())
{
    d->enabled=false;
}

FFEqualizer::~FFEqualizer()
{
    free();
}

int FFEqualizer::init()
{
    free();

    qInfo() << "FFEqualizer::init: d->params.size" << d->params.size();

    if(d->params.size()<1)
        return AVERROR_INVALIDDATA;

    int ret;

    d->filter_graph=avfilter_graph_alloc();

    if(!d->filter_graph) {
        qCritical() << "Unable to create filter graph";
        return AVERROR(ENOMEM);
    }

    char ch_layout[64]={};

    av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, av_get_default_channel_layout(d->channels));


    QString args=QString("sample_rate=%1:sample_fmt=%2:channels=%3:channel_layout=%4")
            .arg(d->sample_rate)
            .arg(av_get_sample_fmt_name((AVSampleFormat)d->format))
            .arg(d->channels)
            .arg(ch_layout);

    ret=avfilter_graph_create_filter(&d->abuffer_ctx, avfilter_get_by_name("abuffer"), "src",
                                     args.toLatin1().constData(), NULL, d->filter_graph);


    if(!d->abuffer_ctx) {
        qCritical() << "Could not allocate the abuffer instance";
        free();
        return AVERROR(ENOMEM);
    }

    if(ret<0) {
        qCritical() << "Could not initialize the abuffer filter" << ffErrorString(ret);
        free();
        return ret;
    }

    if(ret<0) {
        qCritical() << "Could not initialize the equalizer filter" << ffErrorString(ret);
        free();
        return ret;
    }


    d->eq.resize(d->params.size());

    for(size_t i=0; i<d->params.size(); ++i) {
        args=QString("g=%1:f=%2:t=h:w=%3")
                .arg(d->params[i].gain)
                .arg(d->params[i].frequency)
                .arg(d->params[i].width);

        ret=avfilter_graph_create_filter(&d->eq[i], avfilter_get_by_name("equalizer"), "equalizer", args.toLatin1().constData(), NULL, d->filter_graph);

        if(ret<0) {
            qCritical() << "Could not initialize the equalizer filter" << i << ffErrorString(ret);
            return ret;
        }
    }

    ret=avfilter_graph_create_filter(&d->abuffersink_ctx, avfilter_get_by_name("abuffersink"), "sink", NULL, NULL, d->filter_graph);

    if(!d->abuffersink_ctx) {
        qCritical() << "Could not allocate the abuffersink instance";
        free();
        return AVERROR(ENOMEM);
    }

    static const int out_sample_fmts[]={ d->format, -1 };
    static const int64_t out_channel_layouts[]={ av_get_default_channel_layout(d->channels), -1 };
    static const int out_sample_rates[]={ d->sample_rate, -1 };

    ret=av_opt_set_int_list(d->abuffersink_ctx, "sample_fmts", out_sample_fmts, -1, AV_OPT_SEARCH_CHILDREN);
    ret=av_opt_set_int_list(d->abuffersink_ctx, "channel_layouts", out_channel_layouts, -1, AV_OPT_SEARCH_CHILDREN);
    ret=av_opt_set_int_list(d->abuffersink_ctx, "sample_rates", out_sample_rates, -1, AV_OPT_SEARCH_CHILDREN);

    //

    ret=avfilter_link(d->abuffer_ctx, 0, d->eq[0], 0);

    if(ret<0) {
        qCritical() << "Error connecting filters abuffer-eq0" << ffErrorString(ret);
        return ret;
    }

    for(size_t i=1; i<d->eq.size(); ++i) {
        if(ret>=0)
            ret=avfilter_link(d->eq[i - 1], 0, d->eq[i], 0);

        if(ret<0) {
            qCritical() << QString("Error connecting filters eq%1-eq%2").arg(i - 1).arg(i) << ffErrorString(ret);
            return ret;
        }
    }

    if(ret>=0)
        ret=avfilter_link(d->eq[d->eq.size() - 1], 0, d->abuffersink_ctx, 0);

    if(ret<0) {
        qCritical() << QString("Error connecting filters eq%1-sink").arg(d->eq.size() - 1) << ffErrorString(ret);
        return ret;
    }


    ret=avfilter_graph_config(d->filter_graph, NULL);

    if(ret<0) {
        qCritical() << "Error configuring the filter graph" << ffErrorString(ret);
        free();
        return ret;
    }

    qCritical() << "filter graph ok";

    return 0;
}

void FFEqualizer::setup(EQParams params)
{
    d->params=params;

    free();

    d->enabled=true;
}

bool FFEqualizer::proc(AVFrame *frame)
{
    if(!d->enabled)
        return false;

    if(!d->filter_graph || d->sample_rate!=frame->sample_rate || d->format!=frame->format || d->channels!=frame->channels) {
        qInfo() << "init" << frame->sample_rate << frame->format << frame->channels;
        d->sample_rate=frame->sample_rate;
        d->format=frame->format;
        d->channels=frame->channels;

        init();
    }

    if(!d->filter_graph) {
        d->enabled=false;
        return false;
    }

    int ret=av_buffersrc_add_frame(d->abuffer_ctx, frame);

    if(ret<0) {
        qCritical() << "Error write frame to the abuffer" << ffErrorString(ret);
        return false;
    }

    ret=av_buffersink_get_frame(d->abuffersink_ctx, frame);

    if(ret<0) {
        qCritical() << "Error read frame from the abuffersink" << ffErrorString(ret);
        return false;
    }

    return ret>=0;
}

void FFEqualizer::free()
{
    if(d->abuffer_ctx) {
        avfilter_free(d->abuffer_ctx);
        d->abuffer_ctx=nullptr;
    }

    if(d->abuffersink_ctx) {
        avfilter_free(d->abuffersink_ctx);
        d->abuffersink_ctx=nullptr;
    }

    for(size_t i=0; i<d->eq.size(); ++i) {
        if(d->eq[i]) {
            avfilter_free(d->eq[i]);
            d->eq[i]=nullptr;
        }
    }

    if(d->filter_graph) {
        avfilter_graph_free(&d->filter_graph);
        d->filter_graph=nullptr;
    }
}


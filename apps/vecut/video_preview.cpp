#include "video_preview.h"

VideoPreview::VideoPreview(QObject* parent) : QObject(parent) {
    audio_output_ = new QAudioOutput(this);
    player_ = new QMediaPlayer(this);
    video_sink_ = new QVideoSink(this);
    player_->setAudioOutput(audio_output_);
    player_->setVideoSink(video_sink_);
}

void VideoPreview::set_source(const QUrl& source) {
    player_->setSource(source);
}

void VideoPreview::clear() {
    player_->stop();
    player_->setSource(QUrl());
}

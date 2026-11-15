#include "audio_preview.h"

AudioPreview::AudioPreview(QObject* parent) : QObject(parent) {
    audio_output_ = new QAudioOutput(this);
    player_ = new QMediaPlayer(this);
    player_->setAudioOutput(audio_output_);
}

void AudioPreview::set_source(const QUrl& source) {
    player_->setSource(source);
}

void AudioPreview::clear() {
    player_->stop();
    player_->setSource(QUrl());
}

#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QUrl>

/** Portable Qt-backed video preview playback. No Android/iOS UI code. */
class VideoPreview final : public QObject {
public:
    explicit VideoPreview(QObject* parent = nullptr);
    QMediaPlayer* player() const noexcept { return player_; }
    QVideoSink* video_sink() const noexcept { return video_sink_; }
    void set_source(const QUrl& source);
    void clear();
private:
    QAudioOutput* audio_output_ = nullptr;
    QMediaPlayer* player_ = nullptr;
    QVideoSink* video_sink_ = nullptr;
};

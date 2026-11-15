#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

/** Portable Qt-backed audio-only preview playback. */
class AudioPreview final : public QObject {
public:
    explicit AudioPreview(QObject* parent = nullptr);
    QMediaPlayer* player() const noexcept { return player_; }
    void set_source(const QUrl& source);
    void clear();
private:
    QAudioOutput* audio_output_ = nullptr;
    QMediaPlayer* player_ = nullptr;
};

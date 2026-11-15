#pragma once

#include <QImage>
#include <QSize>
#include <QString>

namespace vecut {

/**
 * Cross-platform image inspection and preview helpers.
 *
 * Video/audio metadata and decoding are handled by Qt Multimedia's
 * QMediaPlayer/QVideoSink in the application layer. This class replaces the
 * image-specific part of the former Android MediaProbe helper without using
 * Android Bitmap or MediaMetadataRetriever APIs.
 */
class MediaProbe final {
public:
    struct ImageInfo {
        int width = 0;
        int height = 0;
        bool valid = false;
    };

    static ImageInfo inspect_image(const QString& path);
    static QImage image_thumbnail(const QString& path,
                                  const QSize& maximum_size = QSize(1280, 720));
    static QImage load_image_preview(const QString& path,
                                     const QSize& maximum_size = QSize(1920, 1080));
};

} // namespace vecut

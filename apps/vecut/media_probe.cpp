#include "media_probe.h"

#include <QImageReader>
#include <Qt>

namespace vecut {

MediaProbe::ImageInfo MediaProbe::inspect_image(const QString& path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QSize size = reader.size();
    if (!size.isValid()) return {};
    return {size.width(), size.height(), true};
}

QImage MediaProbe::image_thumbnail(const QString& path, const QSize& maximum_size) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QSize original_size = reader.size();
    if (original_size.isValid() && maximum_size.isValid()) {
        reader.setScaledSize(original_size.scaled(maximum_size, Qt::KeepAspectRatio));
    }
    return reader.read();
}

QImage MediaProbe::load_image_preview(const QString& path, const QSize& maximum_size) {
    return image_thumbnail(path, maximum_size);
}

} // namespace vecut

#pragma once

#include "vecut/project.h"

#include <QImage>
#include <QSize>

#include <cstdint>

class PreviewCompositor final {
public:
    // Composes one decoded/still frame with the selected clip's preview-only
    // transition and animation settings. This is a CPU/QPainter path, not the
    // final multi-track renderer or the future GPU shader pipeline.
    QImage compose(const QImage& frame, const QSize& target,
                   const vecut::Clip* clip,
                   std::int64_t clip_elapsed_ms) const;
};

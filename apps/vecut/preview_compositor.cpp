#include "preview_compositor.h"

#include <QPainter>
#include <QPointF>
#include <QRect>

#include <algorithm>
#include <cmath>

QImage PreviewCompositor::compose(const QImage& frame, const QSize& requested_target,
                                  const vecut::Clip* clip,
                                  std::int64_t clip_elapsed_ms) const {
    if (frame.isNull()) return {};

    QSize target = requested_target;
    if (target.width() < 2 || target.height() < 2)
        target = QSize(640, 360);

    QImage canvas(target, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::black);

    const QImage fitted = frame.scaled(target, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
    double opacity = 1.0;
    double reveal = 1.0;
    double slide = 0.0;
    double zoom = 1.0;
    double pan_x = 0.0;
    double pan_y = 0.0;
    double rotation = 0.0;

    if (clip) {
        const qint64 elapsed = std::max<qint64>(0, clip_elapsed_ms);
        const qint64 duration = clip->source_out_ms - clip->source_in_ms;
        const auto& transition = clip->incoming_transition;

        if (transition.fade_in_ms > 0) {
            opacity = std::min(opacity, std::clamp(
                static_cast<double>(elapsed) / transition.fade_in_ms, 0.0, 1.0));
        }
        if (transition.fade_out_ms > 0) {
            opacity = std::min(opacity, std::clamp(
                static_cast<double>(duration - elapsed) / transition.fade_out_ms,
                0.0, 1.0));
        }

        if (transition.duration_ms > 0 && elapsed < transition.duration_ms) {
            const double progress = std::clamp(
                static_cast<double>(elapsed) / transition.duration_ms, 0.0, 1.0);
            switch (transition.kind) {
            case vecut::TransitionKind::Fade:
            case vecut::TransitionKind::Dissolve:
                // Until adjacent-clip compositing exists, these reveal against black.
                opacity = std::min(opacity, progress);
                break;
            case vecut::TransitionKind::WipeLeftToRight:
            case vecut::TransitionKind::WipeRightToLeft:
                reveal = progress;
                break;
            case vecut::TransitionKind::SlideLeftToRight:
                slide = (1.0 - progress) * target.width();
                break;
            case vecut::TransitionKind::SlideRightToLeft:
                slide = -(1.0 - progress) * target.width();
                break;
            case vecut::TransitionKind::None:
                break;
            }
        }

        const auto& animation = clip->animation;
        if (animation.kind != vecut::AnimationKind::None && animation.duration_ms > 0) {
            const double progress = std::clamp(
                static_cast<double>(elapsed) / animation.duration_ms, 0.0, 1.0);
            switch (animation.kind) {
            case vecut::AnimationKind::ZoomIn: zoom = 1.0 + 0.35 * progress; break;
            case vecut::AnimationKind::ZoomOut: zoom = 1.35 - 0.35 * progress; break;
            case vecut::AnimationKind::PanLeft: pan_x = -0.22 * progress; break;
            case vecut::AnimationKind::PanRight: pan_x = 0.22 * progress; break;
            case vecut::AnimationKind::PanUp: pan_y = -0.22 * progress; break;
            case vecut::AnimationKind::PanDown: pan_y = 0.22 * progress; break;
            case vecut::AnimationKind::RotateClockwise: rotation = 12.0 * progress; break;
            case vecut::AnimationKind::RotateCounterClockwise: rotation = -12.0 * progress; break;
            case vecut::AnimationKind::None: break;
            }
        }
    }

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setOpacity(opacity);
    if (clip && clip->incoming_transition.kind ==
            vecut::TransitionKind::WipeLeftToRight && reveal < 1.0) {
        painter.setClipRect(QRect(0, 0,
            static_cast<int>(target.width() * reveal), target.height()));
    } else if (clip && clip->incoming_transition.kind ==
                   vecut::TransitionKind::WipeRightToLeft && reveal < 1.0) {
        const int reveal_width = static_cast<int>(target.width() * reveal);
        painter.setClipRect(QRect(target.width() - reveal_width, 0,
                                  reveal_width, target.height()));
    }

    const QPointF center(target.width() / 2.0 + pan_x * target.width() + slide,
                         target.height() / 2.0 + pan_y * target.height());
    painter.translate(center);
    painter.rotate(rotation);
    painter.scale(zoom, zoom);
    painter.translate(-fitted.width() / 2.0, -fitted.height() / 2.0);
    painter.drawImage(QPoint(0, 0), fitted);
    painter.end();
    return canvas;
}

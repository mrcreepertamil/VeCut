#include "main_window.h"
#include "media_probe.h"
#include "gpu_preview_widget.h"
#include "preview_shader_library.h"

#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QTabWidget>
#include <QListView>
#include <QMap>
#include <QFile>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFileInfo>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QSizePolicy>
#include <QPaintEvent>
#include <QFont>
#include <QMouseEvent>
#include <QScrollArea>
#include <QAbstractItemView>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QImageReader>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QVideoFrame>
#include <QSignalBlocker>
#include <QTransform>
#include <QStackedWidget>
#include <QSlider>
#include <QPixmap>
#include <QIcon>
#include <QUrl>
#include <QPushButton>
#include <QShortcut>
#include <QKeySequence>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QResizeEvent>
#include <QTimer>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <QStyle>
#include <QSvgRenderer>
#if defined(Q_OS_ANDROID)
#include <QJniObject>
#include <QJniEnvironment>
#include <QCoreApplication>
#include <QElapsedTimer>
#endif

#include <algorithm>
#include <limits>
#include <cmath>
#include <QStringList>
#include <utility>
#include <functional>
#include <memory>


namespace {
constexpr auto kMediaMimeType = "application/x-vecut-media-id";
constexpr std::int64_t kStillImageClipDurationMs = 5000;

class MediaBinListWidget final : public QListWidget {
public:
    using QListWidget::QListWidget;
protected:
    void startDrag(Qt::DropActions supported_actions) override {
        auto* item = currentItem();
        if (!item) return;
        const QString media_id = item->data(Qt::UserRole).toString();
        if (media_id.isEmpty()) return;
        auto* mime = new QMimeData();
        mime->setData(kMediaMimeType, media_id.toUtf8());
        auto* drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->exec(supported_actions & Qt::CopyAction, Qt::CopyAction);
    }
};


}

namespace {
constexpr int kPortraitBreakpoint = 720;

QLabel* make_section_title(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setStyleSheet("color:#f4f5fa;font-size:14px;font-weight:700;");
    return label;
}


enum class VeCutIcon {
    Open, Save, Undo, Redo, Import, Play, Pause, Split, Trim, Transition, Volume,
    Delete, Search, Help, Settings, Video, Photo, Export, AddVideo, AddAudio,
    AutoCut, Retouch, Story, Desktop, Captions, Sparkles, RemoveBackground,
    Grid, FolderPlus, Sort, Edit, Template, Inbox, User
};

// Lucide icons are MIT-licensed SVG artwork. Render them at runtime so Qt can
// scale them cleanly on Android and desktop without bitmap sprite assets.
QString lucide_svg_body(VeCutIcon icon) {
    switch (icon) {
    case VeCutIcon::Open:
        return "<path d='M3 10a2 2 0 0 1 2-2h5l2 2h7a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z'/><path d='M3 10h18'/>";
    case VeCutIcon::Save:
        return "<path d='M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2Z'/><path d='M17 21v-8H7v8'/><path d='M7 3v5h8'/>";
    case VeCutIcon::Undo:
        return "<path d='M3 7v6h6'/><path d='M21 17a9 9 0 0 0-15-6.7L3 13'/>";
    case VeCutIcon::Redo:
        return "<path d='M21 7v6h-6'/><path d='M3 17a9 9 0 0 1 15-6.7L21 13'/>";
    case VeCutIcon::Import:
        return "<path d='M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4'/><path d='m7 10 5 5 5-5'/><path d='M12 15V3'/>";
    case VeCutIcon::Play:
        return "<polygon points='6 3 20 12 6 21 6 3'/>";
    case VeCutIcon::Pause:
        return "<rect x='6' y='4' width='4' height='16' rx='1'/><rect x='14' y='4' width='4' height='16' rx='1'/>";
    case VeCutIcon::AddVideo:
        return "<rect x='3' y='5' width='13' height='14' rx='2'/><path d='m16 10 5-3v10l-5-3'/><path d='M9.5 9v6M6.5 12h6'/>";
    case VeCutIcon::AddAudio:
        return "<path d='M9 18V5l12-2v13'/><circle cx='6' cy='18' r='3'/><circle cx='18' cy='16' r='3'/><path d='M9 9l12-2'/>";
    case VeCutIcon::AutoCut:
        return "<path d='m4 4 16 16M20 4 4 20'/><circle cx='12' cy='12' r='3'/><path d='M12 2v4M12 18v4M2 12h4M18 12h4'/>";
    case VeCutIcon::Retouch:
        return "<path d='m12 3 1.9 5.8L20 11l-6.1 2.2L12 19l-1.9-5.8L4 11l6.1-2.2L12 3Z'/><path d='m19 14 .9 2.1L22 17l-2.1.9L19 20l-.9-2.1L16 17l2.1-.9L19 14Z'/>";
    case VeCutIcon::Story:
        return "<path d='M4 4h16v16H4z'/><path d='M8 8h8M8 12h8M8 16h5'/>";
    case VeCutIcon::Desktop:
        return "<rect x='3' y='4' width='18' height='13' rx='2'/><path d='M8 21h8M12 17v4'/>";
    case VeCutIcon::Captions:
        return "<rect x='3' y='5' width='18' height='14' rx='2'/><path d='M7 10h4M7 14h3M14 10h3M14 14h3'/>";
    case VeCutIcon::Sparkles:
        return "<path d='m12 3 1.8 5.2L19 10l-5.2 1.8L12 17l-1.8-5.2L5 10l5.2-1.8L12 3Z'/><path d='m19 14 .9 2.1L22 17l-2.1.9L19 20l-.9-2.1L16 17l2.1-.9L19 14Z'/>";
    case VeCutIcon::RemoveBackground:
        return "<rect x='3' y='3' width='18' height='18' rx='2'/><path d='m3 15 5-5 4 4 3-3 6 6'/><path d='M3 3l18 18'/>";
    case VeCutIcon::Grid:
        return "<rect x='3' y='3' width='7' height='7' rx='1'/><rect x='14' y='3' width='7' height='7' rx='1'/><rect x='3' y='14' width='7' height='7' rx='1'/><rect x='14' y='14' width='7' height='7' rx='1'/>";
    case VeCutIcon::FolderPlus:
        return "<path d='M3 7a2 2 0 0 1 2-2h5l2 2h7a2 2 0 0 1 2 2v10a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z'/><path d='M12 11v6M9 14h6'/>";
    case VeCutIcon::Sort:
        return "<path d='M4 6h16M7 12h10M10 18h4'/>";
    case VeCutIcon::Edit:
        return "<path d='m16 4 4 4L9 19l-5 1 1-5L16 4Z'/><path d='m14 6 4 4'/>";
    case VeCutIcon::Template:
        return "<rect x='3' y='3' width='18' height='18' rx='2'/><path d='M3 9h18M9 21V9'/>";
    case VeCutIcon::Inbox:
        return "<path d='M4 4h16l2 11v4a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2v-4L4 4Z'/><path d='M2 15h6l2 3h4l2-3h6'/>";
    case VeCutIcon::User:
        return "<circle cx='12' cy='8' r='4'/><path d='M4 21a8 8 0 0 1 16 0'/>";
    case VeCutIcon::Split:
    case VeCutIcon::Trim:
        return "<circle cx='6' cy='6' r='3'/><circle cx='6' cy='18' r='3'/><path d='m8.1 8.1 12.8 12.8'/><path d='m14 6 7-7'/><path d='m8.1 15.9 5.9-5.9'/>";
    case VeCutIcon::Transition:
        return "<path d='M17 2 21 6 17 10'/><path d='M3 12V10a4 4 0 0 1 4-4h14'/><path d='m7 22-4-4 4-4'/><path d='M21 12v2a4 4 0 0 1-4 4H3'/>";
    case VeCutIcon::Volume:
        return "<polygon points='11 5 6 9 2 9 2 15 6 15 11 19 11 5'/><path d='M15.5 8.5a5 5 0 0 1 0 7'/><path d='M19 5a10 10 0 0 1 0 14'/>";
    case VeCutIcon::Delete:
        return "<path d='M3 6h18'/><path d='M8 6V4h8v2'/><path d='m19 6-1 14H6L5 6'/><path d='M10 11v5'/><path d='M14 11v5'/>";
    case VeCutIcon::Search:
        return "<circle cx='11' cy='11' r='8'/><path d='m21 21-4.35-4.35'/>";
    case VeCutIcon::Help:
        return "<circle cx='12' cy='12' r='10'/><path d='M9.1 9a3 3 0 0 1 5.8 1c0 2-3 3-3 3'/><path d='M12 17h.01'/>";
    case VeCutIcon::Settings:
        return "<path d='M12 8a4 4 0 1 0 0 8 4 4 0 0 0 0-8Z'/><path d='m19.4 15 .1.1 1.4 1.1-1.4 2.4-1.7-.6a8 8 0 0 1-1.5.9l-.3 1.8h-2.8l-.3-1.8a8 8 0 0 1-1.5-.9l-1.7.6-1.4-2.4 1.4-1.1a8 8 0 0 1 0-1.8l-1.4-1.1 1.4-2.4 1.7.6a8 8 0 0 1 1.5-.9l.3-1.8h2.8l.3 1.8a8 8 0 0 1 1.5.9l1.7-.6 1.4 2.4-1.4 1.1a8 8 0 0 1 0 1.8Z'/>";
    case VeCutIcon::Video:
        return "<rect x='3' y='5' width='13' height='14' rx='2'/><path d='m16 10 5-3v10l-5-3'/>";
    case VeCutIcon::Photo:
        return "<rect x='3' y='3' width='18' height='18' rx='2'/><circle cx='8.5' cy='8.5' r='1.5'/><path d='m21 15-5-5L5 21'/>";
    case VeCutIcon::Export:
        return "<path d='M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4'/><path d='m17 8-5-5-5 5'/><path d='M12 3v12'/>";
    }
    return {};
}

QIcon vecut_icon(VeCutIcon icon_id) {
    const auto render_state = [icon_id](const QColor& color) {
        const QByteArray svg = QString(
            "<svg xmlns='http://www.w3.org/2000/svg' width='48' height='48' viewBox='0 0 24 24'>"
            "<g fill='none' stroke='%1' stroke-width='2' stroke-linecap='round' "
            "stroke-linejoin='round'>%2</g></svg>")
            .arg(color.name(), lucide_svg_body(icon_id)).toUtf8();
        QSvgRenderer renderer(svg);
        QPixmap pixmap(48, 48);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
        return pixmap;
    };

    QIcon icon;
    icon.addPixmap(render_state(QColor("#A6A4AE")), QIcon::Normal, QIcon::Off);
    icon.addPixmap(render_state(QColor("#39D353")), QIcon::Active, QIcon::Off);
    icon.addPixmap(render_state(QColor("#168A36")), QIcon::Selected, QIcon::Off);
    return icon;
}

QFrame* make_panel(QWidget* parent) {
    auto* frame = new QFrame(parent);
    frame->setObjectName("panel");
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}

bool is_supported_media_file(const QFileInfo& info) {
    static const QStringList supported = {
        "mp4", "mov", "mkv", "webm", "avi", "mp3", "wav", "aac", "m4a",
        "ogg", "flac", "png", "jpg", "jpeg", "webp", "bmp", "gif"
    };
    return supported.contains(info.suffix().toLower());
}

vecut::MediaKind media_kind_for_file(const QFileInfo& info) {
    const QString suffix = info.suffix().toLower();
    static const QStringList audio = {"mp3", "wav", "aac", "m4a", "ogg", "flac"};
    static const QStringList images = {"png", "jpg", "jpeg", "webp", "bmp", "gif"};
    if (audio.contains(suffix)) return vecut::MediaKind::Audio;
    if (images.contains(suffix)) return vecut::MediaKind::Image;
    return vecut::MediaKind::Video;
}

QString media_kind_name(vecut::MediaKind kind) {
    switch (kind) {
    case vecut::MediaKind::Audio: return "Audio";
    case vecut::MediaKind::Image: return "Image";
    case vecut::MediaKind::Video: return "Video";
    }
    return "Media";
}
}


class TimelineWidget final : public QWidget {
public:
    explicit TimelineWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setAcceptDrops(true);
        setMouseTracking(true);
        setMinimumWidth(900);
        setMinimumHeight(90);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    void setProject(const vecut::Project* project) {
        project_ = project;
        refresh();
    }

    void refresh() {
        resize(sizeHint());
        updateGeometry();
        update();
    }

    void selectClip(const QString& clip_id) {
        selected_clip_id_ = clip_id;
        update();
    }

    std::function<void(const QString&, const QString&, std::int64_t)> on_media_dropped;
    std::function<void(const QString&, const QString&, std::int64_t,
                       std::int64_t, std::int64_t)> on_clip_edited;
    std::function<void(const QString&, const QString&)> on_clip_selected;
    std::function<void()> on_clip_edit_started;

    QSize sizeHint() const override {
        std::int64_t duration = project_ ? project_->duration_ms() : 0;
        const int content_width = kTrackLabelWidth +
            static_cast<int>((duration / 1000.0 + 10.0) * kPixelsPerSecond) + 40;
        const int rows = project_ ? static_cast<int>(project_->tracks().size()) : 0;
        return QSize(std::max(900, content_width),
                     kRulerHeight + std::max(1, rows) * kTrackRowHeight + 8);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor("#100E16"));

        painter.fillRect(QRect(0, 0, width(), kRulerHeight), QColor("#1B1922"));
        painter.setPen(QColor("#8992a4"));
        painter.setFont(QFont("Sans Serif", 9, QFont::DemiBold));
        painter.drawText(QRect(10, 0, kTrackLabelWidth - 18, kRulerHeight),
                         Qt::AlignVCenter, "TRACK");
        const int timeline_width = width() - kTrackLabelWidth;
        const int tick_step = static_cast<int>(5 * kPixelsPerSecond);
        for (int x = kTrackLabelWidth, seconds = 0; x < width();
             x += tick_step, seconds += 5) {
            painter.setPen(QColor("#393742"));
            painter.drawLine(x, kRulerHeight - 5, x, height());
            painter.setPen(QColor("#aab2c0"));
            painter.drawText(QRect(x + 5, 2, tick_step - 8, kRulerHeight - 4),
                             Qt::AlignVCenter, QString("%1s").arg(seconds));
        }
        Q_UNUSED(timeline_width);

        if (!project_ || project_->tracks().empty()) {
            painter.setPen(QColor("#929aaa"));
            painter.drawText(QRect(kTrackLabelWidth + 12, kRulerHeight + 8,
                                   width() - kTrackLabelWidth - 24, 52),
                             Qt::AlignVCenter, "Add a video or audio track, then drag media here");
            return;
        }

        for (std::size_t row = 0; row < project_->tracks().size(); ++row) {
            const auto& track = project_->tracks()[row];
            const int y = kRulerHeight + static_cast<int>(row) * kTrackRowHeight;
            painter.fillRect(QRect(0, y, width(), kTrackRowHeight - 2),
                             row % 2 == 0 ? QColor("#1B1922") : QColor("#15131B"));
            painter.setPen(QColor("#e1e5ee"));
            painter.setFont(QFont("Sans Serif", 9, QFont::DemiBold));
            painter.drawText(QRect(10, y + 4, kTrackLabelWidth - 18, 25),
                             Qt::AlignVCenter,
                             QString::fromStdString(track.name));
            painter.setPen(QColor("#7e899b"));
            painter.setFont(QFont("Sans Serif", 8));
            painter.drawText(QRect(10, y + 29, kTrackLabelWidth - 18, 20),
                             Qt::AlignVCenter,
                             track.kind == vecut::TrackKind::Audio ? "AUDIO" : "VIDEO");
            painter.setPen(QColor("#393742"));
            painter.drawLine(kTrackLabelWidth, y, width(), y);

            for (const auto& clip : track.clips) {
                const auto media = std::find_if(project_->media().begin(),
                    project_->media().end(), [&](const vecut::MediaAsset& item) {
                        return item.id == clip.media_id;
                    });
                if (media == project_->media().end()) continue;
                const auto clip_duration = clip.source_out_ms - clip.source_in_ms;
                const int x = kTrackLabelWidth +
                    static_cast<int>(clip.timeline_start_ms * kPixelsPerSecond / 1000.0);
                const int w = std::max(28, static_cast<int>(
                    clip_duration * kPixelsPerSecond / 1000.0));
                const QRect clip_rect(x, y + 12, w, 44);
                QColor fill = media->kind == vecut::MediaKind::Audio
                    ? QColor("#168A36")
                    : (media->kind == vecut::MediaKind::Image
                        ? QColor("#39D353") : QColor("#168A36"));
                painter.setPen(selected_clip_id_ == QString::fromStdString(clip.id)
                    ? QPen(QColor("#39D353"), 2) : QPen(fill.lighter(135), 1));
                painter.setBrush(fill);
                painter.drawRoundedRect(clip_rect, 5, 5);
                painter.setPen(QColor("#ffffff"));
                painter.setFont(QFont("Sans Serif", 8, QFont::DemiBold));
                const QString name = QString::fromStdString(media->display_name);
                const QString text = painter.fontMetrics().elidedText(
                    name, Qt::ElideRight, std::max(8, w - 18));
                painter.drawText(clip_rect.adjusted(9, 0, -8, 0),
                                 Qt::AlignVCenter | Qt::AlignLeft, text);
                painter.setPen(QPen(QColor("#e6edf7"), 2));
                painter.drawLine(x + 4, y + 20, x + 4, y + 48);
                painter.drawLine(x + w - 4, y + 20, x + w - 4, y + 48);
            }
        }
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasFormat(kMediaMimeType))
            event->acceptProposedAction();
        else
            event->ignore();
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (event->mimeData()->hasFormat(kMediaMimeType) &&
            track_index_at(event->position().toPoint().y()) >= 0)
            event->acceptProposedAction();
        else
            event->ignore();
    }

    void dropEvent(QDropEvent* event) override {
        const QPoint point = event->position().toPoint();
        const int row = track_index_at(point.y());
        if (!project_ || row < 0 || !on_media_dropped ||
            !event->mimeData()->hasFormat(kMediaMimeType)) {
            event->ignore();
            return;
        }
        const auto& track = project_->tracks()[static_cast<std::size_t>(row)];
        const auto timeline_ms = static_cast<std::int64_t>(std::max(
            0.0, (point.x() - kTrackLabelWidth) * 1000.0 / kPixelsPerSecond));
        on_media_dropped(QString::fromUtf8(
            event->mimeData()->data(kMediaMimeType)),
            QString::fromStdString(track.id), timeline_ms);
        event->acceptProposedAction();
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() != Qt::LeftButton || !project_) {
            QWidget::mousePressEvent(event);
            return;
        }
        const QPoint point = event->position().toPoint();
        const int row = track_index_at(point.y());
        if (row < 0) {
            selected_clip_id_.clear();
            drag_mode_ = DragMode::None;
            if (on_clip_selected) on_clip_selected({}, {});
            update();
            return;
        }
        const auto& track = project_->tracks()[static_cast<std::size_t>(row)];
        const auto* hit = clip_at(track, point.x(), point.y());
        if (!hit) {
            selected_clip_id_.clear();
            drag_mode_ = DragMode::None;
            if (on_clip_selected) on_clip_selected({}, {});
            update();
            return;
        }
        selected_clip_id_ = QString::fromStdString(hit->id);
        if (on_clip_selected)
            on_clip_selected(QString::fromStdString(track.id), selected_clip_id_);
        drag_track_id_ = QString::fromStdString(track.id);
        drag_clip_id_ = selected_clip_id_;
        drag_origin_x_ = point.x();
        original_start_ms_ = hit->timeline_start_ms;
        original_in_ms_ = hit->source_in_ms;
        original_out_ms_ = hit->source_out_ms;
        const int clip_x = kTrackLabelWidth +
            static_cast<int>(hit->timeline_start_ms * kPixelsPerSecond / 1000.0);
        const int clip_width = std::max(28, static_cast<int>(
            (hit->source_out_ms - hit->source_in_ms) * kPixelsPerSecond / 1000.0));
        if (point.x() <= clip_x + kTrimHandleWidth)
            drag_mode_ = DragMode::TrimLeft;
        else if (point.x() >= clip_x + clip_width - kTrimHandleWidth)
            drag_mode_ = DragMode::TrimRight;
        else
            drag_mode_ = DragMode::Move;
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (drag_mode_ == DragMode::None || !project_ || !on_clip_edited) {
            QWidget::mouseMoveEvent(event);
            return;
        }
        const auto delta = static_cast<std::int64_t>(std::llround(
            (event->position().x() - drag_origin_x_) * 1000.0 / kPixelsPerSecond));
        std::int64_t start = original_start_ms_;
        std::int64_t source_in = original_in_ms_;
        std::int64_t source_out = original_out_ms_;

        if (drag_mode_ == DragMode::Move) {
            start = std::max<std::int64_t>(0, original_start_ms_ + delta);
        } else if (drag_mode_ == DragMode::TrimLeft) {
            const auto min_delta = std::max(-original_start_ms_, -original_in_ms_);
            const auto max_delta = original_out_ms_ - original_in_ms_ - 1;
            const auto applied = std::clamp(delta, min_delta, max_delta);
            start = original_start_ms_ + applied;
            source_in = original_in_ms_ + applied;
        } else if (drag_mode_ == DragMode::TrimRight) {
            const auto media = media_for_clip(drag_clip_id_);
            const auto maximum = media ? media->duration_ms : original_out_ms_;
            source_out = std::clamp(original_out_ms_ + delta,
                                    original_in_ms_ + 1, maximum);
        }
        if (!drag_history_saved_ && on_clip_edit_started) {
            on_clip_edit_started();
            drag_history_saved_ = true;
        }
        on_clip_edited(drag_track_id_, drag_clip_id_, start, source_in, source_out);
        refresh();
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            drag_mode_ = DragMode::None;
            drag_history_saved_ = false;
            unsetCursor();
        }
        QWidget::mouseReleaseEvent(event);
    }

private:
    enum class DragMode { None, Move, TrimLeft, TrimRight };
    static constexpr int kTrackLabelWidth = 112;
    static constexpr int kRulerHeight = 30;
    static constexpr int kTrackRowHeight = 70;
    static constexpr int kTrimHandleWidth = 8;
    static constexpr double kPixelsPerSecond = 80.0;

    int track_index_at(int y) const {
        if (!project_ || y < kRulerHeight) return -1;
        const int row = (y - kRulerHeight) / kTrackRowHeight;
        return row >= 0 && row < static_cast<int>(project_->tracks().size()) ? row : -1;
    }

    const vecut::Clip* clip_at(const vecut::Track& track, int x, int y) const {
        const int row_y = kRulerHeight + track_index_at(y) * kTrackRowHeight;
        if (y < row_y + 12 || y > row_y + 56) return nullptr;
        for (const auto& clip : track.clips) {
            const int clip_x = kTrackLabelWidth +
                static_cast<int>(clip.timeline_start_ms * kPixelsPerSecond / 1000.0);
            const int clip_width = std::max(28, static_cast<int>(
                (clip.source_out_ms - clip.source_in_ms) * kPixelsPerSecond / 1000.0));
            if (x >= clip_x && x <= clip_x + clip_width) return &clip;
        }
        return nullptr;
    }

    const vecut::MediaAsset* media_for_clip(const QString& clip_id) const {
        if (!project_) return nullptr;
        for (const auto& track : project_->tracks()) {
            const auto clip = std::find_if(track.clips.begin(), track.clips.end(),
                [&](const vecut::Clip& item) {
                    return QString::fromStdString(item.id) == clip_id;
                });
            if (clip != track.clips.end()) {
                const auto media = std::find_if(project_->media().begin(),
                    project_->media().end(), [&](const vecut::MediaAsset& item) {
                        return item.id == clip->media_id;
                    });
                return media == project_->media().end() ? nullptr : &*media;
            }
        }
        return nullptr;
    }

    const vecut::Project* project_ = nullptr;
    QString selected_clip_id_;
    QString drag_track_id_;
    QString drag_clip_id_;
    DragMode drag_mode_ = DragMode::None;
    bool drag_history_saved_ = false;
    int drag_origin_x_ = 0;
    std::int64_t original_start_ms_ = 0;
    std::int64_t original_in_ms_ = 0;
    std::int64_t original_out_ms_ = 0;
};

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("VeCut");
    resize(1360, 860);
    setMinimumSize(320, 480);

    auto* root = new QWidget(this);
    root->setObjectName("root");
    auto* outer = new QVBoxLayout(root);
    outer->setContentsMargins(12, 10, 12, 10);
    outer->setSpacing(10);

    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);
    auto* brand = new QLabel("VeCut  <span style='color:#22c55e'>STUDIO</span>", root);
    brand->setTextFormat(Qt::RichText);
    brand->setStyleSheet("font-size:22px;font-weight:800;color:#f4f5fa;");
    toolbar->addWidget(brand);
    toolbar->addStretch();

    auto* open_button = new QPushButton("Open project", root);
    open_button->setIcon(vecut_icon(VeCutIcon::Open));
    auto* save_button = new QPushButton("Save project", root);
    save_button->setIcon(vecut_icon(VeCutIcon::Save));
    undo_button_ = new QPushButton("Undo", root);
    undo_button_->setIcon(vecut_icon(VeCutIcon::Undo));
    redo_button_ = new QPushButton("Redo", root);
    redo_button_->setIcon(vecut_icon(VeCutIcon::Redo));
    undo_button_->setEnabled(false);
    redo_button_->setEnabled(false);
    auto* import_button = new QPushButton("Import media", root);
    import_button->setIcon(vecut_icon(VeCutIcon::Import));
    import_button->setObjectName("primaryButton");
    import_button->setMinimumHeight(42);
    toolbar->addWidget(open_button);
    toolbar->addWidget(save_button);
    toolbar->addWidget(undo_button_);
    toolbar->addWidget(redo_button_);
    toolbar->addWidget(import_button);
    outer->addLayout(toolbar);

    workspace_ = new QSplitter(Qt::Horizontal, root);
    workspace_->setChildrenCollapsible(false);
    workspace_->setHandleWidth(6);

    auto* media_panel = make_panel(workspace_);
    auto* media_layout = new QVBoxLayout(media_panel);
    media_layout->setContentsMargins(10, 10, 10, 10);
    media_layout->setSpacing(8);
    media_layout->addWidget(make_section_title("MEDIA BIN", media_panel));
    media_list_ = new MediaBinListWidget(media_panel);
    media_list_->setObjectName("mediaList");
    media_list_->setMinimumWidth(0);
    media_list_->setSpacing(4);
    media_list_->setDragEnabled(true);
    media_list_->setDragDropMode(QAbstractItemView::DragOnly);
    media_layout->addWidget(media_list_, 1);
    media_details_ = new QLabel("No media selected", media_panel);
    media_details_->setWordWrap(true);
    media_details_->setStyleSheet("color:#a4a8b5;font-size:12px;");
    media_layout->addWidget(media_details_);
    workspace_->addWidget(media_panel);

    auto* preview_panel = make_panel(workspace_);
    auto* preview_layout = new QVBoxLayout(preview_panel);
    preview_layout->setContentsMargins(10, 10, 10, 10);
    preview_layout->setSpacing(8);
    preview_layout->addWidget(make_section_title("PREVIEW", preview_panel));
    preview_label_ = new QLabel("Your preview will appear here

Import media to begin", preview_panel);
    preview_label_->setAlignment(Qt::AlignCenter);
    preview_label_->setMinimumSize(0, 0);
    preview_label_->setWordWrap(true);
    preview_label_->setStyleSheet("background:#090a0e;border:1px solid #303440;border-radius:12px;"
                                  "color:#a4a8b5;font-size:15px;");
    preview_stack_ = new QStackedWidget(preview_panel);
    preview_stack_->addWidget(preview_label_);
    gpu_preview_ = new GpuPreviewWidget(preview_panel);
    preview_stack_->addWidget(gpu_preview_);
    preview_layout->addWidget(preview_stack_, 1);

    auto* preview_controls = new QHBoxLayout();
    preview_play_button_ = new QPushButton("Play", preview_panel);
    preview_play_button_->setIcon(vecut_icon(VeCutIcon::Play));
    preview_play_button_->setMinimumHeight(36);
    preview_seek_ = new QSlider(Qt::Horizontal, preview_panel);
    preview_seek_->setRange(0, 0);
    preview_controls->addWidget(preview_play_button_);
    preview_controls->addWidget(preview_seek_, 1);
    preview_filter_ = new QComboBox(preview_panel);
    preview_filter_->addItem("Normal", static_cast<int>(vecut::PreviewFilterMode::Normal));
    preview_filter_->addItem("Monochrome", static_cast<int>(vecut::PreviewFilterMode::Monochrome));
    preview_filter_->addItem("Warm", static_cast<int>(vecut::PreviewFilterMode::Warm));
    preview_filter_->addItem("Cool", static_cast<int>(vecut::PreviewFilterMode::Cool));
    preview_filter_->addItem("Sepia", static_cast<int>(vecut::PreviewFilterMode::Sepia));
    preview_filter_->addItem("Vintage", static_cast<int>(vecut::PreviewFilterMode::Vintage));
    preview_filter_->addItem("Invert", static_cast<int>(vecut::PreviewFilterMode::Invert));
    preview_filter_->addItem("Contrast", static_cast<int>(vecut::PreviewFilterMode::Contrast));
    preview_controls->addWidget(preview_filter_);
    preview_layout->addLayout(preview_controls);
    connect(preview_filter_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (gpu_preview_) gpu_preview_->set_filter(
            static_cast<vecut::PreviewFilterMode>(preview_filter_->itemData(index).toInt()));
    });

    video_preview_ = new VideoPreview(this);
    audio_preview_ = new AudioPreview(this);
    preview_player_ = video_preview_->player();
    connect(video_preview_->video_sink(), &QVideoSink::videoFrameChanged, this,
            [this](const QVideoFrame& frame) {
        const QImage image = frame.toImage();
        if (!image.isNull()) {
            current_preview_frame_ = image;
            render_preview_frame();
        }
    });
    preview_effect_timer_ = new QTimer(this);
    preview_effect_timer_->setInterval(16);
    connect(preview_effect_timer_, &QTimer::timeout, this, [this] {
        const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
            [&](const vecut::Track& item) {
                return QString::fromStdString(item.id) == selected_track_id_;
            });
        if (track == project_.tracks().end()) {
            preview_effect_timer_->stop();
            preview_play_button_->setText("Play");
            preview_play_button_->setIcon(vecut_icon(VeCutIcon::Play));
            return;
        }
        const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
            [&](const vecut::Clip& item) {
                return QString::fromStdString(item.id) == selected_clip_id_;
            });
        if (clip == track->clips.end()) {
            preview_effect_timer_->stop();
            preview_play_button_->setText("Play");
            preview_play_button_->setIcon(vecut_icon(VeCutIcon::Play));
            return;
        }
        preview_effect_time_ms_ = std::min<std::int64_t>(
            preview_effect_time_ms_ + 16, clip->source_out_ms - clip->source_in_ms);
        preview_seek_->setValue(static_cast<int>(preview_effect_time_ms_));
        render_preview_frame();
        if (preview_effect_time_ms_ >= clip->source_out_ms - clip->source_in_ms) {
            preview_effect_timer_->stop();
            preview_play_button_->setText("Play");
            preview_play_button_->setIcon(vecut_icon(VeCutIcon::Play));
        }
    });
    connect(preview_play_button_, &QPushButton::clicked, this, [this] {
        if (!preview_player_) return;
        if (!preview_player_->source().isEmpty()) {
            if (preview_player_->playbackState() == QMediaPlayer::PlayingState)
                preview_player_->pause();
            else
                preview_player_->play();
            return;
        }
        if (current_preview_frame_.isNull() || selected_clip_id_.isEmpty()) return;
        if (preview_effect_timer_->isActive()) {
            preview_effect_timer_->stop();
            preview_play_button_->setText("Play");
            preview_play_button_->setIcon(vecut_icon(VeCutIcon::Play));
        } else {
            preview_effect_timer_->start();
            preview_play_button_->setText("Pause");
            preview_play_button_->setIcon(vecut_icon(VeCutIcon::Pause));
        }
    });
    const auto connect_preview_player = [this](QMediaPlayer* player) {
        connect(player, &QMediaPlayer::playbackStateChanged, this,
                [this](QMediaPlayer::PlaybackState state) {
            const bool playing = state == QMediaPlayer::PlayingState;
            preview_play_button_->setText(playing ? "Pause" : "Play");
            preview_play_button_->setIcon(vecut_icon(playing ? VeCutIcon::Pause : VeCutIcon::Play));
        });
        connect(player, &QMediaPlayer::durationChanged, this,
                [this, player](qint64 duration) {
            if (player != preview_player_) return;
            preview_seek_->setRange(0, static_cast<int>(std::min<qint64>(
                duration, std::numeric_limits<int>::max())));
        });
        connect(player, &QMediaPlayer::positionChanged, this, [this, player](qint64 position) {
            if (player == preview_player_ && !preview_seek_->isSliderDown())
                preview_seek_->setValue(static_cast<int>(position));
            if (player == preview_player_) render_preview_frame();
        });
        connect(player, &QMediaPlayer::errorOccurred, this,
                [this, player](QMediaPlayer::Error, const QString& error) {
            if (player != preview_player_) return;
            current_preview_frame_ = QImage();
            preview_label_->setPixmap(QPixmap());
            preview_label_->setText(QString("Preview could not play this file\
%1").arg(error));
            preview_stack_->setCurrentWidget(preview_label_);
            status_label_->setText("Preview playback error.");
        });
    };
    connect_preview_player(video_preview_->player());
    connect_preview_player(audio_preview_->player());
    connect(preview_seek_, &QSlider::sliderMoved, this, [this](int position) {
        if (!preview_player_->source().isEmpty()) {
            preview_player_->setPosition(position);
        } else {
            preview_effect_time_ms_ = std::max(0, position);
            render_preview_frame();
        }
    });
    workspace_->addWidget(preview_panel);
    workspace_->setStretchFactor(0, 1);
    workspace_->setStretchFactor(1, 3);
    outer->addWidget(workspace_, 1);

    timeline_panel_ = make_panel(root);
    auto* timeline_layout = new QVBoxLayout(timeline_panel_);
    timeline_layout->setContentsMargins(10, 8, 10, 8);
    timeline_layout->setSpacing(6);
    auto* timeline_toolbar = new QHBoxLayout();
    timeline_toolbar->setSpacing(6);
    timeline_toolbar->addWidget(make_section_title("TIMELINE", timeline_panel_));
    timeline_toolbar->addStretch();
    split_button_ = new QPushButton("Split", timeline_panel_);
    split_button_->setIcon(vecut_icon(VeCutIcon::Split));
    trim_button_ = new QPushButton("Trim", timeline_panel_);
    trim_button_->setIcon(vecut_icon(VeCutIcon::Trim));
    transition_button_ = new QPushButton("Transition", timeline_panel_);
    transition_button_->setIcon(vecut_icon(VeCutIcon::Transition));
    volume_button_ = new QPushButton("Volume", timeline_panel_);
    volume_button_->setIcon(vecut_icon(VeCutIcon::Volume));
    remove_clip_button_ = new QPushButton("Remove clip", timeline_panel_);
    remove_clip_button_->setIcon(vecut_icon(VeCutIcon::Delete));
    add_video_button_ = new QPushButton("Video track", timeline_panel_);
    add_video_button_->setIcon(vecut_icon(VeCutIcon::AddVideo));
    add_audio_button_ = new QPushButton("Audio track", timeline_panel_);
    add_audio_button_->setIcon(vecut_icon(VeCutIcon::AddAudio));
    add_video_button_->setToolTip("Add a video track");
    add_audio_button_->setToolTip("Add an audio track");
    add_video_button_->setMinimumHeight(40);
    add_audio_button_->setMinimumHeight(40);
    timeline_toolbar->addWidget(split_button_);
    timeline_toolbar->addWidget(trim_button_);
    timeline_toolbar->addWidget(transition_button_);
    timeline_toolbar->addWidget(volume_button_);
    timeline_audio_button_ = new QPushButton("Play timeline audio", timeline_panel_);
    timeline_audio_button_->setIcon(vecut_icon(VeCutIcon::Play));
    timeline_audio_button_->setToolTip("Play or stop the timeline audio mix");
    timeline_toolbar->addWidget(timeline_audio_button_);
    timeline_toolbar->addWidget(remove_clip_button_);
    timeline_toolbar->addWidget(add_video_button_);
    timeline_toolbar->addWidget(add_audio_button_);
    timeline_layout->addLayout(timeline_toolbar);

    auto* timeline_scroll = new QScrollArea(timeline_panel_);
    timeline_scroll->setWidgetResizable(false);
    timeline_scroll->setFrameShape(QFrame::NoFrame);
    timeline_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timeline_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timeline_widget_ = new TimelineWidget();
    timeline_widget_->setProject(&project_);
    timeline_widget_->on_media_dropped =
        [this](const QString& media_id, const QString& track_id, std::int64_t timeline_ms) {
            add_media_to_track(media_id, track_id, timeline_ms);
        };
    timeline_widget_->on_clip_selected =
        [this](const QString& track_id, const QString& clip_id) {
            selected_track_id_ = track_id;
            selected_clip_id_ = clip_id;
            if (clip_id.isEmpty()) {
                status_label_->setText("No timeline clip selected.");
                return;
            }
            const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
                [&](const vecut::Track& item) {
                    return QString::fromStdString(item.id) == track_id;
                });
            if (track != project_.tracks().end()) {
                const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
                    [&](const vecut::Clip& item) {
                        return QString::fromStdString(item.id) == clip_id;
                    });
                if (clip != track->clips.end()) {
                    const auto media = std::find_if(project_.media().begin(), project_.media().end(),
                        [&](const vecut::MediaAsset& item) { return item.id == clip->media_id; });
                    if (media != project_.media().end()) {
                        const QString media_id = QString::fromStdString(media->id);
                        load_preview_media(media_id);
                        if (media->kind != vecut::MediaKind::Image)
                            preview_player_->setPosition(clip->source_in_ms);
                    }
                }
            }
            status_label_->setText(QString("Selected clip %1 on %2.")
                .arg(clip_id, track_id));
        };
    timeline_widget_->on_clip_edit_started = [this] { push_undo_state(); };
    timeline_widget_->on_clip_edited =
        [this](const QString& track_id, const QString& clip_id, std::int64_t start_ms,
               std::int64_t source_in_ms, std::int64_t source_out_ms) {
            const auto track = track_id.toStdString();
            const auto clip = clip_id.toStdString();
            if (project_.trim_clip(track, clip, source_in_ms, source_out_ms) &&
                project_.move_clip_to_time(track, clip, start_ms)) {
                timeline_widget_->refresh();
            }
        };
    timeline_scroll->setWidget(timeline_widget_);
    timeline_scroll->setMinimumHeight(110);
    timeline_scroll->setMaximumHeight(180);
    timeline_layout->addWidget(timeline_scroll);
    outer->addWidget(timeline_panel_);

    status_label_ = new QLabel("C++17 project engine ready · Import metadata probing enabled",
                               root);
    status_label_->setWordWrap(true);
    status_label_->setStyleSheet("color:#9aa1af;font-size:11px;");
    outer->addWidget(status_label_);

    connect(open_button, &QPushButton::clicked, this, [this] { open_project(); });
    connect(save_button, &QPushButton::clicked, this, [this] { save_project(); });
    connect(import_button, &QPushButton::clicked, this, [this] { import_media(); });
    connect(undo_button_, &QPushButton::clicked, this, [this] { undo_edit(); });
    connect(redo_button_, &QPushButton::clicked, this, [this] { redo_edit(); });
    auto* undo_shortcut = new QShortcut(QKeySequence::Undo, this);
    auto* redo_shortcut = new QShortcut(QKeySequence::Redo, this);
    connect(undo_shortcut, &QShortcut::activated, this, [this] { undo_edit(); });
    connect(redo_shortcut, &QShortcut::activated, this, [this] { redo_edit(); });
    connect(media_list_, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem*, QListWidgetItem*) { update_media_details(); });
    connect(add_video_button_, &QPushButton::clicked, this,
            [this] { add_track(vecut::TrackKind::Video); });
    connect(add_audio_button_, &QPushButton::clicked, this,
            [this] { add_track(vecut::TrackKind::Audio); });
    connect(split_button_, &QPushButton::clicked, this,
            [this] { split_selected_clip(); });
    connect(trim_button_, &QPushButton::clicked, this,
            [this] { trim_selected_clip(); });
    connect(transition_button_, &QPushButton::clicked, this,
            [this] { edit_selected_transition(); });
    connect(volume_button_, &QPushButton::clicked, this,
            [this] { edit_selected_volume(); });
    connect(timeline_audio_button_, &QPushButton::clicked, this,
            [this] { toggle_timeline_audio(); });
    audio_mix_timer_ = new QTimer(this);
    audio_mix_timer_->setInterval(10);
    connect(audio_mix_timer_, &QTimer::timeout, this,
            [this] { update_timeline_audio(); });
    connect(remove_clip_button_, &QPushButton::clicked, this,
            [this] { remove_selected_clip(); });

    setStyleSheet(
        "QMainWindow,#root{background:#100e16;}"
        "QFrame#panel{background:#1b1922;border:1px solid #393742;border-radius:14px;}"
        "QListWidget{background:#15131b;border:1px solid #393742;border-radius:10px;"
        "color:#e4e7ef;padding:6px;}"
        "QListWidget::item{padding:8px;border-radius:5px;}"
        "QListWidget::item:selected{background:#168a36;color:#ffffff;}"
        "QPushButton{background:#25232d;color:#f4f5fa;border:1px solid #393742;"
        "border-radius:10px;padding:9px 12px;font-weight:650;}"
        "QPushButton:hover{background:#30303a;border-color:#39d353;}"
        "QPushButton:pressed{background:#168a36;border-color:#39d353;color:#ffffff;}"
        "QPushButton#primaryButton{background:#39d353;border-color:#39d353;color:#101010;}"
        "QPushButton#primaryButton:hover{background:#55e66c;border-color:#55e66c;}"
    );
    // The mobile-first home screen is the app's landing page. The existing
    // editor remains intact as the second page in the stack.
    app_stack_ = new QStackedWidget(this);
    home_page_ = new QWidget(app_stack_);
    home_page_->setObjectName("homePage");
    auto* home_layout = new QVBoxLayout(home_page_);
    home_layout->setContentsMargins(16, 12, 16, 10);
    home_layout->setSpacing(12);

    auto* home_header = new QHBoxLayout();
    auto* home_brand = new QLabel("VeCut", home_page_);
    home_brand->setStyleSheet("font-size:25px;font-weight:900;color:#f4f5fa;");
    home_header->addWidget(home_brand);
    home_header->addStretch();
    auto* search_button = new QPushButton(home_page_);
    auto* help_button = new QPushButton(home_page_);
    auto* settings_button = new QPushButton(home_page_);
    search_button->setIcon(vecut_icon(VeCutIcon::Search));
    help_button->setIcon(vecut_icon(VeCutIcon::Help));
    settings_button->setIcon(vecut_icon(VeCutIcon::Settings));
    for (auto* button : {search_button, help_button, settings_button}) {
        button->setFixedSize(42, 42);
        button->setIconSize(QSize(23, 23));
        button->setStyleSheet("QPushButton{background:#191d24;border:1px solid #303843;border-radius:12px;padding:0;} QPushButton:hover{border-color:#39d353;} QPushButton:pressed{background:#168a36;border-color:#39d353;color:#ffffff;}");
        home_header->addWidget(button);
    }
    home_layout->addLayout(home_header);

    auto* home_scroll = new QScrollArea(home_page_);
    home_scroll->setWidgetResizable(true);
    home_scroll->setFrameShape(QFrame::NoFrame);
    home_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* home_content = new QWidget(home_scroll);
    auto* content_layout = new QVBoxLayout(home_content);
    content_layout->setContentsMargins(0, 2, 0, 8);
    content_layout->setSpacing(18);

    auto* start_cards = new QHBoxLayout();
    start_cards->setSpacing(12);
    auto* new_video = new QPushButton("New video", home_content);
    new_video->setIcon(vecut_icon(VeCutIcon::Video));
    new_video->setIconSize(QSize(38, 38));
    new_video->setObjectName("homePrimary");
    new_video->setMinimumHeight(132);
    new_video->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* edit_photo = new QPushButton("Edit photo", home_content);
    edit_photo->setIcon(vecut_icon(VeCutIcon::Photo));
    edit_photo->setIconSize(QSize(34, 34));
    edit_photo->setObjectName("homePrimary");
    edit_photo->setMinimumHeight(132);
    edit_photo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    start_cards->addWidget(new_video, 3);
    start_cards->addWidget(edit_photo, 2);
    content_layout->addLayout(start_cards);

    auto* tools_grid = new QGridLayout();
    tools_grid->setHorizontalSpacing(10);
    tools_grid->setVerticalSpacing(14);
    const QStringList tool_names = {
        "AutoCut", "Retouch", "AI story maker", "Desktop editor",
        "Auto captions", "AI generator", "Remove background", "All tools"
    };
    const QList<VeCutIcon> tool_icons = {
        VeCutIcon::AutoCut, VeCutIcon::Retouch, VeCutIcon::Story, VeCutIcon::Desktop,
        VeCutIcon::Captions, VeCutIcon::Sparkles, VeCutIcon::RemoveBackground, VeCutIcon::Grid
    };
    for (int i = 0; i < tool_names.size(); ++i) {
        auto* tile = new QWidget(home_content);
        auto* tile_layout = new QVBoxLayout(tile);
        tile_layout->setContentsMargins(0, 0, 0, 0);
        tile_layout->setSpacing(5);
        auto* tile_button = new QPushButton(tile);
        tile_button->setIcon(vecut_icon(tool_icons[i]));
        tile_button->setIconSize(QSize(30, 30));
        tile_button->setMinimumHeight(66);
        tile_button->setStyleSheet("QPushButton{background:#191f1d;border:1px solid #2c4637;border-radius:13px;color:#39d353;} QPushButton:hover{border-color:#39d353;} QPushButton:pressed{background:#168a36;border-color:#39d353;color:#ffffff;}");
        auto* tile_label = new QLabel(tool_names[i], tile);
        tile_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        tile_label->setWordWrap(true);
        tile_label->setStyleSheet("color:#d5ddd8;font-size:11px;font-weight:650;");
        tile_layout->addWidget(tile_button);
        tile_layout->addWidget(tile_label);
        tools_grid->addWidget(tile, i / 4, i % 4);
        connect(tile_button, &QPushButton::clicked, this, [this, name = tool_names[i]] {
            QMessageBox::information(this, name, name + " is not available yet.");
        });
    }
    content_layout->addLayout(tools_grid);

    auto* projects_header = new QHBoxLayout();
    auto* projects_title = new QLabel("Projects", home_content);
    projects_title->setStyleSheet("font-size:21px;font-weight:800;color:#f4f5fa;");
    projects_header->addWidget(projects_title);
    projects_header->addStretch();
    auto* project_sort = new QPushButton(home_content);
    project_sort->setIcon(vecut_icon(VeCutIcon::Sort));
    project_sort->setToolTip("Sort projects");
    project_sort->setIconSize(QSize(22, 22));
    project_sort->setStyleSheet("QPushButton{background:transparent;border:0;padding:6px;}");
    projects_header->addWidget(project_sort);
    content_layout->addLayout(projects_header);

    auto* empty_projects = new QWidget(home_content);
    auto* empty_layout = new QVBoxLayout(empty_projects);
    empty_layout->setContentsMargins(0, 28, 0, 30);
    empty_layout->setSpacing(10);
    auto* empty_icon = new QLabel(empty_projects);
    empty_icon->setPixmap(vecut_icon(VeCutIcon::FolderPlus).pixmap(56, 56));
    empty_icon->setAlignment(Qt::AlignCenter);
    auto* empty_text = new QLabel("Your projects will appear here.
Start creating now.", empty_projects);
    empty_text->setAlignment(Qt::AlignCenter);
    empty_text->setWordWrap(true);
    empty_text->setStyleSheet("font-size:15px;color:#929b98;");
    empty_layout->addWidget(empty_icon);
    empty_layout->addWidget(empty_text);
    content_layout->addWidget(empty_projects);
    content_layout->addStretch();
    home_scroll->setWidget(home_content);
    home_layout->addWidget(home_scroll, 1);

    auto* bottom_nav = new QHBoxLayout();
    bottom_nav->setContentsMargins(0, 4, 0, 0);
    const QStringList nav_names = {"Edit", "Templates", "AI Lab", "Inbox", "Me"};
    const QList<VeCutIcon> nav_icons = {
        VeCutIcon::Edit, VeCutIcon::Template, VeCutIcon::Sparkles,
        VeCutIcon::Inbox, VeCutIcon::User
    };
    for (int i = 0; i < nav_names.size(); ++i) {
        auto* nav_button = new QPushButton(nav_names[i], home_page_);
        nav_button->setIcon(vecut_icon(nav_icons[i]));
        nav_button->setIconSize(QSize(20, 20));
        nav_button->setMinimumHeight(56);
        nav_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        nav_button->setStyleSheet(i == 0
            ? "QPushButton{background:transparent;border:0;color:#39d353;font-size:12px;font-weight:800;}"
            : "QPushButton{background:transparent;border:0;color:#929b98;font-size:11px;}");
        bottom_nav->addWidget(nav_button);
        if (i != 0) {
            connect(nav_button, &QPushButton::clicked, this, [this, name = nav_names[i]] {
                QMessageBox::information(this, name.section('
', 1), "This section is coming soon.");
            });
        }
    }
    home_layout->addLayout(bottom_nav);

    home_page_->setStyleSheet(
        "QWidget#homePage{background:#100e16;color:#f4f5fa;}"
        "QPushButton#homePrimary{background:#18251d;border:1px solid #39d353;border-radius:17px;color:#f4f5fa;font-size:18px;font-weight:800;padding:12px;text-align:left;}"
        "QPushButton#homePrimary:hover{background:#203629;border-color:#55e66c;}"
        "QPushButton#homePrimary:pressed{background:#168a36;color:#ffffff;}"
    );
    app_stack_->addWidget(home_page_);
    app_stack_->addWidget(root);
    app_stack_->setCurrentWidget(home_page_);
    setCentralWidget(app_stack_);

    connect(new_video, &QPushButton::clicked, this, [this] {
        app_stack_->setCurrentWidget(centralWidget()->findChild<QWidget*>("root"));
        import_media();
    });
    connect(edit_photo, &QPushButton::clicked, this, [this] {
        app_stack_->setCurrentWidget(centralWidget()->findChild<QWidget*>("root"));
        import_media();
    });
    connect(search_button, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, "Search", "Project search will be available soon.");
    });
    connect(help_button, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, "Help", "VeCut help is coming soon.");
    });
    connect(settings_button, &QPushButton::clicked, this, [this] {
        QMessageBox::information(this, "Settings", "VeCut settings are coming soon.");
    });
    apply_responsive_layout();
}

void MainWindow::push_undo_state() {
    undo_stack_.push_back(project_);
    constexpr std::size_t kMaximumHistory = 100;
    if (undo_stack_.size() > kMaximumHistory) undo_stack_.erase(undo_stack_.begin());
    redo_stack_.clear();
    update_history_buttons();
}

void MainWindow::update_history_buttons() {
    if (undo_button_) undo_button_->setEnabled(!undo_stack_.empty());
    if (redo_button_) redo_button_->setEnabled(!redo_stack_.empty());
}

void MainWindow::undo_edit() {
    if (undo_stack_.empty()) return;
    redo_stack_.push_back(project_);
    project_ = undo_stack_.back();
    undo_stack_.pop_back();
    selected_track_id_.clear();
    selected_clip_id_.clear();
    if (timeline_widget_) {
        timeline_widget_->selectClip({});
        timeline_widget_->refresh();
    }
    update_history_buttons();
    render_preview_frame();
    status_label_->setText("Undo complete.");
}

void MainWindow::redo_edit() {
    if (redo_stack_.empty()) return;
    undo_stack_.push_back(project_);
    project_ = redo_stack_.back();
    redo_stack_.pop_back();
    selected_track_id_.clear();
    selected_clip_id_.clear();
    if (timeline_widget_) {
        timeline_widget_->selectClip({});
        timeline_widget_->refresh();
    }
    update_history_buttons();
    render_preview_frame();
    status_label_->setText("Redo complete.");
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    apply_responsive_layout();
    render_preview_frame();
}

void MainWindow::apply_responsive_layout() {
    if (!workspace_ || !timeline_panel_) return;

    // A narrow window uses a portrait stack. Wider windows use the desktop
    // editing arrangement with the media bin beside the preview.
    const bool use_portrait_layout = width() < kPortraitBreakpoint;
    if (responsive_layout_initialized_ && use_portrait_layout == portrait_layout_) return;

    portrait_layout_ = use_portrait_layout;
    responsive_layout_initialized_ = true;

    if (portrait_layout_) {
        workspace_->setOrientation(Qt::Vertical);
        workspace_->setSizes({145, 300});
        timeline_panel_->setMaximumHeight(190);
        add_video_button_->setText("Video");
        add_audio_button_->setText("Audio");
        status_label_->setMaximumHeight(34);
    } else {
        workspace_->setOrientation(Qt::Horizontal);
        workspace_->setSizes({280, 820});
        timeline_panel_->setMaximumHeight(230);
        add_video_button_->setText("Video track");
        add_audio_button_->setText("Audio track");
        status_label_->setMaximumHeight(24);
    }
}

void MainWindow::import_media() {
#if defined(Q_OS_ANDROID)
    const jint sdkVersion = QNativeInterface::QAndroidApplication::sdkVersion();
    QStringList permissions;
    if (sdkVersion >= 33) {
        permissions = {
            "android.permission.READ_MEDIA_IMAGES",
            "android.permission.READ_MEDIA_VIDEO",
            "android.permission.READ_MEDIA_AUDIO"
        };
    } else {
        permissions = {"android.permission.READ_EXTERNAL_STORAGE"};
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid()) {
        status_label_->setText("Could not access Android permission service.");
        return;
    }

    QJniEnvironment env;
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray permissionArray = env->NewObjectArray(
        permissions.size(), stringClass, nullptr);
    for (int i = 0; i < permissions.size(); ++i) {
        const QJniObject permission = QJniObject::fromString(permissions.at(i));
        env->SetObjectArrayElement(permissionArray, i, permission.object<jstring>());
    }
    activity.callMethod<void>("requestPermissions",
                              "([Ljava/lang/String;I)V",
                              permissionArray, 7351);
    env->DeleteLocalRef(permissionArray);
    env->DeleteLocalRef(stringClass);

    auto* permissionPoll = new QTimer(this);
    QElapsedTimer wait;
    wait.start();
    connect(permissionPoll, &QTimer::timeout, this,
            [this, permissionPoll, permissions, activity, wait]() mutable {
        bool granted = true;
        for (const QString& name : permissions) {
            const QJniObject permission = QJniObject::fromString(name);
            const jint result = activity.callMethod<jint>(
                "checkSelfPermission", "(Ljava/lang/String;)I",
                permission.object<jstring>());
            if (result != 0) {
                granted = false;
                break;
            }
        }
        if (granted) {
            permissionPoll->stop();
            permissionPoll->deleteLater();
            show_media_file_picker();
        } else if (wait.elapsed() >= 60000) {
            permissionPoll->stop();
            permissionPoll->deleteLater();
            status_label_->setText("Media access was not granted.");
            QMessageBox::information(
                this, "Media access needed",
                "Allow VeCut to access photos, videos and audio so you can import them. "
                "You can change this permission in Android Settings.");
        }
    });
    permissionPoll->start(350);
    return;
#else
    show_media_file_picker();
#endif
}

void MainWindow::show_media_file_picker() {
    struct GalleryEntry {
        QString path;
        QFileInfo info;
        vecut::MediaKind kind;
    };

    QMap<QString, GalleryEntry> unique_entries;
#if defined(Q_OS_ANDROID)
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    const jobjectArray scanned = QJniObject::callStaticMethod<jobjectArray>(
        "com/mrcreepertamil/vecut/MediaScanner", "scan",
        "(Landroid/content/Context;)[Ljava/lang/String;", context.object());
    if (scanned) {
        QJniEnvironment env;
        const jsize count = env->GetArrayLength(scanned);
        for (jsize i = 0; i < count; ++i) {
            const auto value = static_cast<jstring>(env->GetObjectArrayElement(scanned, i));
            const QString path = QJniObject::fromLocalRef(value).toString();
            const QFileInfo info(path);
            if (!path.isEmpty() && info.exists() && is_supported_media_file(info))
                unique_entries.insert(path, GalleryEntry{path, info, media_kind_for_file(info)});
        }
        env->DeleteLocalRef(scanned);
    }
#else
    QStringList scan_roots;
    scan_roots << QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
               << QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
               << QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
               << QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
               << QDir::homePath();
    scan_roots.removeDuplicates();

    for (const QString& root : scan_roots) {
        if (root.isEmpty() || !QDir(root).exists()) continue;
        QDirIterator iterator(root,
            QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const QString path = iterator.next();
            const QFileInfo info(path);
            if (!is_supported_media_file(info)) continue;
            const QString key = info.canonicalFilePath().isEmpty()
                ? info.absoluteFilePath() : info.canonicalFilePath();
            if (!unique_entries.contains(key))
                unique_entries.insert(key, GalleryEntry{key, QFileInfo(key),
                                                         media_kind_for_file(info)});
        }
    }
#endif

    QList<GalleryEntry> entries = unique_entries.values();
    std::sort(entries.begin(), entries.end(),
        [](const GalleryEntry& a, const GalleryEntry& b) {
            return a.info.lastModified() > b.info.lastModified();
        });

    QDialog dialog(this);
    dialog.setWindowTitle("Choose media");
    dialog.setMinimumSize(300, 460);
    dialog.resize(std::min(520, std::max(320, width() - 24)),
                  std::min(780, std::max(480, height() - 40)));
    dialog.setStyleSheet(
        "QDialog{background:#100E16;color:#f4f5fa;}"
        "QLabel{color:#f4f5fa;}"
        "QComboBox,QTabBar::tab{background:#1b1a22;color:#f4f5fa;"
        "border:1px solid #34313d;border-radius:7px;padding:7px;}"
        "QTabBar::tab:selected{background:#168A36;border-color:#39D353;}"
        "QListWidget{background:#100E16;border:0;}"
        "QListWidget::item{background:#1b1a22;color:#f4f5fa;"
        "border:1px solid #34313d;border-radius:7px;padding:5px;}"
        "QListWidget::item:hover{border:1px solid #39D353;}"
        "QPushButton{background:#24222c;color:#f4f5fa;border:0;"
        "border-radius:8px;padding:9px;}"
        "QPushButton#nextButton{background:#39D353;color:#07120a;font-weight:800;}"
    );

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto* top_row = new QHBoxLayout();
    auto* title = new QLabel("Recent", &dialog);
    title->setStyleSheet("font-size:18px;font-weight:800;");
    auto* folder_filter = new QComboBox(&dialog);
    folder_filter->addItems({"Recent", "All folders", "Camera", "Pictures",
                             "Movies", "Downloads", "Music"});
    folder_filter->setMinimumWidth(130);
    top_row->addWidget(title);
    top_row->addStretch();
    top_row->addWidget(folder_filter);
    layout->addLayout(top_row);

    auto* material_label = new QLabel("Material", &dialog);
    material_label->setStyleSheet("color:#39D353;font-weight:700;");
    layout->addWidget(material_label);

    auto* tabs = new QTabWidget(&dialog);
    auto* video_list = new QListWidget(tabs);
    auto* photo_list = new QListWidget(tabs);
    auto* all_list = new QListWidget(tabs);
    const QList<QListWidget*> lists{video_list, photo_list, all_list};
    for (auto* list : lists) {
        list->setViewMode(QListView::IconMode);
        list->setFlow(QListView::LeftToRight);
        list->setWrapping(true);
        list->setResizeMode(QListView::Adjust);
        list->setMovement(QListView::Static);
        list->setSpacing(7);
        list->setIconSize(QSize(92, 76));
        list->setGridSize(QSize(108, 124));
        list->setWordWrap(true);
        list->setSelectionMode(QAbstractItemView::NoSelection);
    }
    tabs->addTab(video_list, "VIDEO");
    tabs->addTab(photo_list, "PHOTO");
    tabs->addTab(all_list, "ALL");
    layout->addWidget(tabs, 1);

    auto* selection_label = new QLabel("0 selected", &dialog);
    selection_label->setStyleSheet("font-weight:700;");
    layout->addWidget(selection_label);

    auto* selected_scroll = new QScrollArea(&dialog);
    selected_scroll->setWidgetResizable(true);
    selected_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    selected_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    selected_scroll->setFixedHeight(78);
    selected_scroll->setStyleSheet("QScrollArea{background:#17151e;border:0;border-radius:7px;}");
    auto* selected_strip = new QWidget(selected_scroll);
    auto* selected_layout = new QHBoxLayout(selected_strip);
    selected_layout->setContentsMargins(6, 4, 6, 4);
    selected_layout->setSpacing(6);
    selected_scroll->setWidget(selected_strip);
    layout->addWidget(selected_scroll);

    auto* preview_label = new QLabel("Tap media to add it to your selection", &dialog);
    preview_label->setStyleSheet("color:#a8a5b2;font-size:11px;");
    preview_label->setWordWrap(true);
    layout->addWidget(preview_label);

    auto* bottom_row = new QHBoxLayout();
    auto* cancel_button = new QPushButton("Cancel", &dialog);
    auto* next_button = new QPushButton("Next", &dialog);
    next_button->setObjectName("nextButton");
    next_button->setEnabled(false);
    bottom_row->addWidget(cancel_button);
    bottom_row->addStretch();
    bottom_row->addWidget(next_button);
    layout->addLayout(bottom_row);

    QMap<QString, QPixmap> thumbnail_cache;
    for (const GalleryEntry& entry : entries) {
        const QString name = entry.info.fileName();
        const QString item_text = entry.kind == vecut::MediaKind::Video
            ? name + "
VIDEO" : (entry.kind == vecut::MediaKind::Image
                ? name + "
PHOTO" : name + "
AUDIO");
        for (auto* list : lists) {
            auto* item = new QListWidgetItem(item_text);
            item->setData(Qt::UserRole, entry.path);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            if (entry.kind == vecut::MediaKind::Image) {
                const QImage thumb = vecut::MediaProbe::image_thumbnail(
                    entry.path, QSize(184, 152));
                if (!thumb.isNull()) {
                    const QPixmap pixmap = QPixmap::fromImage(thumb);
                    item->setIcon(QIcon(pixmap));
                    thumbnail_cache.insert(entry.path, pixmap);
                }
            } else {
                item->setIcon(style()->standardIcon(entry.kind == vecut::MediaKind::Video
                    ? QStyle::SP_MediaPlay : QStyle::SP_FileIcon));
            }
            list->addItem(item);
            if (list == video_list && entry.kind != vecut::MediaKind::Video) item->setHidden(true);
            if (list == photo_list && entry.kind != vecut::MediaKind::Image) item->setHidden(true);
        }
    }

    auto update_selection = [&]() {
        QMap<QString, bool> selected;
        for (auto* list : lists) {
            for (int i = 0; i < list->count(); ++i) {
                auto* item = list->item(i);
                const QString path = item->data(Qt::UserRole).toString();
                if (item->checkState() == Qt::Checked) selected.insert(path, true);
            }
        }
        selection_label->setText(QString("%1 selected").arg(selected.size()));
        next_button->setEnabled(!selected.isEmpty());

        while (QLayoutItem* child = selected_layout->takeAt(0)) {
            if (child->widget()) child->widget()->deleteLater();
            delete child;
        }
        QStringList names;
        for (auto it = selected.cbegin(); it != selected.cend(); ++it) {
            const QString path = it.key();
            const QString name = QFileInfo(path).fileName();
            names << name;
            auto* thumb = new QLabel(selected_strip);
            thumb->setFixedSize(58, 58);
            thumb->setAlignment(Qt::AlignCenter);
            thumb->setStyleSheet("background:#282530;border-radius:6px;");
            thumb->setToolTip(name);
            if (thumbnail_cache.contains(path)) {
                thumb->setPixmap(thumbnail_cache.value(path).scaled(
                    54, 54, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                thumb->setPixmap(style()->standardIcon(
                    media_kind_for_file(QFileInfo(path)) == vecut::MediaKind::Video
                        ? QStyle::SP_MediaPlay : QStyle::SP_FileIcon)
                    .pixmap(30, 30));
            }
            selected_layout->addWidget(thumb);
        }
        selected_layout->addStretch();
        selected_strip->adjustSize();
        preview_label->setText(names.isEmpty()
            ? "Tap media to add it to your selection"
            : names.mid(0, 3).join("   •   ") +
              (names.size() > 3 ? QString("   •   +%1 more").arg(names.size() - 3) : ""));
    };

    for (auto* list : lists) {
        connect(list, &QListWidget::itemChanged, &dialog,
                [&, list](QListWidgetItem* changed) {
            const QString path = changed->data(Qt::UserRole).toString();
            for (auto* other : lists) {
                if (other == list) continue;
                for (int i = 0; i < other->count(); ++i) {
                    auto* item = other->item(i);
                    if (item->data(Qt::UserRole).toString() == path &&
                        item->checkState() != changed->checkState()) {
                        const QSignalBlocker blocker(other);
                        item->setCheckState(changed->checkState());
                        break;
                    }
                }
            }
            update_selection();
        });
    }

    auto apply_filters = [&]() {
        const QString folder = folder_filter->currentText();
        const int tab_index = tabs->currentIndex();
        for (auto* list : lists) {
            for (int i = 0; i < list->count(); ++i) {
                auto* item = list->item(i);
                const QString path = item->data(Qt::UserRole).toString();
                const QString lower = QFileInfo(path).absolutePath().toLower();
                const auto kind = media_kind_for_file(QFileInfo(path));
                bool visible = tab_index == 2 ||
                    (tab_index == 0 && kind == vecut::MediaKind::Video) ||
                    (tab_index == 1 && kind == vecut::MediaKind::Image);
                if (folder == "Camera") visible = visible && lower.contains("/dcim");
                else if (folder == "Pictures") visible = visible && lower.contains("picture");
                else if (folder == "Movies") visible = visible && lower.contains("movie");
                else if (folder == "Downloads") visible = visible && lower.contains("download");
                else if (folder == "Music") visible = visible &&
                    (lower.contains("music") || lower.contains("recording"));
                item->setHidden(!visible);
            }
        }
    };
    connect(folder_filter, &QComboBox::currentTextChanged, &dialog,
            [&, apply_filters](const QString&) { apply_filters(); });
    connect(tabs, &QTabWidget::currentChanged, &dialog,
            [&, apply_filters](int) { apply_filters(); });

    // Decode video thumbnails one at a time to avoid opening a decoder for every
    // gallery item. The picker remains usable while thumbnails are being prepared.
    QStringList video_paths;
    for (const GalleryEntry& entry : entries)
        if (entry.kind == vecut::MediaKind::Video) video_paths << entry.path;

    auto* thumbnail_player = new QMediaPlayer(&dialog);
    auto* thumbnail_sink = new QVideoSink(&dialog);
    thumbnail_player->setVideoSink(thumbnail_sink);
    int next_video_index = 0;
    QString active_video_path;
    bool thumbnail_decoded_for_active = false;
    std::function<void()> start_next_video;
    start_next_video = [&]() {
        if (next_video_index >= video_paths.size()) return;
        active_video_path = video_paths.at(next_video_index++);
        thumbnail_decoded_for_active = false;
        thumbnail_player->stop();
        thumbnail_player->setSource(QUrl::fromLocalFile(active_video_path));
        thumbnail_player->play();
        const QString requested_path = active_video_path;
        QTimer::singleShot(2200, &dialog, [&, requested_path]() {
            if (active_video_path != requested_path || thumbnail_decoded_for_active) return;
            thumbnail_player->stop();
            start_next_video();
        });
    };

    auto format_duration = [](qint64 milliseconds) {
        const qint64 seconds = std::max<qint64>(0, milliseconds / 1000);
        return QString("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QLatin1Char('0'));
    };
    connect(thumbnail_player, &QMediaPlayer::durationChanged, &dialog,
            [&, format_duration](qint64 duration) {
        if (active_video_path.isEmpty() || duration <= 0) return;
        const QString label = format_duration(duration);
        for (auto* list : lists) {
            for (int i = 0; i < list->count(); ++i) {
                auto* item = list->item(i);
                if (item->data(Qt::UserRole).toString() == active_video_path)
                    item->setText(QFileInfo(active_video_path).fileName() + "
" + label);
            }
        }
    });
    connect(thumbnail_sink, &QVideoSink::videoFrameChanged, &dialog,
            [&](const QVideoFrame& frame) {
        if (active_video_path.isEmpty() || thumbnail_decoded_for_active) return;
        const QImage image = frame.toImage();
        if (image.isNull()) return;
        const QPixmap pixmap = QPixmap::fromImage(image.scaled(
            184, 152, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        thumbnail_decoded_for_active = true;
        thumbnail_cache.insert(active_video_path, pixmap);
        for (auto* list : lists) {
            for (int i = 0; i < list->count(); ++i) {
                auto* item = list->item(i);
                if (item->data(Qt::UserRole).toString() == active_video_path)
                    item->setIcon(QIcon(pixmap));
            }
        }
        thumbnail_player->stop();
        const QString completed_path = active_video_path;
        QTimer::singleShot(0, &dialog, [&, completed_path]() {
            if (active_video_path == completed_path) start_next_video();
        });
    });
    if (!video_paths.isEmpty())
        QTimer::singleShot(0, &dialog, [&]() { start_next_video(); });

    connect(cancel_button, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(next_button, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (entries.isEmpty()) {
        selection_label->setText("No media found in accessible folders.");
        preview_label->setText("Add photos or videos to your device's shared media folders, then scan again.");
    }
    if (dialog.exec() != QDialog::Accepted) return;

    QStringList paths;
    for (auto* list : lists) {
        for (int i = 0; i < list->count(); ++i) {
            auto* item = list->item(i);
            if (item->checkState() == Qt::Checked)
                paths << item->data(Qt::UserRole).toString();
        }
    }
    paths.removeDuplicates();

    int imported = 0;
    for (const QString& path : paths) {
        const QFileInfo info(path);
        if (!is_supported_media_file(info)) {
            status_label_->setText(QString("Skipped unsupported file: %1").arg(info.fileName()));
            continue;
        }
        vecut::MediaAsset asset;
        asset.id = "media-" + std::to_string(next_media_id_++);
        asset.uri = info.absoluteFilePath().toStdString();
        asset.display_name = info.fileName().toStdString();
        asset.kind = media_kind_for_file(info);
        asset.duration_ms = asset.kind == vecut::MediaKind::Image
            ? kStillImageClipDurationMs : 0;
        asset.file_size_bytes = static_cast<std::uint64_t>(std::max<qint64>(0, info.size()));

        if (asset.kind == vecut::MediaKind::Image) {
            const auto image_info = vecut::MediaProbe::inspect_image(path);
            if (image_info.valid) {
                asset.width = image_info.width;
                asset.height = image_info.height;
            }
        }

        const QString media_id = QString::fromStdString(asset.id);
        const auto media_kind = asset.kind;
        if (!project_.add_media(std::move(asset))) continue;

        auto* item = new QListWidgetItem(
            media_kind_name(media_kind) + "  ·  " + info.fileName(), media_list_);
        item->setData(Qt::UserRole, media_id);
        if (media_kind == vecut::MediaKind::Image) {
            const QImage thumbnail = vecut::MediaProbe::image_thumbnail(path, QSize(112, 64));
            if (!thumbnail.isNull()) item->setIcon(QIcon(QPixmap::fromImage(thumbnail)));
        }
        probe_media(media_id, path, media_kind);
        ++imported;
    }

    if (imported > 0) {
        status_label_->setText(QString("Imported %1 file(s). Reading metadata…")
                                   .arg(imported));
        if (!media_list_->currentItem()) media_list_->setCurrentRow(0);
    }
}

void MainWindow::add_track(vecut::TrackKind kind) {
    vecut::Track track;
    track.kind = kind;
    if (kind == vecut::TrackKind::Audio) {
        track.name = "Audio " + std::to_string(++audio_track_count_);
    } else {
        track.name = "Video " + std::to_string(++video_track_count_);
    }
    const std::string name = track.name;
    push_undo_state();
    if (!project_.add_track(std::move(track))) {
        status_label_->setText("Could not add track.");
        return;
    }
    update_track_list();
    status_label_->setText(QString::fromStdString(name) + " added.");
}

void MainWindow::update_media_details(bool reload_preview) {
    const auto* item = media_list_->currentItem();
    if (!item) {
        media_details_->setText("No media selected");
        current_preview_frame_ = QImage();
        preview_label_->setPixmap(QPixmap());
        preview_label_->setText("Your preview will appear here

Import media to begin");
        return;
    }

    const QString id = item->data(Qt::UserRole).toString();
    const auto found = std::find_if(project_.media().begin(), project_.media().end(),
        [&](const vecut::MediaAsset& asset) { return QString::fromStdString(asset.id) == id; });
    if (found == project_.media().end()) return;

    const QString duration = found->kind == vecut::MediaKind::Image
        ? QString("Still image · 5s default")
        : (found->duration_ms > 0
            ? QString("%1:%2").arg(found->duration_ms / 60000, 2, 10, QLatin1Char('0'))
                                .arg((found->duration_ms / 1000) % 60, 2, 10, QLatin1Char('0'))
            : QString("Probing…"));
    const QString dimensions = found->width > 0 && found->height > 0
        ? QString("%1 × %2").arg(found->width).arg(found->height)
        : QString("Dimensions unavailable");
    const double size_mb = static_cast<double>(found->file_size_bytes) / (1024.0 * 1024.0);
    media_details_->setText(
        QString("%1
%2
Duration: %3
Size: %4 MB
Dimensions: %5")
            .arg(QString::fromStdString(found->display_name), media_kind_name(found->kind),
                 duration, QString::number(size_mb, 'f', 2), dimensions));
    if (reload_preview) load_preview_media(id);
}

void MainWindow::load_preview_media(const QString& media_id) {
    const auto found = std::find_if(project_.media().begin(), project_.media().end(),
        [&](const vecut::MediaAsset& asset) {
            return QString::fromStdString(asset.id) == media_id;
        });
    if (found == project_.media().end()) return;

    preview_player_ = found->kind == vecut::MediaKind::Audio
        ? audio_preview_->player() : video_preview_->player();
    video_preview_->clear();
    audio_preview_->clear();
    if (preview_effect_timer_) preview_effect_timer_->stop();
    preview_effect_time_ms_ = 0;
    preview_seek_->setRange(0, 0);
    preview_seek_->setValue(0);
    preview_play_button_->setText("Play");
            preview_play_button_->setIcon(vecut_icon(VeCutIcon::Play));

    if (found->kind == vecut::MediaKind::Video) {
        preview_label_->setText("Loading video preview…");
        current_preview_frame_ = QImage();
        preview_stack_->setCurrentWidget(preview_label_);
        preview_player_->setSource(QUrl::fromLocalFile(
            QString::fromStdString(found->uri)));
    } else if (found->kind == vecut::MediaKind::Audio) {
        current_preview_frame_ = QImage();
        preview_label_->setPixmap(QPixmap());
        preview_label_->setText(QString("AUDIO PREVIEW

%1")
            .arg(QString::fromStdString(found->display_name)));
        preview_stack_->setCurrentWidget(preview_label_);
        preview_player_->setSource(QUrl::fromLocalFile(
            QString::fromStdString(found->uri)));
    } else {
        preview_player_->setSource(QUrl());
        std::int64_t still_duration = 5000;
        const auto selected_track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
            [&](const vecut::Track& item) {
                return QString::fromStdString(item.id) == selected_track_id_;
            });
        if (selected_track != project_.tracks().end()) {
            const auto selected_clip = std::find_if(selected_track->clips.begin(),
                selected_track->clips.end(), [&](const vecut::Clip& item) {
                    return QString::fromStdString(item.id) == selected_clip_id_;
                });
            if (selected_clip != selected_track->clips.end())
                still_duration = selected_clip->source_out_ms - selected_clip->source_in_ms;
        }
        preview_seek_->setRange(0, static_cast<int>(std::min<std::int64_t>(
            still_duration, std::numeric_limits<int>::max())));
        const QImage image = vecut::MediaProbe::load_image_preview(
            QString::fromStdString(found->uri));
        if (!image.isNull()) {
            current_preview_frame_ = image;
            preview_label_->setText(QString());
            render_preview_frame();
        } else {
            current_preview_frame_ = QImage();
            preview_label_->setPixmap(QPixmap());
            preview_label_->setText("Could not load image preview.");
        }
        preview_stack_->setCurrentWidget(preview_label_);
    }
}

void MainWindow::render_preview_frame() {
    if (current_preview_frame_.isNull() || !gpu_preview_) return;

    QSize target = gpu_preview_->contentsRect().size();
    if (target.width() < 2 || target.height() < 2)
        target = QSize(640, 360);

    const vecut::Clip* selected_clip = nullptr;
    if (!selected_track_id_.isEmpty() && !selected_clip_id_.isEmpty()) {
        const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
            [&](const vecut::Track& item) {
                return QString::fromStdString(item.id) == selected_track_id_;
            });
        if (track != project_.tracks().end() && track->kind != vecut::TrackKind::Audio) {
            const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
                [&](const vecut::Clip& item) {
                    return QString::fromStdString(item.id) == selected_clip_id_;
                });
            if (clip != track->clips.end()) selected_clip = &*clip;
        }
    }

    qint64 clip_elapsed_ms = preview_effect_time_ms_;
    if (selected_clip) {
        const qint64 source_time = preview_player_ &&
            !preview_player_->source().isEmpty()
            ? preview_player_->position()
            : selected_clip->source_in_ms + preview_effect_time_ms_;
        clip_elapsed_ms = std::max<qint64>(
            0, source_time - selected_clip->source_in_ms);
    }

    PreviewGpuEffects effects;
    if (selected_clip) {
        const auto& transition = selected_clip->incoming_transition;
        const qint64 duration = selected_clip->source_out_ms - selected_clip->source_in_ms;
        if (transition.fade_in_ms > 0)
            effects.opacity = std::min(effects.opacity, static_cast<float>(
                std::clamp(static_cast<double>(clip_elapsed_ms) / transition.fade_in_ms, 0.0, 1.0)));
        if (transition.fade_out_ms > 0)
            effects.opacity = std::min(effects.opacity, static_cast<float>(
                std::clamp(static_cast<double>(duration - clip_elapsed_ms) / transition.fade_out_ms, 0.0, 1.0)));

        if (transition.duration_ms > 0 && clip_elapsed_ms < transition.duration_ms) {
            const float progress = static_cast<float>(std::clamp(
                static_cast<double>(clip_elapsed_ms) / transition.duration_ms, 0.0, 1.0));
            switch (transition.kind) {
            case vecut::TransitionKind::Fade:
            case vecut::TransitionKind::Dissolve:
                effects.opacity = std::min(effects.opacity, progress);
                break;
            case vecut::TransitionKind::WipeLeftToRight:
                effects.reveal = progress;
                effects.wipe_direction = 1;
                break;
            case vecut::TransitionKind::WipeRightToLeft:
                effects.reveal = progress;
                effects.wipe_direction = 2;
                break;
            case vecut::TransitionKind::SlideLeftToRight:
                effects.slide = (1.0f - progress);
                break;
            case vecut::TransitionKind::SlideRightToLeft:
                effects.slide = -(1.0f - progress);
                break;
            case vecut::TransitionKind::None:
                break;
            }
        }

        const auto& animation = selected_clip->animation;
        if (animation.kind != vecut::AnimationKind::None && animation.duration_ms > 0) {
            const float progress = static_cast<float>(std::clamp(
                static_cast<double>(clip_elapsed_ms) / animation.duration_ms, 0.0, 1.0));
            switch (animation.kind) {
            case vecut::AnimationKind::ZoomIn: effects.zoom = 1.0f + 0.35f * progress; break;
            case vecut::AnimationKind::ZoomOut: effects.zoom = 1.35f - 0.35f * progress; break;
            case vecut::AnimationKind::PanLeft: effects.pan_x = 0.22f * progress; break;
            case vecut::AnimationKind::PanRight: effects.pan_x = -0.22f * progress; break;
            case vecut::AnimationKind::PanUp: effects.pan_y = 0.22f * progress; break;
            case vecut::AnimationKind::PanDown: effects.pan_y = -0.22f * progress; break;
            case vecut::AnimationKind::RotateClockwise: effects.rotation_degrees = 12.0f * progress; break;
            case vecut::AnimationKind::RotateCounterClockwise: effects.rotation_degrees = -12.0f * progress; break;
            case vecut::AnimationKind::None: break;
            }
        }
    }

    gpu_preview_->set_effects(effects);
    gpu_preview_->set_frame(current_preview_frame_);
    preview_stack_->setCurrentWidget(gpu_preview_);
}

void MainWindow::probe_media(const QString& media_id, const QString& path,
                             vecut::MediaKind kind) {
    if (kind == vecut::MediaKind::Image) {
        const auto found = std::find_if(project_.media().begin(), project_.media().end(),
            [&](const vecut::MediaAsset& asset) {
                return QString::fromStdString(asset.id) == media_id;
            });
        if (found != project_.media().end()) {
            project_.update_media_info(found->id, found->duration_ms,
                                       found->width, found->height);
            refresh_media_item(media_id);
        }
        return;
    }

    auto* player = new QMediaPlayer(this);
    active_probes_.push_back(player);
    connect(player, &QMediaPlayer::durationChanged, this,
            [this, player, media_id](qint64 duration_ms) {
        if (duration_ms <= 0) return;
        const auto found = std::find_if(project_.media().begin(), project_.media().end(),
            [&](const vecut::MediaAsset& asset) {
                return QString::fromStdString(asset.id) == media_id;
            });
        if (found != project_.media().end()) {
            project_.update_media_info(found->id, duration_ms, found->width, found->height);
            refresh_media_item(media_id);
            status_label_->setText(QString("Media metadata ready: %1")
                                       .arg(QString::fromStdString(found->display_name)));
        }
        active_probes_.erase(std::remove(active_probes_.begin(), active_probes_.end(), player),
                             active_probes_.end());
        player->deleteLater();
    });
    connect(player, &QMediaPlayer::errorOccurred, this,
            [this, player, media_id](QMediaPlayer::Error, const QString& error) {
        status_label_->setText(QString("Could not probe media: %1").arg(error));
        active_probes_.erase(std::remove(active_probes_.begin(), active_probes_.end(), player),
                             active_probes_.end());
        player->deleteLater();
        refresh_media_item(media_id);
    });
    player->setSource(QUrl::fromLocalFile(path));
}

void MainWindow::refresh_media_item(const QString& media_id) {
    for (int row = 0; row < media_list_->count(); ++row) {
        auto* item = media_list_->item(row);
        if (item->data(Qt::UserRole).toString() == media_id) {
            const auto found = std::find_if(project_.media().begin(), project_.media().end(),
                [&](const vecut::MediaAsset& asset) {
                    return QString::fromStdString(asset.id) == media_id;
                });
            if (found != project_.media().end()) {
                item->setText(media_kind_name(found->kind) + "  ·  " +
                              QString::fromStdString(found->display_name));
                if (media_list_->currentItem() == item) update_media_details(false);
            }
            break;
        }
    }
}

void MainWindow::add_media_to_track(const QString& media_id, const QString& track_id,
                                    std::int64_t timeline_start_ms) {
    const auto media = std::find_if(project_.media().begin(), project_.media().end(),
        [&](const vecut::MediaAsset& asset) {
            return QString::fromStdString(asset.id) == media_id;
        });
    const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
        [&](const vecut::Track& item) {
            return QString::fromStdString(item.id) == track_id;
        });
    if (media == project_.media().end() || track == project_.tracks().end()) return;

    const bool is_audio = media->kind == vecut::MediaKind::Audio;
    if ((is_audio && track->kind != vecut::TrackKind::Audio) ||
        (!is_audio && track->kind == vecut::TrackKind::Audio)) {
        status_label_->setText("That media type cannot be dropped on this track.");
        return;
    }

    const std::int64_t clip_duration = media->kind == vecut::MediaKind::Image
        ? kStillImageClipDurationMs : media->duration_ms;
    if (clip_duration <= 0) {
        status_label_->setText("Media duration is not ready yet. Try dropping it again shortly.");
        return;
    }

    vecut::Clip clip;
    clip.id = "clip-" + std::to_string(next_clip_id_++);
    clip.media_id = media->id;
    clip.source_in_ms = 0;
    clip.source_out_ms = clip_duration;
    clip.timeline_start_ms = timeline_start_ms;
    push_undo_state();
    if (!project_.add_clip(track->id, std::move(clip))) {
        status_label_->setText("Could not add this media clip.");
        return;
    }
    update_track_list();
    status_label_->setText(QString("Added %1 to %2.")
        .arg(QString::fromStdString(media->display_name),
             QString::fromStdString(track->name)));
}


void MainWindow::save_project(bool save_as) {
    QString target_path = current_project_path_;
    if (target_path.isEmpty() || save_as) {
        target_path = QFileDialog::getSaveFileName(
            this, "Save VeCut project", target_path.isEmpty()
                ? QString("Untitled.vecut") : target_path,
            "VeCut project (*.vecut)");
        if (target_path.isEmpty()) return;
    }

    QJsonObject document;
    document.insert("format", "VeCutProject");
    document.insert("version", 1);
    document.insert("name", QString::fromStdString(project_.name()));

    QJsonArray media_items;
    for (const auto& asset : project_.media()) {
        QJsonObject item;
        item.insert("id", QString::fromStdString(asset.id));
        item.insert("uri", QString::fromStdString(asset.uri));
        item.insert("displayName", QString::fromStdString(asset.display_name));
        item.insert("kind", static_cast<int>(asset.kind));
        item.insert("durationMs", static_cast<double>(asset.duration_ms));
        item.insert("fileSizeBytes", static_cast<double>(asset.file_size_bytes));
        item.insert("width", asset.width);
        item.insert("height", asset.height);
        media_items.append(item);
    }
    document.insert("media", media_items);

    QJsonArray track_items;
    for (const auto& track : project_.tracks()) {
        QJsonObject item;
        item.insert("id", QString::fromStdString(track.id));
        item.insert("name", QString::fromStdString(track.name));
        item.insert("kind", static_cast<int>(track.kind));
        item.insert("muted", track.muted);
        item.insert("visible", track.visible);

        QJsonArray clips;
        for (const auto& clip : track.clips) {
            QJsonObject clip_item;
            clip_item.insert("id", QString::fromStdString(clip.id));
            clip_item.insert("mediaId", QString::fromStdString(clip.media_id));
            clip_item.insert("sourceInMs", static_cast<double>(clip.source_in_ms));
            clip_item.insert("sourceOutMs", static_cast<double>(clip.source_out_ms));
            clip_item.insert("timelineStartMs", static_cast<double>(clip.timeline_start_ms));
            clip_item.insert("transitionKind",
                             static_cast<int>(clip.incoming_transition.kind));
            clip_item.insert("transitionDurationMs",
                             static_cast<double>(clip.incoming_transition.duration_ms));
            clip_item.insert("animationKind", static_cast<int>(clip.animation.kind));
            clip_item.insert("animationDurationMs", static_cast<double>(clip.animation.duration_ms));
            clip_item.insert("transitionFadeInMs", static_cast<double>(clip.incoming_transition.fade_in_ms));
            clip_item.insert("transitionFadeOutMs", static_cast<double>(clip.incoming_transition.fade_out_ms));
            clip_item.insert("volume", clip.volume);
            clips.append(clip_item);
        }
        item.insert("clips", clips);
        track_items.append(item);
    }
    document.insert("tracks", track_items);

    QSaveFile file(target_path);
    if (!file.open(QIODevice::WriteOnly)) {
        status_label_->setText(QString("Could not save project: %1").arg(file.errorString()));
        return;
    }
    const QByteArray bytes = QJsonDocument(document).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        status_label_->setText(QString("Could not finish saving project: %1")
                                   .arg(file.errorString()));
        return;
    }
    // Keep the previous active path if opening/writing/committing the new file fails.
    current_project_path_ = target_path;
    setWindowTitle(QString("VeCut — %1").arg(QFileInfo(current_project_path_).fileName()));
    status_label_->setText(QString("Project saved: %1").arg(current_project_path_));
}

void MainWindow::open_project() {
    const QString path = QFileDialog::getOpenFileName(
        this, "Open VeCut project", QString(), "VeCut project (*.vecut);;JSON files (*.json)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        status_label_->setText(QString("Could not open project: %1").arg(file.errorString()));
        return;
    }
    QJsonParseError parse_error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
        status_label_->setText(QString("Invalid project file: %1").arg(parse_error.errorString()));
        return;
    }

    const QJsonObject root = document.object();
    if (root.value("format").toString() != "VeCutProject" ||
        root.value("version").toInt(-1) != 1) {
        status_label_->setText("Unsupported VeCut project format or version.");
        return;
    }

    auto read_integer = [](const QJsonObject& object, const QString& key,
                           qint64 fallback = 0) -> qint64 {
        const QJsonValue value = object.value(key);
        if (!value.isDouble()) return fallback;
        const double number = value.toDouble();
        if (!std::isfinite(number) ||
            number < static_cast<double>(std::numeric_limits<qint64>::min()) ||
            number >= std::ldexp(1.0, 63))
            return fallback;
        return static_cast<qint64>(number);
    };

    if (!root.value("media").isArray() || !root.value("tracks").isArray()) {
        status_label_->setText("Project is missing its media or track list.");
        return;
    }

    vecut::Project loaded(root.value("name").toString("Untitled Project").toStdString());
    const QJsonArray media_items = root.value("media").toArray();
    for (const QJsonValue& value : media_items) {
        if (!value.isObject()) {
            status_label_->setText("Project contains an invalid media entry.");
            return;
        }
        const QJsonObject item = value.toObject();
        const int kind = item.value("kind").toInt(-1);
        if (kind < static_cast<int>(vecut::MediaKind::Video) ||
            kind > static_cast<int>(vecut::MediaKind::Image)) {
            status_label_->setText("Project contains an unknown media type.");
            return;
        }
        vecut::MediaAsset asset;
        asset.id = item.value("id").toString().toStdString();
        asset.uri = item.value("uri").toString().toStdString();
        asset.display_name = item.value("displayName").toString().toStdString();
        asset.kind = static_cast<vecut::MediaKind>(kind);
        asset.duration_ms = read_integer(item, "durationMs", -1);
        asset.file_size_bytes = static_cast<std::uint64_t>(
            std::max<qint64>(0, read_integer(item, "fileSizeBytes")));
        const qint64 width = read_integer(item, "width", -1);
        const qint64 height = read_integer(item, "height", -1);
        if (width < 0 || height < 0 ||
            width > std::numeric_limits<int>::max() ||
            height > std::numeric_limits<int>::max()) {
            status_label_->setText("Project contains invalid media dimensions.");
            return;
        }
        asset.width = static_cast<int>(width);
        asset.height = static_cast<int>(height);
        if (!loaded.add_media(std::move(asset))) {
            status_label_->setText("Project contains invalid or duplicate media entries.");
            return;
        }
    }

    const QJsonArray track_items = root.value("tracks").toArray();
    for (const QJsonValue& value : track_items) {
        if (!value.isObject()) {
            status_label_->setText("Project contains an invalid track entry.");
            return;
        }
        const QJsonObject item = value.toObject();
        const int kind = item.value("kind").toInt(-1);
        if (kind < static_cast<int>(vecut::TrackKind::Video) ||
            kind > static_cast<int>(vecut::TrackKind::Overlay)) {
            status_label_->setText("Project contains an unknown track type.");
            return;
        }
        vecut::Track track;
        track.id = item.value("id").toString().toStdString();
        track.name = item.value("name").toString().toStdString();
        track.kind = static_cast<vecut::TrackKind>(kind);
        track.muted = item.value("muted").toBool(false);
        track.visible = item.value("visible").toBool(true);
        if (!loaded.add_track(std::move(track))) {
            status_label_->setText("Project contains duplicate or invalid tracks.");
            return;
        }
    }

    for (int track_index = 0; track_index < track_items.size(); ++track_index) {
        const QJsonObject track_item = track_items.at(track_index).toObject();
        const QString track_id = track_item.value("id").toString();
        if (!track_item.value("clips").isArray()) {
            status_label_->setText("Project contains a track without a valid clip list.");
            return;
        }
        const QJsonArray clips = track_item.value("clips").toArray();
        for (const QJsonValue& clip_value : clips) {
            if (!clip_value.isObject()) {
                status_label_->setText("Project contains an invalid clip entry.");
                return;
            }
            const QJsonObject item = clip_value.toObject();
            const int transition_kind = item.value("transitionKind").toInt(0);
            const int animation_kind = item.value("animationKind").toInt(0);
            if (transition_kind < static_cast<int>(vecut::TransitionKind::None) ||
                transition_kind > static_cast<int>(vecut::TransitionKind::SlideRightToLeft)) {
                status_label_->setText("Project contains an unknown transition type.");
                return;
            }
            if (animation_kind < static_cast<int>(vecut::AnimationKind::None) ||
                animation_kind > static_cast<int>(vecut::AnimationKind::RotateCounterClockwise)) {
                status_label_->setText("Project contains an unknown clip animation.");
                return;
            }
            vecut::Clip clip;
            clip.id = item.value("id").toString().toStdString();
            clip.media_id = item.value("mediaId").toString().toStdString();
            clip.source_in_ms = read_integer(item, "sourceInMs", -1);
            clip.source_out_ms = read_integer(item, "sourceOutMs", -1);
            clip.timeline_start_ms = read_integer(item, "timelineStartMs", -1);
            clip.incoming_transition.kind =
                static_cast<vecut::TransitionKind>(transition_kind);
            clip.incoming_transition.duration_ms =
                read_integer(item, "transitionDurationMs", -1);
            // Read legacy clip-level fade fields as a fallback for older project files.
            clip.incoming_transition.fade_in_ms = read_integer(item, "transitionFadeInMs",
                read_integer(item, "fadeInMs", 0));
            clip.incoming_transition.fade_out_ms = read_integer(item, "transitionFadeOutMs",
                read_integer(item, "fadeOutMs", 0));
            clip.animation.kind = static_cast<vecut::AnimationKind>(animation_kind);
            clip.animation.duration_ms = read_integer(item, "animationDurationMs", 0);
            clip.volume = item.value("volume").toDouble(1.0);
            if (!loaded.add_clip(track_id.toStdString(), std::move(clip))) {
                status_label_->setText("Project contains a clip with invalid media, timing or track data.");
                return;
            }
        }
    }

    std::string validation_error;
    if (!loaded.validate(&validation_error)) {
        status_label_->setText(QString("Project validation failed: %1")
                                   .arg(QString::fromStdString(validation_error)));
        return;
    }

    undo_stack_.clear();
    redo_stack_.clear();
    update_history_buttons();
    stop_timeline_audio();
    preview_player_->stop();
    preview_player_->setSource(QUrl());
    project_ = std::move(loaded);
    current_project_path_ = path;
    setWindowTitle(QString("VeCut — %1").arg(QFileInfo(path).fileName()));
    media_list_->clear();

    unsigned int max_media_id = 0;
    unsigned int max_clip_id = 0;
    for (const auto& asset : project_.media()) {
        const QString id = QString::fromStdString(asset.id);
        const QFileInfo source_info(QString::fromStdString(asset.uri));
        auto* item = new QListWidgetItem(
            media_kind_name(asset.kind) + "  ·  " +
            QString::fromStdString(asset.display_name), media_list_);
        item->setData(Qt::UserRole, id);
        if (asset.kind == vecut::MediaKind::Image) {
            const QImage thumbnail = vecut::MediaProbe::image_thumbnail(
                QString::fromStdString(asset.uri), QSize(112, 64));
            if (!thumbnail.isNull()) item->setIcon(QIcon(QPixmap::fromImage(thumbnail)));
        }
        if (source_info.exists()) probe_media(id, source_info.absoluteFilePath(), asset.kind);
        bool ok = false;
        const uint media_number = id.startsWith("media-")
            ? id.mid(6).toUInt(&ok) : 0;
        if (ok) max_media_id = std::max(max_media_id, media_number);
    }
    audio_track_count_ = 0;
    video_track_count_ = 0;
    for (const auto& track : project_.tracks()) {
        if (track.kind == vecut::TrackKind::Audio) ++audio_track_count_;
        else ++video_track_count_;
        for (const auto& clip : track.clips) {
            const QString id = QString::fromStdString(clip.id);
            bool ok = false;
            const uint clip_number = id.startsWith("clip-")
                ? id.mid(5).toUInt(&ok) : 0;
            if (ok) max_clip_id = std::max(max_clip_id, clip_number);
        }
    }
    next_media_id_ = max_media_id + 1;
    next_clip_id_ = max_clip_id + 1;
    update_track_list();
    if (media_list_->count() > 0) media_list_->setCurrentRow(0);
    status_label_->setText(QString("Opened project: %1").arg(path));
}


void MainWindow::split_selected_clip() {
    if (selected_track_id_.isEmpty() || selected_clip_id_.isEmpty()) {
        status_label_->setText("Select a timeline clip before splitting.");
        return;
    }
    const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
        [&](const vecut::Track& item) {
            return QString::fromStdString(item.id) == selected_track_id_;
        });
    if (track == project_.tracks().end()) return;
    const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
        [&](const vecut::Clip& item) {
            return QString::fromStdString(item.id) == selected_clip_id_;
        });
    if (clip == track->clips.end() || clip->source_out_ms - clip->source_in_ms < 2) {
        status_label_->setText("This clip is too short to split.");
        return;
    }

    const auto midpoint = clip->source_in_ms +
        (clip->source_out_ms - clip->source_in_ms) / 2;
    const auto playhead = preview_player_ ? preview_player_->position() : midpoint;
    const auto default_split = playhead > clip->source_in_ms &&
        playhead < clip->source_out_ms ? playhead : midpoint;
    QDialog dialog(this);
    dialog.setWindowTitle("Split clip");
    auto* layout = new QFormLayout(&dialog);
    auto* position = new QSpinBox(&dialog);
    position->setRange(static_cast<int>(std::min<std::int64_t>(
                           clip->source_in_ms + 1, std::numeric_limits<int>::max())),
                       static_cast<int>(std::min<std::int64_t>(
                           clip->source_out_ms - 1, std::numeric_limits<int>::max())));
    position->setValue(static_cast<int>(std::min<std::int64_t>(
        default_split, std::numeric_limits<int>::max())));
    position->setSuffix(" ms");
    layout->addRow("Split at source time:", position);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                         Qt::Horizontal, &dialog);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;

    push_undo_state();
    const auto new_id = project_.split_clip(selected_track_id_.toStdString(),
        selected_clip_id_.toStdString(), position->value());
    if (!new_id) {
        status_label_->setText("Could not split the selected clip at that position.");
        return;
    }
    selected_clip_id_ = QString::fromStdString(*new_id);
    timeline_widget_->selectClip(selected_clip_id_);
    timeline_widget_->refresh();
    status_label_->setText(QString("Clip split at %1 ms. New clip: %2.")
        .arg(position->value()).arg(selected_clip_id_));
}

void MainWindow::trim_selected_clip() {
    if (selected_track_id_.isEmpty() || selected_clip_id_.isEmpty()) {
        status_label_->setText("Select a timeline clip before trimming.");
        return;
    }
    const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
        [&](const vecut::Track& item) {
            return QString::fromStdString(item.id) == selected_track_id_;
        });
    if (track == project_.tracks().end()) return;
    const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
        [&](const vecut::Clip& item) {
            return QString::fromStdString(item.id) == selected_clip_id_;
        });
    if (clip == track->clips.end()) return;
    const auto media = std::find_if(project_.media().begin(), project_.media().end(),
        [&](const vecut::MediaAsset& item) { return item.id == clip->media_id; });
    if (media == project_.media().end() || media->duration_ms <= 0) {
        status_label_->setText("Media duration is not available for trimming.");
        return;
    }

    const int maximum = static_cast<int>(std::min<std::int64_t>(
        media->duration_ms, std::numeric_limits<int>::max()));
    QDialog dialog(this);
    dialog.setWindowTitle("Trim clip");
    auto* layout = new QFormLayout(&dialog);
    auto* in_point = new QSpinBox(&dialog);
    auto* out_point = new QSpinBox(&dialog);
    in_point->setRange(0, std::max(0, maximum - 1));
    out_point->setRange(1, std::max(1, maximum));
    in_point->setValue(static_cast<int>(std::min<std::int64_t>(
        clip->source_in_ms, maximum - 1)));
    out_point->setValue(static_cast<int>(std::min<std::int64_t>(
        clip->source_out_ms, maximum)));
    in_point->setSuffix(" ms");
    out_point->setSuffix(" ms");
    layout->addRow("Source IN:", in_point);
    layout->addRow("Source OUT:", out_point);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                         Qt::Horizontal, &dialog);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    if (out_point->value() <= in_point->value()) {
        status_label_->setText("Source OUT must be after source IN.");
        return;
    }
    push_undo_state();
    if (!project_.trim_clip(selected_track_id_.toStdString(),
            selected_clip_id_.toStdString(), in_point->value(), out_point->value())) {
        status_label_->setText("The trim range is invalid or exceeds the media duration.");
        return;
    }
    timeline_widget_->refresh();
    status_label_->setText(QString("Clip trimmed to %1–%2 ms.")
        .arg(in_point->value()).arg(out_point->value()));
}

void MainWindow::edit_selected_transition() {
    if (selected_track_id_.isEmpty() || selected_clip_id_.isEmpty()) {
        status_label_->setText("Select a timeline clip before editing its transition.");
        return;
    }
    const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
        [&](const vecut::Track& item) {
            return QString::fromStdString(item.id) == selected_track_id_;
        });
    if (track == project_.tracks().end()) return;
    const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
        [&](const vecut::Clip& item) {
            return QString::fromStdString(item.id) == selected_clip_id_;
        });
    if (clip == track->clips.end()) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Transitions and animations");
    auto* layout = new QFormLayout(&dialog);
    auto* kind = new QComboBox(&dialog);
    layout->addRow(new QLabel("TRANSITIONS", &dialog));
    kind->addItem("None", static_cast<int>(vecut::TransitionKind::None));
    kind->addItem("Fade", static_cast<int>(vecut::TransitionKind::Fade));
    kind->addItem("Dissolve", static_cast<int>(vecut::TransitionKind::Dissolve));
    kind->addItem("Wipe left to right", static_cast<int>(vecut::TransitionKind::WipeLeftToRight));
    kind->addItem("Wipe right to left", static_cast<int>(vecut::TransitionKind::WipeRightToLeft));
    kind->addItem("Slide left to right", static_cast<int>(vecut::TransitionKind::SlideLeftToRight));
    kind->addItem("Slide right to left", static_cast<int>(vecut::TransitionKind::SlideRightToLeft));
    const int existing_kind = static_cast<int>(clip->incoming_transition.kind);
    kind->setCurrentIndex(std::max(0, kind->findData(existing_kind)));
    auto* duration = new QSpinBox(&dialog);
    const int max_duration = static_cast<int>(std::min<std::int64_t>(
        clip->source_out_ms - clip->source_in_ms, std::numeric_limits<int>::max()));
    duration->setRange(0, std::max(0, max_duration));
    duration->setValue(static_cast<int>(std::min<std::int64_t>(
        clip->incoming_transition.duration_ms, max_duration)));
    duration->setSuffix(" ms");
    layout->addRow("Incoming transition:", kind);
    layout->addRow("Transition duration:", duration);

    auto* fade_in = new QSpinBox(&dialog);
    auto* fade_out = new QSpinBox(&dialog);
    fade_in->setRange(0, std::max(0, max_duration));
    fade_out->setRange(0, std::max(0, max_duration));
    fade_in->setValue(static_cast<int>(std::min<std::int64_t>(clip->incoming_transition.fade_in_ms, max_duration)));
    fade_out->setValue(static_cast<int>(std::min<std::int64_t>(clip->incoming_transition.fade_out_ms, max_duration)));
    fade_in->setSuffix(" ms");
    fade_out->setSuffix(" ms");
    layout->addRow("Transition fade in:", fade_in);
    layout->addRow("Transition fade out:", fade_out);
    layout->addRow(new QLabel("ANIMATIONS", &dialog));

    auto* animation = new QComboBox(&dialog);
    animation->addItem("None", static_cast<int>(vecut::AnimationKind::None));
    animation->addItem("Zoom in", static_cast<int>(vecut::AnimationKind::ZoomIn));
    animation->addItem("Zoom out", static_cast<int>(vecut::AnimationKind::ZoomOut));
    animation->addItem("Pan left", static_cast<int>(vecut::AnimationKind::PanLeft));
    animation->addItem("Pan right", static_cast<int>(vecut::AnimationKind::PanRight));
    animation->addItem("Pan up", static_cast<int>(vecut::AnimationKind::PanUp));
    animation->addItem("Pan down", static_cast<int>(vecut::AnimationKind::PanDown));
    animation->addItem("Rotate clockwise", static_cast<int>(vecut::AnimationKind::RotateClockwise));
    animation->addItem("Rotate counter-clockwise", static_cast<int>(vecut::AnimationKind::RotateCounterClockwise));
    animation->setCurrentIndex(std::max(0, animation->findData(
        static_cast<int>(clip->animation.kind))));
    auto* animation_duration = new QSpinBox(&dialog);
    animation_duration->setRange(0, std::max(0, max_duration));
    animation_duration->setValue(static_cast<int>(std::min<std::int64_t>(
        clip->animation.duration_ms, max_duration)));
    animation_duration->setSuffix(" ms");

    const bool visual_track = track->kind != vecut::TrackKind::Audio;
    kind->setEnabled(visual_track);
    duration->setEnabled(visual_track);
    fade_in->setEnabled(visual_track);
    fade_out->setEnabled(visual_track);
    animation->setEnabled(visual_track);
    animation_duration->setEnabled(visual_track);
    layout->addRow("Clip animation (main / overlay):", animation);
    layout->addRow("Animation duration:", animation_duration);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                         Qt::Horizontal, &dialog);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;

    push_undo_state();
    vecut::Transition transition;
    transition.kind = static_cast<vecut::TransitionKind>(kind->currentData().toInt());
    transition.duration_ms = transition.kind == vecut::TransitionKind::None
        ? 0 : duration->value();
    transition.fade_in_ms = visual_track ? fade_in->value() : 0;
    transition.fade_out_ms = visual_track ? fade_out->value() : 0;
    vecut::ClipAnimation clip_animation;
    clip_animation.kind = static_cast<vecut::AnimationKind>(animation->currentData().toInt());
    clip_animation.duration_ms = clip_animation.kind == vecut::AnimationKind::None
        ? 0 : animation_duration->value();
    if (!project_.set_transition(selected_track_id_.toStdString(),
            selected_clip_id_.toStdString(), transition) ||
        (visual_track && !project_.set_clip_animation(selected_track_id_.toStdString(),
            selected_clip_id_.toStdString(), clip_animation))) {
        status_label_->setText("Could not apply those transition or animation settings.");
        return;
    }
    timeline_widget_->refresh();
    render_preview_frame();
    status_label_->setText("Transition fades and clip animation settings saved in the project.");
}

void MainWindow::toggle_timeline_audio() {
    if (timeline_audio_playing_) {
        stop_timeline_audio();
        return;
    }

    stop_timeline_audio();
    std::size_t channel_count = 0;
    for (const auto& track : project_.tracks()) {
        if (track.muted) continue;
        for (const auto& clip : track.clips) {
            const auto media = std::find_if(project_.media().begin(), project_.media().end(),
                [&](const vecut::MediaAsset& asset) { return asset.id == clip.media_id; });
            if (media == project_.media().end() || media->kind == vecut::MediaKind::Image)
                continue;

            AudioMixChannel channel;
            channel.player = new QMediaPlayer(this);
            channel.output = new QAudioOutput(this);
            channel.player->setAudioOutput(channel.output);
            channel.timeline_start_ms = clip.timeline_start_ms;
            channel.timeline_end_ms = clip.timeline_start_ms +
                (clip.source_out_ms - clip.source_in_ms);
            channel.source_in_ms = clip.source_in_ms;
            channel.output->setVolume(std::clamp(clip.volume, 0.0, 1.0));
            QMediaPlayer* player = channel.player;
            connect(player, &QMediaPlayer::mediaStatusChanged, this,
                [this, player](QMediaPlayer::MediaStatus status) {
                    const auto it = std::find_if(audio_mix_channels_.begin(),
                        audio_mix_channels_.end(), [player](const AudioMixChannel& item) {
                            return item.player == player;
                        });
                    if (it == audio_mix_channels_.end()) return;
                    if (status == QMediaPlayer::LoadedMedia ||
                        status == QMediaPlayer::BufferedMedia) {
                        it->ready = true;
                    } else if (status == QMediaPlayer::InvalidMedia) {
                        it->failed = true;
                    }
                });
            connect(player, &QMediaPlayer::errorOccurred, this,
                [this, player](QMediaPlayer::Error, const QString&) {
                    const auto it = std::find_if(audio_mix_channels_.begin(),
                        audio_mix_channels_.end(), [player](const AudioMixChannel& item) {
                            return item.player == player;
                        });
                    if (it != audio_mix_channels_.end()) it->failed = true;
                });
            audio_mix_channels_.push_back(channel);
            player->setSource(QUrl::fromLocalFile(QString::fromStdString(media->uri)));
            ++channel_count;
        }
    }

    if (channel_count == 0) {
        status_label_->setText("Add audio or video clips to an unmuted track first.");
        return;
    }

    timeline_audio_playing_ = true;
    audio_mix_clock_.start();
    audio_mix_timer_->start();
    timeline_audio_button_->setText("Stop timeline audio");
    timeline_audio_button_->setIcon(vecut_icon(VeCutIcon::Pause));
    status_label_->setText(QString("Playing timeline audio · %1 clip(s) mixed")
                               .arg(channel_count));
    update_timeline_audio();
}

void MainWindow::stop_timeline_audio() {
    if (audio_mix_timer_) audio_mix_timer_->stop();
    for (auto& channel : audio_mix_channels_) {
        if (channel.player) {
            channel.player->stop();
            delete channel.player;
        }
        if (channel.output) delete channel.output;
    }
    audio_mix_channels_.clear();
    timeline_audio_playing_ = false;
    if (timeline_audio_button_) {
        timeline_audio_button_->setText("Play timeline audio");
        timeline_audio_button_->setIcon(vecut_icon(VeCutIcon::Play));
    }
}

void MainWindow::update_timeline_audio() {
    if (!timeline_audio_playing_) return;
    const std::int64_t now_ms = audio_mix_clock_.elapsed();
    bool any_pending_or_playing = false;

    for (auto& channel : audio_mix_channels_) {
        if (channel.failed) continue;
        if (now_ms >= channel.timeline_end_ms) {
            if (channel.started && channel.player) channel.player->stop();
            continue;
        }
        any_pending_or_playing = true;
        if (!channel.started && channel.ready && now_ms >= channel.timeline_start_ms) {
            // If media loading completed after its timeline start, seek past the
            // elapsed portion instead of starting the clip late from its IN point.
            const auto lateness_ms = std::max<std::int64_t>(
                0, now_ms - channel.timeline_start_ms);
            channel.player->setPosition(channel.source_in_ms + lateness_ms);
            channel.started = true;
            channel.player->play();
        }
    }

    if (!any_pending_or_playing) {
        stop_timeline_audio();
        status_label_->setText("Timeline audio playback finished.");
    }
}

void MainWindow::edit_selected_volume() {
    if (selected_track_id_.isEmpty() || selected_clip_id_.isEmpty()) {
        status_label_->setText("Select a timeline clip before changing its volume.");
        return;
    }

    const auto track = std::find_if(project_.tracks().begin(), project_.tracks().end(),
        [&](const vecut::Track& item) {
            return QString::fromStdString(item.id) == selected_track_id_;
        });
    if (track == project_.tracks().end()) return;
    const auto clip = std::find_if(track->clips.begin(), track->clips.end(),
        [&](const vecut::Clip& item) {
            return QString::fromStdString(item.id) == selected_clip_id_;
        });
    if (clip == track->clips.end()) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Clip volume");
    auto* layout = new QFormLayout(&dialog);
    auto* volume = new QDoubleSpinBox(&dialog);
    volume->setRange(0.0, 4.0);
    volume->setDecimals(2);
    volume->setSingleStep(0.05);
    volume->setSuffix("×");
    volume->setValue(clip->volume);
    layout->addRow("Volume gain", volume);
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;

    push_undo_state();
    if (!project_.set_clip_volume(selected_track_id_.toStdString(),
                                  selected_clip_id_.toStdString(), volume->value())) {
        status_label_->setText("Could not update clip volume.");
        return;
    }
    status_label_->setText(QString("Clip volume set to %1×.")
                               .arg(volume->value(), 0, 'f', 2));
}

void MainWindow::remove_selected_clip() {
    if (selected_track_id_.isEmpty() || selected_clip_id_.isEmpty()) {
        status_label_->setText("Select a timeline clip before removing it.");
        return;
    }
    push_undo_state();
    if (!project_.remove_clip(selected_track_id_.toStdString(),
                              selected_clip_id_.toStdString())) {
        status_label_->setText("Could not remove the selected clip.");
        return;
    }
    selected_track_id_.clear();
    selected_clip_id_.clear();
    timeline_widget_->selectClip({});
    timeline_widget_->refresh();
    status_label_->setText("Clip removed from the timeline. Source media remains in the media bin.");
}

void MainWindow::update_track_list() {
    if (timeline_widget_) timeline_widget_->refresh();
}

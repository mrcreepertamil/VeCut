#include "main_window.h"

#include <QFileDialog>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QMouseEvent>
#include <QScrollArea>
#include <QAbstractItemView>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QImageReader>
#include <QMediaPlayer>
#include <QPixmap>
#include <QIcon>
#include <QUrl>
#include <QPushButton>
#include <QResizeEvent>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <QStringList>
#include <utility>
#include <functional>


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

    std::function<void(const QString&, const QString&, std::int64_t)> on_media_dropped;
    std::function<void(const QString&, const QString&, std::int64_t,
                       std::int64_t, std::int64_t)> on_clip_edited;

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
        painter.fillRect(rect(), QColor("#101219"));

        painter.fillRect(QRect(0, 0, width(), kRulerHeight), QColor("#1c202a"));
        painter.setPen(QColor("#8992a4"));
        painter.setFont(QFont("Sans Serif", 9, QFont::DemiBold));
        painter.drawText(QRect(10, 0, kTrackLabelWidth - 18, kRulerHeight),
                         Qt::AlignVCenter, "TRACK");
        const int timeline_width = width() - kTrackLabelWidth;
        const int tick_step = static_cast<int>(5 * kPixelsPerSecond);
        for (int x = kTrackLabelWidth, seconds = 0; x < width();
             x += tick_step, seconds += 5) {
            painter.setPen(QColor("#394150"));
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
                             row % 2 == 0 ? QColor("#171b24") : QColor("#141821"));
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
            painter.setPen(QColor("#303746"));
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
                    ? QColor("#315b83")
                    : (media->kind == vecut::MediaKind::Image
                        ? QColor("#76549a") : QColor("#28734f"));
                painter.setPen(selected_clip_id_ == QString::fromStdString(clip.id)
                    ? QPen(QColor("#ffffff"), 2) : QPen(fill.lighter(135), 1));
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
            update();
            return;
        }
        const auto& track = project_->tracks()[static_cast<std::size_t>(row)];
        const auto* hit = clip_at(track, point.x(), point.y());
        if (!hit) {
            selected_clip_id_.clear();
            drag_mode_ = DragMode::None;
            update();
            return;
        }
        selected_clip_id_ = QString::fromStdString(hit->id);
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
            const auto max_delta = original_out_ms_ - original_in_ms_ - 100;
            const auto applied = std::clamp(delta, min_delta, max_delta);
            start = original_start_ms_ + applied;
            source_in = original_in_ms_ + applied;
        } else if (drag_mode_ == DragMode::TrimRight) {
            const auto media = media_for_clip(drag_clip_id_);
            const auto maximum = media ? media->duration_ms : original_out_ms_;
            source_out = std::clamp(original_out_ms_ + delta,
                                    original_in_ms_ + 100, maximum);
        }
        on_clip_edited(drag_track_id_, drag_clip_id_, start, source_in, source_out);
        refresh();
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            drag_mode_ = DragMode::None;
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

    auto* import_button = new QPushButton("Import media", root);
    import_button->setObjectName("primaryButton");
    import_button->setMinimumHeight(42);
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
    preview_label_ = new QLabel("Your preview will appear here\n\nImport media to begin", preview_panel);
    preview_label_->setAlignment(Qt::AlignCenter);
    preview_label_->setMinimumSize(0, 0);
    preview_label_->setWordWrap(true);
    preview_label_->setStyleSheet("background:#090a0e;border:1px solid #303440;border-radius:12px;"
                                  "color:#a4a8b5;font-size:15px;");
    preview_layout->addWidget(preview_label_, 1);
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
    add_video_button_ = new QPushButton("+ Video track", timeline_panel_);
    add_audio_button_ = new QPushButton("+ Audio track", timeline_panel_);
    add_video_button_->setMinimumHeight(40);
    add_audio_button_->setMinimumHeight(40);
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

    connect(import_button, &QPushButton::clicked, this, [this] { import_media(); });
    connect(media_list_, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem*, QListWidgetItem*) { update_media_details(); });
    connect(add_video_button_, &QPushButton::clicked, this,
            [this] { add_track(vecut::TrackKind::Video); });
    connect(add_audio_button_, &QPushButton::clicked, this,
            [this] { add_track(vecut::TrackKind::Audio); });

    setStyleSheet(
        "QMainWindow,#root{background:#0f1015;}"
        "QFrame#panel{background:#191b23;border:1px solid #303440;border-radius:12px;}"
        "QListWidget{background:#11131a;border:1px solid #303440;border-radius:8px;"
        "color:#e4e7ef;padding:6px;}"
        "QListWidget::item{padding:8px;border-radius:5px;}"
        "QListWidget::item:selected{background:#263b31;color:#ffffff;}"
        "QPushButton{background:#292d38;color:#e9ebf2;border:1px solid #3a4050;"
        "border-radius:7px;padding:8px 10px;font-weight:600;}"
        "QPushButton:hover{background:#343a48;}"
        "QPushButton#primaryButton{background:#16834a;border-color:#16834a;color:white;}"
        "QPushButton#primaryButton:hover{background:#1b9958;}"
    );
    setCentralWidget(root);
    apply_responsive_layout();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    apply_responsive_layout();
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
        add_video_button_->setText("+ Video");
        add_audio_button_->setText("+ Audio");
        status_label_->setMaximumHeight(34);
    } else {
        workspace_->setOrientation(Qt::Horizontal);
        workspace_->setSizes({280, 820});
        timeline_panel_->setMaximumHeight(230);
        add_video_button_->setText("+ Video track");
        add_audio_button_->setText("+ Audio track");
        status_label_->setMaximumHeight(24);
    }
}

void MainWindow::import_media() {
    const QStringList paths = QFileDialog::getOpenFileNames(
        this, "Import media", QString(),
        "Media files (*.mp4 *.mov *.mkv *.webm *.avi *.mp3 *.wav *.aac *.m4a *.ogg *.flac "
        "*.png *.jpg *.jpeg *.webp *.bmp *.gif);;All files (*)");
    if (paths.isEmpty()) return;

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
        // Zero means "duration not probed yet"; never invent a source duration.
        asset.duration_ms = asset.kind == vecut::MediaKind::Image
            ? kStillImageClipDurationMs : 0;
        asset.file_size_bytes = static_cast<std::uint64_t>(std::max<qint64>(0, info.size()));

        QImageReader image_reader(path);
        if (asset.kind == vecut::MediaKind::Image) {
            const QSize original_size = image_reader.size();
            if (original_size.isValid()) {
                asset.width = original_size.width();
                asset.height = original_size.height();
            }
        }

        const QString media_id = QString::fromStdString(asset.id);
        const auto media_kind = asset.kind;
        if (!project_.add_media(std::move(asset))) continue;

        auto* item = new QListWidgetItem(
            media_kind_name(media_kind) + "  ·  " + info.fileName(), media_list_);
        item->setData(Qt::UserRole, media_id);
        if (media_kind == vecut::MediaKind::Image) {
            const QSize original_size = image_reader.size();
            if (original_size.isValid()) {
                image_reader.setScaledSize(original_size.scaled(QSize(112, 64), Qt::KeepAspectRatio));
                const QImage thumbnail = image_reader.read();
                if (!thumbnail.isNull()) item->setIcon(QIcon(QPixmap::fromImage(thumbnail)));
            }
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
    if (!project_.add_track(std::move(track))) {
        status_label_->setText("Could not add track.");
        return;
    }
    update_track_list();
    status_label_->setText(QString::fromStdString(name) + " added.");
}

void MainWindow::update_media_details() {
    const auto* item = media_list_->currentItem();
    if (!item) {
        media_details_->setText("No media selected");
        preview_label_->setText("Your preview will appear here\n\nImport media to begin");
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
        QString("%1\n%2\nDuration: %3\nSize: %4 MB\nDimensions: %5")
            .arg(QString::fromStdString(found->display_name), media_kind_name(found->kind),
                 duration, QString::number(size_mb, 'f', 2), dimensions));
    preview_label_->setText(
        QString("%1\n\nMetadata loaded\nPreview playback is not connected yet")
            .arg(QString::fromStdString(found->display_name)));
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
                if (media_list_->currentItem() == item) update_media_details();
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
    if (!project_.add_clip(track->id, std::move(clip))) {
        status_label_->setText("Could not add this media clip.");
        return;
    }
    update_track_list();
    status_label_->setText(QString("Added %1 to %2.")
        .arg(QString::fromStdString(media->display_name),
             QString::fromStdString(track->name)));
}

void MainWindow::update_track_list() {
    if (timeline_widget_) timeline_widget_->refresh();
}

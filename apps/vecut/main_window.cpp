#include "main_window.h"

#include <QFileDialog>
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
#include <QStringList>
#include <utility>

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
    media_list_ = new QListWidget(media_panel);
    media_list_->setObjectName("mediaList");
    media_list_->setMinimumWidth(0);
    media_list_->setSpacing(4);
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

    track_list_ = new QListWidget(timeline_panel_);
    track_list_->setObjectName("trackList");
    track_list_->setMinimumHeight(0);
    track_list_->setMaximumHeight(180);
    track_list_->addItem("No tracks yet — add a video or audio track");
    timeline_layout->addWidget(track_list_);
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
        asset.duration_ms = 0;
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

    const QString duration = found->duration_ms > 0
        ? QString("%1:%2").arg(found->duration_ms / 60000, 2, 10, QLatin1Char('0'))
                            .arg((found->duration_ms / 1000) % 60, 2, 10, QLatin1Char('0'))
        : (found->kind == vecut::MediaKind::Image ? QString("Still image") : QString("Probing…"));
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
            project_.update_media_info(found->id, 0, found->width, found->height);
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

void MainWindow::update_track_list() {
    track_list_->clear();
    for (const auto& track : project_.tracks()) {
        const QString kind = track.kind == vecut::TrackKind::Audio ? "AUDIO" : "VIDEO";
        auto* item = new QListWidgetItem(
            QString("[%1]  %2     ·     %3 clip(s)")
                .arg(kind, QString::fromStdString(track.name))
                .arg(track.clips.size()), track_list_);
        item->setData(Qt::UserRole, QString::fromStdString(track.id));
    }
    if (project_.tracks().empty()) {
        track_list_->addItem("No tracks yet — add a video or audio track");
    }
}

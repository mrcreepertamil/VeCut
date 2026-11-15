#include "main_window.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <QStringList>
#include <utility>

namespace {
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
    setMinimumSize(980, 640);

    auto* root = new QWidget(this);
    root->setObjectName("root");
    auto* outer = new QVBoxLayout(root);
    outer->setContentsMargins(20, 16, 20, 16);
    outer->setSpacing(14);

    auto* toolbar = new QHBoxLayout();
    auto* brand = new QLabel("VeCut  <span style='color:#22c55e'>STUDIO</span>", root);
    brand->setTextFormat(Qt::RichText);
    brand->setStyleSheet("font-size:23px;font-weight:800;color:#f4f5fa;");
    toolbar->addWidget(brand);
    toolbar->addStretch();

    auto* import_button = new QPushButton("Import media", root);
    import_button->setObjectName("primaryButton");
    toolbar->addWidget(import_button);
    outer->addLayout(toolbar);

    auto* workspace = new QSplitter(Qt::Horizontal, root);
    workspace->setChildrenCollapsible(false);

    auto* media_panel = make_panel(workspace);
    auto* media_layout = new QVBoxLayout(media_panel);
    media_layout->setContentsMargins(14, 14, 14, 14);
    media_layout->addWidget(make_section_title("MEDIA BIN", media_panel));
    media_list_ = new QListWidget(media_panel);
    media_list_->setObjectName("mediaList");
    media_list_->setMinimumWidth(210);
    media_list_->setSpacing(4);
    media_layout->addWidget(media_list_, 1);
    media_details_ = new QLabel("No media selected", media_panel);
    media_details_->setWordWrap(true);
    media_details_->setStyleSheet("color:#a4a8b5;font-size:12px;");
    media_layout->addWidget(media_details_);
    workspace->addWidget(media_panel);

    auto* preview_panel = make_panel(workspace);
    auto* preview_layout = new QVBoxLayout(preview_panel);
    preview_layout->setContentsMargins(14, 14, 14, 14);
    preview_layout->addWidget(make_section_title("PREVIEW", preview_panel));
    preview_label_ = new QLabel("Your preview will appear here\n\nImport media to begin", preview_panel);
    preview_label_->setAlignment(Qt::AlignCenter);
    preview_label_->setMinimumSize(420, 320);
    preview_label_->setStyleSheet("background:#090a0e;border:1px solid #303440;border-radius:12px;"
                                  "color:#a4a8b5;font-size:15px;");
    preview_layout->addWidget(preview_label_, 1);
    workspace->addWidget(preview_panel);
    workspace->setStretchFactor(0, 1);
    workspace->setStretchFactor(1, 3);
    outer->addWidget(workspace, 1);

    auto* timeline_panel = make_panel(root);
    auto* timeline_layout = new QVBoxLayout(timeline_panel);
    timeline_layout->setContentsMargins(14, 12, 14, 12);
    auto* timeline_toolbar = new QHBoxLayout();
    timeline_toolbar->addWidget(make_section_title("TIMELINE", timeline_panel));
    timeline_toolbar->addStretch();
    auto* add_video = new QPushButton("+ Video track", timeline_panel);
    auto* add_audio = new QPushButton("+ Audio track", timeline_panel);
    timeline_toolbar->addWidget(add_video);
    timeline_toolbar->addWidget(add_audio);
    timeline_layout->addLayout(timeline_toolbar);

    track_list_ = new QListWidget(timeline_panel);
    track_list_->setObjectName("trackList");
    track_list_->setMinimumHeight(120);
    track_list_->setMaximumHeight(180);
    track_list_->addItem("No tracks yet — add a video or audio track");
    timeline_layout->addWidget(track_list_);
    outer->addWidget(timeline_panel);

    status_label_ = new QLabel("C++17 project engine ready · Media duration probing and playback are not connected yet",
                               root);
    status_label_->setStyleSheet("color:#9aa1af;font-size:11px;");
    outer->addWidget(status_label_);

    connect(import_button, &QPushButton::clicked, this, [this] { import_media(); });
    connect(media_list_, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem*, QListWidgetItem*) { update_media_details(); });
    connect(add_video, &QPushButton::clicked, this,
            [this] { add_track(vecut::TrackKind::Video); });
    connect(add_audio, &QPushButton::clicked, this,
            [this] { add_track(vecut::TrackKind::Audio); });

    setStyleSheet(
        "QMainWindow,#root{background:#0f1015;}"
        "QFrame#panel{background:#191b23;border:1px solid #303440;border-radius:12px;}"
        "QListWidget{background:#11131a;border:1px solid #303440;border-radius:8px;"
        "color:#e4e7ef;padding:6px;}"
        "QListWidget::item{padding:8px;border-radius:5px;}"
        "QListWidget::item:selected{background:#263b31;color:#ffffff;}"
        "QPushButton{background:#292d38;color:#e9ebf2;border:1px solid #3a4050;"
        "border-radius:7px;padding:8px 12px;font-weight:600;}"
        "QPushButton:hover{background:#343a48;}"
        "QPushButton#primaryButton{background:#16834a;border-color:#16834a;color:white;}"
        "QPushButton#primaryButton:hover{background:#1b9958;}"
    );
    setCentralWidget(root);
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
        vecut::MediaAsset asset;
        asset.id = "media-" + std::to_string(next_media_id_++);
        asset.uri = info.absoluteFilePath().toStdString();
        asset.display_name = info.fileName().toStdString();
        asset.kind = media_kind_for_file(info);
        // Zero means "duration not probed yet"; never invent a source duration.
        asset.duration_ms = 0;
        if (!project_.add_media(std::move(asset))) continue;

        const auto& added = project_.media().back();
        auto* item = new QListWidgetItem(
            media_kind_name(added.kind) + "  ·  " + QString::fromStdString(added.display_name),
            media_list_);
        item->setData(Qt::UserRole, QString::fromStdString(added.id));
        ++imported;
    }

    if (imported > 0) {
        status_label_->setText(QString("Imported %1 file(s). Select an item to inspect it.")
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

    media_details_->setText(
        QString("%1\n%2\nDuration: not probed")
            .arg(QString::fromStdString(found->display_name), media_kind_name(found->kind)));
    preview_label_->setText(
        QString("%1\n\nMedia imported successfully\nPreview decoding is the next stage")
            .arg(QString::fromStdString(found->display_name)));
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

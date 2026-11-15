#pragma once

#include "vecut/project.h"

#include <QMainWindow>

class QLabel;
class QListWidget;
class QResizeEvent;
class QSplitter;
class QFrame;
class QPushButton;
class QMediaPlayer;
class QString;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void apply_responsive_layout();
    void import_media();
    void add_track(vecut::TrackKind kind);
    void update_media_details();
    void probe_media(const QString& media_id, const QString& path, vecut::MediaKind kind);
    void refresh_media_item(const QString& media_id);
    void update_track_list();

    vecut::Project project_;
    QSplitter* workspace_ = nullptr;
    QFrame* timeline_panel_ = nullptr;
    QListWidget* media_list_ = nullptr;
    QListWidget* track_list_ = nullptr;
    QLabel* preview_label_ = nullptr;
    QLabel* media_details_ = nullptr;
    QLabel* status_label_ = nullptr;
    QPushButton* add_video_button_ = nullptr;
    QPushButton* add_audio_button_ = nullptr;
    bool portrait_layout_ = false;
    bool responsive_layout_initialized_ = false;
    unsigned int next_media_id_ = 1;
    std::vector<QMediaPlayer*> active_probes_;
    unsigned int video_track_count_ = 0;
    unsigned int audio_track_count_ = 0;
};

#pragma once

#include "vecut/project.h"

#include <QString>

#include <QMainWindow>

#include <cstdint>
#include <vector>

class QLabel;
class QListWidget;
class QResizeEvent;
class QSplitter;
class QFrame;
class QPushButton;
class QMediaPlayer;
class QAudioOutput;
class QVideoWidget;
class QStackedWidget;
class QSlider;
class TimelineWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void apply_responsive_layout();
    void import_media();
    void save_project(bool save_as = false);
    void open_project();
    void add_track(vecut::TrackKind kind);
    void update_media_details(bool reload_preview = true);
    void load_preview_media(const QString& media_id);
    void probe_media(const QString& media_id, const QString& path, vecut::MediaKind kind);
    void refresh_media_item(const QString& media_id);
    void update_track_list();
    void split_selected_clip();
    void trim_selected_clip();
    void edit_selected_transition();
    void add_media_to_track(const QString& media_id, const QString& track_id,
                            std::int64_t timeline_start_ms);

    vecut::Project project_;
    QSplitter* workspace_ = nullptr;
    QFrame* timeline_panel_ = nullptr;
    QListWidget* media_list_ = nullptr;
    TimelineWidget* timeline_widget_ = nullptr;
    QLabel* preview_label_ = nullptr;
    QVideoWidget* video_preview_ = nullptr;
    QStackedWidget* preview_stack_ = nullptr;
    QSlider* preview_seek_ = nullptr;
    QPushButton* preview_play_button_ = nullptr;
    QMediaPlayer* preview_player_ = nullptr;
    QAudioOutput* preview_audio_ = nullptr;
    QLabel* media_details_ = nullptr;
    QLabel* status_label_ = nullptr;
    QPushButton* add_video_button_ = nullptr;
    QPushButton* add_audio_button_ = nullptr;
    QPushButton* split_button_ = nullptr;
    QPushButton* trim_button_ = nullptr;
    QPushButton* transition_button_ = nullptr;
    QString selected_track_id_;
    QString selected_clip_id_;
    bool portrait_layout_ = false;
    bool responsive_layout_initialized_ = false;
    QString current_project_path_;
    unsigned int next_media_id_ = 1;
    unsigned int next_clip_id_ = 1;
    std::vector<QMediaPlayer*> active_probes_;
    unsigned int video_track_count_ = 0;
    unsigned int audio_track_count_ = 0;
};

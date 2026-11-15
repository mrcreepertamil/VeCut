#pragma once

#include "vecut/project.h"

#include <QMainWindow>

class QLabel;
class QListWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void import_media();
    void add_track(vecut::TrackKind kind);
    void update_media_details();
    void update_track_list();

    vecut::Project project_;
    QListWidget* media_list_ = nullptr;
    QListWidget* track_list_ = nullptr;
    QLabel* preview_label_ = nullptr;
    QLabel* media_details_ = nullptr;
    QLabel* status_label_ = nullptr;
    unsigned int next_media_id_ = 1;
    unsigned int video_track_count_ = 0;
    unsigned int audio_track_count_ = 0;
};

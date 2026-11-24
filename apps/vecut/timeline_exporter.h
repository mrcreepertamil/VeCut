#pragma once

#include "vecut/project.h"

#include <QString>

class TimelineExporter final {
public:
    static bool render(const vecut::Project& project, const QString& ffmpeg_path,
                       const QString& output_path, QString* error = nullptr);
};

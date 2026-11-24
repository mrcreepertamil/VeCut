#include "timeline_exporter.h"

#include <QFileInfo>
#include <QProcess>

#include <algorithm>

namespace {
QString seconds(std::int64_t ms) {
    return QString::number(static_cast<double>(ms) / 1000.0, 'f', 3);
}
}

bool TimelineExporter::render(const vecut::Project& project, const QString& ffmpeg_path,
                              const QString& output_path, QString* error) {
    auto fail = [&](const QString& message) {
        if (error) *error = message;
        return false;
    };
    QString validation_error;
    if (!project.validate(&validation_error))
        return fail(QString("Project validation failed: %1").arg(validation_error));
    const auto total_ms = project.duration_ms();
    if (total_ms <= 0) return fail("The timeline is empty.");
    if (ffmpeg_path.isEmpty() || !QFileInfo::exists(ffmpeg_path))
        return fail("FFmpeg was not found. Install FFmpeg and add it to PATH.");
    if (output_path.isEmpty()) return fail("Choose an output MP4 file.");

    QStringList args;
    args << "-y" << "-hide_banner" << "-loglevel" << "error"
         << "-f" << "lavfi" << "-i"
         << QString("color=c=black:s=1280x720:r=30:d=%1").arg(seconds(total_ms));
    struct Entry { int input; const vecut::Clip* clip; const vecut::MediaAsset* media; };
    std::vector<Entry> visuals;
    std::vector<Entry> audios;
    int input = 1;
    for (const auto& track : project.tracks()) {
        if (track.muted) continue;
        for (const auto& clip : track.clips) {
            const auto media = std::find_if(project.media().begin(), project.media().end(),
                [&](const vecut::MediaAsset& item) { return item.id == clip.media_id; });
            if (media == project.media().end()) continue;
            const auto clip_ms = clip.source_out_ms - clip.source_in_ms;
            if (track.kind == vecut::TrackKind::Audio) {
                args << "-i" << QString::fromStdString(media->uri);
                audios.push_back({input++, &clip, &*media});
            } else if (track.visible) {
                if (media->kind == vecut::MediaKind::Image)
                    args << "-loop" << "1" << "-framerate" << "30" << "-t" << seconds(clip_ms);
                args << "-i" << QString::fromStdString(media->uri);
                visuals.push_back({input++, &clip, &*media});
            }
        }
    }

    QStringList graph;
    graph << "[0:v]format=rgba[base0]";
    QString base = "base0";
    int layer = 0;
    for (const auto& entry : visuals) {
        const auto& clip = *entry.clip;
        const auto clip_ms = clip.source_out_ms - clip.source_in_ms;
        const double start = static_cast<double>(clip.timeline_start_ms) / 1000.0;
        const double end = static_cast<double>(clip.timeline_start_ms + clip_ms) / 1000.0;
        const QString source = QString("vsrc%1").arg(layer);
        const QString next = QString("base%1").arg(layer + 1);
        QString filter = QString("[%1:v]trim=start=%2:duration=%3,setpts=PTS-STARTPTS+%4/TB,")
            .arg(entry.input).arg(seconds(clip.source_in_ms), seconds(clip_ms),
                 QString::number(start, 'f', 3));
        filter += "scale=1280:720:force_original_aspect_ratio=decrease,"
                  "pad=1280:720:(ow-iw)/2:(oh-ih)/2,setsar=1,format=rgba";
        if (clip.incoming_transition.fade_in_ms > 0)
            filter += QString(",fade=t=in:st=%1:d=%2:alpha=1")
                .arg(QString::number(start, 'f', 3), seconds(clip.incoming_transition.fade_in_ms));
        if (clip.incoming_transition.fade_out_ms > 0) {
            const double fade_start = start +
                static_cast<double>(clip_ms - clip.incoming_transition.fade_out_ms) / 1000.0;
            filter += QString(",fade=t=out:st=%1:d=%2:alpha=1")
                .arg(QString::number(fade_start, 'f', 3), seconds(clip.incoming_transition.fade_out_ms));
        }
        filter += QString("[%1]").arg(source);
        graph << filter;
        graph << QString("[%1][%2]overlay=0:0:format=auto:enable='between(t,%3,%4)'[%5]")
            .arg(base, source, QString::number(start, 'f', 3),
                 QString::number(end, 'f', 3), next);
        base = next;
        ++layer;
    }

    QStringList audio_labels;
    int audio_index = 0;
    for (const auto& entry : audios) {
        const auto& clip = *entry.clip;
        const auto clip_ms = clip.source_out_ms - clip.source_in_ms;
        const QString label = QString("asrc%1").arg(audio_index++);
        graph << QString("[%1:a]atrim=start=%2:duration=%3,asetpts=PTS-STARTPTS,")
            .arg(entry.input).arg(seconds(clip.source_in_ms), seconds(clip_ms))
            + QString("adelay=%1|%1,volume=%2[%3]")
                .arg(clip.timeline_start_ms)
                .arg(QString::number(std::clamp(clip.volume, 0.0, 4.0), 'f', 3), label);
        audio_labels << QString("[%1]").arg(label);
    }
    if (!audio_labels.isEmpty()) {
        QString labels;
        for (const auto& label : audio_labels) labels += label;
        graph << labels + QString("amix=inputs=%1:duration=longest:dropout_transition=0[aout]")
            .arg(audio_labels.size());
    }

    args << "-filter_complex" << graph.join(";")
         << "-map" << QString("[%1]").arg(base)
         << "-t" << seconds(total_ms)
         << "-c:v" << "libx264" << "-preset" << "veryfast"
         << "-pix_fmt" << "yuv420p" << "-r" << "30" << "-movflags" << "+faststart";
    if (audio_labels.isEmpty()) args << "-an";
    else args << "-map" << "[aout]" << "-c:a" << "aac" << "-b:a" << "192k";
    args << output_path;

    QProcess process;
    process.setProgram(ffmpeg_path);
    process.setArguments(args);
    process.start();
    if (!process.waitForStarted(10000))
        return fail(QString("Could not start FFmpeg: %1").arg(process.errorString()));
    if (!process.waitForFinished(300000)) {
        process.kill();
        process.waitForFinished();
        return fail("Export timed out after five minutes.");
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString details = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        return fail(details.isEmpty() ? "FFmpeg could not render this timeline." : details);
    }
    if (error) error->clear();
    return true;
}

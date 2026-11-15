#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QImage>
#include <QVector2D>

#include "preview_shader_library.h"

struct PreviewGpuEffects {
    float opacity = 1.0f;
    float zoom = 1.0f;
    float rotation_degrees = 0.0f;
    float pan_x = 0.0f;
    float pan_y = 0.0f;
    float reveal = 1.0f;
    float slide = 0.0f;
    int wipe_direction = 0; // 0 none, 1 left-to-right, 2 right-to-left
};

class GpuPreviewWidget final : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GpuPreviewWidget(QWidget* parent = nullptr);
    ~GpuPreviewWidget() override;

    void set_frame(const QImage& frame);
    void set_filter(vecut::PreviewFilterMode mode);
    void set_effects(const PreviewGpuEffects& effects);
    bool shader_ready() const { return shader_ready_; }
    QString shader_error() const { return shader_error_; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    bool create_shader_program();
    void upload_frame();

    QImage frame_;
    QOpenGLShaderProgram program_;
    GLuint texture_ = 0;
    vecut::PreviewFilterMode filter_ = vecut::PreviewFilterMode::Normal;
    PreviewGpuEffects effects_;
    bool shader_ready_ = false;
    QString shader_error_;
    int position_attribute_ = -1;
    int texcoord_attribute_ = -1;
};

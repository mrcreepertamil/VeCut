#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QImage>

#include "preview_shader_library.h"

class GpuPreviewWidget final : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GpuPreviewWidget(QWidget* parent = nullptr);
    ~GpuPreviewWidget() override;

    void set_frame(const QImage& frame);
    void set_filter(vecut::PreviewFilterMode mode);
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
    bool shader_ready_ = false;
    QString shader_error_;
    int position_attribute_ = -1;
    int texcoord_attribute_ = -1;
};

#include "gpu_preview_widget.h"

#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QPainter>

#include <array>
#include <cmath>

GpuPreviewWidget::GpuPreviewWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
    setMinimumSize(0, 0);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

GpuPreviewWidget::~GpuPreviewWidget() {
    if (context() && texture_) {
        makeCurrent();
        glDeleteTextures(1, &texture_);
        texture_ = 0;
        doneCurrent();
    }
}

void GpuPreviewWidget::set_frame(const QImage& frame) {
    frame_ = frame;
    if (context() && isValid()) {
        makeCurrent();
        upload_frame();
        doneCurrent();
    }
    update();
}

void GpuPreviewWidget::set_filter(vecut::PreviewFilterMode mode) {
    filter_ = mode;
    update();
}

void GpuPreviewWidget::set_effects(const PreviewGpuEffects& effects) {
    effects_ = effects;
    update();
}

void GpuPreviewWidget::initializeGL() {
    initializeOpenGLFunctions();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.035f, 0.04f, 0.055f, 1.0f);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    shader_ready_ = create_shader_program();
    if (shader_ready_) upload_frame();
}

void GpuPreviewWidget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
}

bool GpuPreviewWidget::create_shader_program() {
    const bool gles = context() && context()->isOpenGLES();
    if (!program_.addShaderFromSourceCode(QOpenGLShader::Vertex,
            vecut::PreviewShaderLibrary::vertex_source(gles))) {
        shader_error_ = program_.log();
        return false;
    }
    if (!program_.addShaderFromSourceCode(QOpenGLShader::Fragment,
            vecut::PreviewShaderLibrary::fragment_source(gles))) {
        shader_error_ = program_.log();
        return false;
    }
    if (!program_.link()) {
        shader_error_ = program_.log();
        return false;
    }
    position_attribute_ = program_.attributeLocation("aPosition");
    texcoord_attribute_ = program_.attributeLocation("aTexCoord");
    if (position_attribute_ < 0 || texcoord_attribute_ < 0) {
        shader_error_ = QStringLiteral("Preview shader attributes were not found.");
        return false;
    }
    return true;
}

void GpuPreviewWidget::upload_frame() {
    if (!texture_ || frame_.isNull()) return;
    const QImage rgba = frame_.convertToFormat(QImage::Format_RGBA8888);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba.width(), rgba.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba.constBits());
}

void GpuPreviewWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (frame_.isNull()) return;

    if (!shader_ready_ || !program_.isLinked()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(rect(), frame_);
        return;
    }

    static constexpr std::array<GLfloat, 8> positions = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f
    };
    static constexpr std::array<GLfloat, 8> texcoords = {
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };

    const float widget_aspect = height() > 0
        ? static_cast<float>(width()) / static_cast<float>(height()) : 1.0f;
    const float image_aspect = frame_.height() > 0
        ? static_cast<float>(frame_.width()) / static_cast<float>(frame_.height()) : 1.0f;
    QVector2D content_scale(1.0f, 1.0f);
    if (widget_aspect > image_aspect)
        content_scale.setX(image_aspect / widget_aspect);
    else
        content_scale.setY(widget_aspect / image_aspect);

    program_.bind();
    program_.enableAttributeArray(position_attribute_);
    program_.enableAttributeArray(texcoord_attribute_);
    program_.setAttributeArray(position_attribute_, GL_FLOAT, positions.data(), 2);
    program_.setAttributeArray(texcoord_attribute_, GL_FLOAT, texcoords.data(), 2);
    program_.setUniformValue("uTexture", 0);
    program_.setUniformValue("uFilter", static_cast<int>(filter_));
    program_.setUniformValue("uContentScale", content_scale);
    program_.setUniformValue("uOpacity", effects_.opacity);
    program_.setUniformValue("uZoom", effects_.zoom);
    program_.setUniformValue("uRotation", effects_.rotation_degrees * 0.01745329252f);
    program_.setUniformValue("uPan", QVector2D(effects_.pan_x, effects_.pan_y));
    program_.setUniformValue("uReveal", effects_.reveal);
    program_.setUniformValue("uSlide", effects_.slide);
    program_.setUniformValue("uWipeDirection", effects_.wipe_direction);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    program_.disableAttributeArray(position_attribute_);
    program_.disableAttributeArray(texcoord_attribute_);
    program_.release();
}

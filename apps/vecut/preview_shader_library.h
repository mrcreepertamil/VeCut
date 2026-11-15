#pragma once

#include <QString>

namespace vecut {

enum class PreviewFilterMode : int {
    Normal = 0,
    Monochrome = 1,
    Warm = 2,
    Cool = 3,
    Sepia = 4,
    Vintage = 5,
    Invert = 6,
    Contrast = 7
};

// GLSL ES 2.0 shader sources for the future QOpenGLWidget preview backend.
// The current QLabel/QImage preview does not create an OpenGL context, so
// these sources are not compiled or executed by the current preview widget.
class PreviewShaderLibrary final {
public:
    static QString vertex_source();
    static QString fragment_source();
};

} // namespace vecut

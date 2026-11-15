#pragma once

#include <QString>

namespace vecut {

enum class PreviewFilterMode : int {
    Normal = 0, Monochrome = 1, Warm = 2, Cool = 3,
    Sepia = 4, Vintage = 5, Invert = 6, Contrast = 7
};

class PreviewShaderLibrary final {
public:
    static QString vertex_source(bool gles = true);
    static QString fragment_source(bool gles = true);
};

} // namespace vecut

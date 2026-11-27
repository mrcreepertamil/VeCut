#pragma once

#include <QStringList>

#if defined(Q_OS_ANDROID)
QStringList scan_android_media_files();
#endif

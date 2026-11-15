#include "main_window.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("VeCut");
    resize(1280, 800);
    setMinimumSize(900, 600);

    auto* root = new QWidget(this);
    root->setObjectName("root");
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    auto* header = new QLabel("VeCut  <span style='color:#22c55e'>STUDIO</span>", root);
    header->setTextFormat(Qt::RichText);
    header->setStyleSheet("font-size:24px;font-weight:700;color:#f4f5fa;");
    layout->addWidget(header);

    auto* preview = new QLabel("PREVIEW\n\nImport media to begin editing", root);
    preview->setAlignment(Qt::AlignCenter);
    preview->setMinimumHeight(360);
    preview->setStyleSheet("background:#090a0e;border:1px solid #303440;border-radius:14px;"
                           "color:#a4a8b5;font-size:16px;");
    layout->addWidget(preview, 1);

    auto* timeline = new QLabel("TIMELINE     + Video Track     + Audio Track", root);
    timeline->setMinimumHeight(130);
    timeline->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    timeline->setContentsMargins(16, 12, 16, 12);
    timeline->setStyleSheet("background:#191b23;border:1px solid #303440;border-radius:12px;"
                            "color:#d9dce5;font-size:14px;");
    layout->addWidget(timeline);

    status_label_ = new QLabel("C++17 core loaded · Cross-platform UI foundation", root);
    status_label_->setStyleSheet("color:#22c55e;font-size:12px;");
    layout->addWidget(status_label_);

    setStyleSheet("QMainWindow,#root{background:#0f1015;}");
    setCentralWidget(root);
}

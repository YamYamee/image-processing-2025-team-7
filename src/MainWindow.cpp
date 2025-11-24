#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QScrollArea>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Team 7 - Smart Eye Dropper");
    resize(1000, 750);

    QWidget* central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // 1. 상단 컨트롤 패널 (테마 선택 & 업로드)
    QHBoxLayout* topLayout = new QHBoxLayout;

    QLabel* lblTheme = new QLabel("Select Theme:");
    lblTheme->setFont(QFont("Arial", 12, QFont::Bold));

    themeCombo = new QComboBox;
    themeCombo->addItems({ "Fashion", "Interior", "Design" }); // 기획서 테마 반영 [cite: 55]
    themeCombo->setStyleSheet("padding: 5px; font-size: 14px;");

    QPushButton* btnUpload = new QPushButton("Upload Image");
    btnUpload->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px; font-weight: bold;");
    connect(btnUpload, &QPushButton::clicked, this, &MainWindow::uploadImage);

    topLayout->addWidget(lblTheme);
    topLayout->addWidget(themeCombo);
    topLayout->addStretch();
    topLayout->addWidget(btnUpload);
    mainLayout->addLayout(topLayout);

    // 2. 이미지 표시 영역
    imageLabel = new QLabel("Upload an image to start analysis");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("border: 2px dashed #ccc; background-color: #f9f9f9;");
    imageLabel->setMinimumHeight(400);
    mainLayout->addWidget(imageLabel, 1);

    // 3. 결과 영역 (대표색 + 추천 팔레트)
    QHBoxLayout* resultLayout = new QHBoxLayout;

    // 3-1. 추출된 Dominant Color
    QVBoxLayout* domLayout = new QVBoxLayout;
    domLayout->addWidget(new QLabel("Dominant Color"));
    dominantColorLabel = new QLabel;
    dominantColorLabel->setFixedSize(100, 100);
    dominantColorLabel->setStyleSheet("border: 1px solid black; background-color: #eee;");
    domLayout->addWidget(dominantColorLabel);
    resultLayout->addLayout(domLayout);

    // 3-2. 추천 팔레트 영역
    QVBoxLayout* palLayout = new QVBoxLayout;
    palLayout->addWidget(new QLabel("Recommended Palette (Based on Theme)"));
    paletteContainer = new QWidget;
    paletteContainer->setLayout(new QHBoxLayout); // 가로로 배치
    palLayout->addWidget(paletteContainer);
    resultLayout->addLayout(palLayout);

    mainLayout->addLayout(resultLayout);
}

void MainWindow::uploadImage() {
    QString path = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.jpg *.png *.jpeg)");
    if (path.isEmpty()) return;

    if (processor.load(path.toStdString())) {
        // 이미지 표시
        QImage qimg = cvMatToQImage(processor.getProcessedImage());
        imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        analyze(); // 이미지 로드 즉시 분석 시작
    }
}

void MainWindow::analyze() {
    // 1. Dominant Color 추출 (K-Means) [cite: 59, 73]
    cv::Vec3b dominant = processor.extractDominantColor(5);

    // UI 표시
    QString hex = QString("#%1%2%3")
        .arg(dominant[2], 2, 16, QChar('0'))
        .arg(dominant[1], 2, 16, QChar('0'))
        .arg(dominant[0], 2, 16, QChar('0')).toUpper();
    dominantColorLabel->setStyleSheet(QString("background-color: %1; border: 1px solid #333;").arg(hex));
    dominantColorLabel->setToolTip(hex);

    // 2. 테마 기반 팔레트 추천 
    std::string selectedTheme = themeCombo->currentText().toStdString();
    ThemePalette recommended = matcher.recommendPalette(selectedTheme, dominant);

    // 3. 팔레트 시각화 [cite: 64]
    // 기존 위젯 제거
    QLayoutItem* item;
    while ((item = paletteContainer->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 새 팔레트 색상 추가
    for (const auto& color : recommended.colors) {
        QLabel* colorBox = new QLabel;
        colorBox->setFixedSize(60, 60);
        QString pHex = QString("background-color: rgb(%1, %2, %3); border-radius: 5px; border: 1px solid #ccc;")
            .arg(color[2]).arg(color[1]).arg(color[0]);
        colorBox->setStyleSheet(pHex);
        paletteContainer->layout()->addWidget(colorBox);
    }
}

QImage MainWindow::cvMatToQImage(const cv::Mat& mat) {
    if (mat.empty()) return QImage();
    return QImage((const unsigned char*)(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_BGR888).rgbSwapped();
}
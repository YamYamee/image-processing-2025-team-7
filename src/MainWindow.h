#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "ImageProcessor.h"
#include "PaletteMatcher.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void uploadImage();   // [cite: 56]
    void analyze();       // 분석 실행

private:
    void displayColor(QLabel* label, const cv::Vec3b& color, const QString& text);
    QImage cvMatToQImage(const cv::Mat& mat);

    // UI 요소
    QComboBox* themeCombo;       // 테마 선택 [cite: 54]
    QLabel* imageLabel;          // 이미지 표시
    QLabel* dominantColorLabel;  // 추출된 색 [cite: 58]
    QWidget* paletteContainer;   // 추천 팔레트 [cite: 61]

    ImageProcessor processor;
    PaletteMatcher matcher;
};
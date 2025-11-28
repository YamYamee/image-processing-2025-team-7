#define CVUI_IMPLEMENTATION
#include "cvui.h"

#include <iostream>
#include <cstdio>
#include "ImageProcessor.h"
#include "PaletteMatcher.h"

#define NOMINMAX
#include <windows.h>
#include <commdlg.h>

std::string OpenFileDialog() {
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.jpg;*.png;*.bmp;*.jpeg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn) == TRUE) return std::string(ofn.lpstrFile);
    return "";
}

std::string bgrToHex(const cv::Vec3b& color) {
    char hex[8];
    sprintf(hex, "#%02X%02X%02X", color[2], color[1], color[0]);
    return std::string(hex);
}

#define WINDOW_NAME "Team 7 - Smart Eye Dropper"
#define PANEL_WIDTH 320

int main(int argc, char** argv) {
    cv::Mat frame = cv::Mat(700, 800 + PANEL_WIDTH, CV_8UC3); // Increased height slightly
    cvui::init(WINDOW_NAME);

    ImageProcessor processor;
    PaletteMatcher matcher;

    std::string currentTheme = "Fashion";
    bool imageLoaded = false;
    int kClusters = 5;

    // [Modified] Store a list of colors
    std::vector<cv::Vec3b> extractedColors;
    ThemePalette recommended;
    cv::Mat displayImg;

    while (true) {
        frame = cv::Scalar(245, 245, 245);

        // --- LEFT PANEL ---
        cvui::rect(frame, 0, 0, PANEL_WIDTH, 700, 0x333333, 0x333333);

        int y = 25;
        cvui::text(frame, 20, y, "Smart Eye-Dropper", 0.9, 0xFFFFFF);
        y += 50;

        // 1. Theme
        cvui::text(frame, 20, y, "1. Select Theme", 0.6, 0xDDDDDD);
        y += 30;
        if (cvui::button(frame, 20, y, 280, 40, "Fashion")) currentTheme = "Fashion";
        y += 45;
        if (cvui::button(frame, 20, y, 280, 40, "Interior")) currentTheme = "Interior";
        y += 45;
        if (cvui::button(frame, 20, y, 280, 40, "Design")) currentTheme = "Design";
        y += 40;
        cvui::printf(frame, 20, y, 0.5, 0x00FF00, "Current: %s", currentTheme.c_str());
        y += 40;

        // 2. K-Means Settings
        cvui::text(frame, 20, y, "2. Settings", 0.6, 0xDDDDDD);
        y += 25;
        cvui::text(frame, 20, y, "Clusters (K):", 0.4, 0xAAAAAA);
        cvui::trackbar(frame, 120, y - 10, 160, &kClusters, 1, 10);
        y += 45;

        // 3. Upload
        cvui::text(frame, 20, y, "3. Upload Image", 0.6, 0xDDDDDD);
        y += 30;
        if (cvui::button(frame, 20, y, 280, 50, "Open Image File")) {
            std::string path = OpenFileDialog();
            if (!path.empty() && processor.load(path)) {
                imageLoaded = true;

                // [Modified] Extract list of colors
                extractedColors = processor.extractDominantColors(kClusters);

                displayImg = processor.getProcessedImage().clone();
                double scale = 800.0 / displayImg.cols;
                if (displayImg.rows * scale > 600) scale = 600.0 / displayImg.rows;
                cv::resize(displayImg, displayImg, cv::Size(), scale, scale);
            }
        }
        y += 60;

        if (imageLoaded) {
            if (cvui::button(frame, 20, y, 280, 40, "Re-analyze (K)")) {
                // [Modified] Re-extract list
                extractedColors = processor.extractDominantColors(kClusters);
            }
        }

        // --- RIGHT PANEL ---
        if (imageLoaded) {
            cvui::image(frame, PANEL_WIDTH, 0, displayImg);

            // Use the most dominant color (index 0) for recommendation
            if (!extractedColors.empty()) {
                recommended = matcher.recommendPalette(currentTheme, extractedColors);
            }

            int resultX = frame.cols - 300 - 20;
            int resultY = 20;

            // Background box for results
            cvui::rect(frame, resultX, resultY, 300, 550, 0xFFFFFF, 0xFFFFFF);

            // --- Section 1: Extracted Colors (K items) ---
            cvui::text(frame, resultX + 10, resultY + 10, "Extracted Colors (K-Means)", 0.6, 0x000000);

            int colorY = resultY + 40;

            for (size_t i = 0; i < extractedColors.size(); ++i) {

                unsigned int hexInt = (extractedColors[i][2] << 16) | (extractedColors[i][1] << 8) | extractedColors[i][0];

                cvui::rect(frame, resultX + 10, colorY, 40, 30, hexInt, hexInt);

                std::string hexStr = bgrToHex(extractedColors[i]);
                std::string label = (i == 0) ? hexStr + " (Main)" : hexStr;

                cvui::text(frame, resultX + 60, colorY + 10, label.c_str(), 0.5, 0x333333);

                colorY += 35;
                if (i >= 9) break;
            }

            // --- Section 2: Recommended Palette ---
            // Adjust Y position based on how many colors were drawn
            int palStartY = colorY + 20;
            cvui::text(frame, resultX + 10, palStartY, "Theme Recommendation", 0.6, 0x000000);
            cvui::text(frame, resultX + 10, palStartY + 25, ("Theme: " + recommended.name).c_str(), 0.5, 0x555555);

            int palY = palStartY + 50;
            for (const auto& c : recommended.colors) {
                unsigned int hexColor = (c[2] << 16) | (c[1] << 8) | c[0];

                cvui::rect(frame, resultX + 10, palY, 120, 30, hexColor, hexColor);

                std::string hexStr = bgrToHex(c);
                cvui::text(frame, resultX + 140, palY + 10, hexStr.c_str(), 0.5, 0x333333);
                palY += 40;
            }
        }
        else {
            cvui::text(frame, PANEL_WIDTH + 250, 300, "Please Upload an Image", 1.0, 0xCCCCCC);
        }

        cvui::imshow(WINDOW_NAME, frame);
        if (cv::waitKey(20) == 27) break;
    }

    return 0;
}
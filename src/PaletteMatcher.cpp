#include "PaletteMatcher.h"
#include <cmath>
#include <limits>
#include <iostream>
#include <algorithm>

PaletteMatcher::PaletteMatcher() {
    initDatabase();
}

double PaletteMatcher::calculateDistance(const cv::Vec3b& c1, const cv::Vec3b& c2) {
    // Calculate Euclidean distance in BGR space
    // sqrt((b1-b2)^2 + (g1-g2)^2 + (r1-r2)^2)
    return std::sqrt(std::pow(c1[0] - c2[0], 2) +
        std::pow(c1[1] - c2[1], 2) +
        std::pow(c1[2] - c2[2], 2));
}

ThemePalette PaletteMatcher::recommendPalette(const std::string& theme, const std::vector<cv::Vec3b>& extractedColors) {
    // Return empty if theme doesn't exist or no colors provided
    if (database.find(theme) == database.end() || extractedColors.empty()) return {};

    const auto& palettes = database[theme];
    ThemePalette bestMatch;
    double minTotalDist = std::numeric_limits<double>::max();

    // 사용할 색상 개수 (최대 2개, 추출된 색이 1개라면 1개만 사용)
    int colorsToMatch = std::min((int)extractedColors.size(), 2);

    // 모든 팔레트를 순회하며 점수 계산
    for (const auto& p : palettes) {
        double currentPaletteScore = 0.0;

        // 추출된 상위 N개(최대 2개) 색상 각각에 대해...
        for (int i = 0; i < colorsToMatch; ++i) {
            double minDistForThisColor = std::numeric_limits<double>::max();

            // 현재 팔레트의 구성 색상들 중 가장 비슷한(거리가 짧은) 색을 찾음
            // (base_color만 비교하는 것이 아니라 팔레트 전체 색상과 비교)
            for (const auto& paletteColor : p.colors) {
                double dist = calculateDistance(extractedColors[i], paletteColor);
                if (dist < minDistForThisColor) {
                    minDistForThisColor = dist;
                }
            }
            // 가장 비슷한 색상과의 거리를 점수에 누적
            currentPaletteScore += minDistForThisColor;
        }

        // 점수가 낮을수록(거리가 가까울수록) 더 좋은 매칭
        if (currentPaletteScore < minTotalDist) {
            minTotalDist = currentPaletteScore;
            bestMatch = p;
        }
    }
    return bestMatch;
}

void PaletteMatcher::initDatabase() {
    std::string filename = "palettes.json";
    cv::FileStorage fs(filename, cv::FileStorage::READ);

    if (!fs.isOpened()) {
        // If the file is not found, print an error (or fallback to hardcoded data)
        std::cerr << "Warning: palettes.json not found!" << std::endl;
        return;
    }

    std::vector<std::string> themes = { "Fashion", "Interior", "Design" };

    for (const auto& themeName : themes) {
        cv::FileNode themeNode = fs[themeName];
        if (themeNode.type() != cv::FileNode::SEQ) continue;

        std::vector<ThemePalette> paletteList;

        for (auto it = themeNode.begin(); it != themeNode.end(); ++it) {
            ThemePalette palette;
            (*it)["name"] >> palette.name;

            // Read Base Color (Convert RGB to BGR)
            std::vector<int> rgb;
            (*it)["base_color"] >> rgb;
            if (rgb.size() >= 3) {
                // Input is RGB(0,1,2) -> OpenCV uses BGR(2,1,0)
                palette.baseColor = cv::Vec3b(rgb[2], rgb[1], rgb[0]);
            }

            // Read Palette Colors (Convert RGB to BGR)
            cv::FileNode colorsNode = (*it)["colors"];
            for (auto cIt = colorsNode.begin(); cIt != colorsNode.end(); ++cIt) {
                std::vector<int> cVal;
                (*cIt) >> cVal;
                if (cVal.size() >= 3) {
                    palette.colors.push_back(cv::Vec3b(cVal[2], cVal[1], cVal[0]));
                }
            }
            paletteList.push_back(palette);
        }
        database[themeName] = paletteList;
    }
    fs.release();
}
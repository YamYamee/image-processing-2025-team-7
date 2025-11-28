#include "PaletteMatcher.h"
#include <cmath>
#include <limits>
#include <iostream>

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

ThemePalette PaletteMatcher::recommendPalette(const std::string& theme, const cv::Vec3b& extractedColor) {
    // Return empty if theme doesn't exist
    if (database.find(theme) == database.end()) return {};

    const auto& palettes = database[theme];
    ThemePalette bestMatch;
    double minDist = std::numeric_limits<double>::max();

    // Find the palette with the minimum color distance
    for (const auto& p : palettes) {
        double dist = calculateDistance(extractedColor, p.baseColor);
        if (dist < minDist) {
            minDist = dist;
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
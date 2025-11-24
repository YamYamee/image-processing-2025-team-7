#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <map>

// Structure to represent a palette associated with a theme
struct ThemePalette {
    std::string name;             // Palette Name (e.g., "Modern Chic")
    cv::Vec3b baseColor;          // Base color for matching
    std::vector<cv::Vec3b> colors;// The palette colors
};

class PaletteMatcher {
public:
    PaletteMatcher();

    // Recommends a palette based on the selected theme and extracted dominant color
    // Uses Euclidean distance for matching
    ThemePalette recommendPalette(const std::string& theme, const cv::Vec3b& extractedColor);

private:
    // Database storing palettes for each theme (Fashion, Interior, Design)
    std::map<std::string, std::vector<ThemePalette>> database;

    // Calculates Euclidean distance between two colors
    double calculateDistance(const cv::Vec3b& c1, const cv::Vec3b& c2);

    // Initializes the hardcoded palette database
    void initDatabase();
};
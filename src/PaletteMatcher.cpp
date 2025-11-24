#include "PaletteMatcher.h"
#include <cmath>
#include <limits>

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
    // 1. Fashion Theme
    std::vector<ThemePalette> fashion;
    fashion.push_back({ "Classic Navy", {120, 50, 50}, {{120,50,50}, {200,200,200}, {50,50,150}, {100,100,100}} });
    fashion.push_back({ "Spring Pastel", {180, 200, 255}, {{180,200,255}, {200,255,220}, {255,200,210}, {255,255,240}} });
    fashion.push_back({ "Earthy Tone", {50, 100, 150}, {{50,100,150}, {80,130,180}, {40,80,120}, {200,210,220}} });
    fashion.push_back({ "Monochrome", {30, 30, 30}, {{0,0,0}, {50,50,50}, {150,150,150}, {220,220,220}} });
    database["Fashion"] = fashion;

    // 2. Interior Theme
    std::vector<ThemePalette> interior;
    interior.push_back({ "Cozy Wood", {30, 70, 120}, {{30,70,120}, {220,230,240}, {50,100,160}, {100,150,200}} });
    interior.push_back({ "Modern Gray", {128, 128, 128}, {{128,128,128}, {50,50,50}, {200,200,200}, {255,255,255}} });
    interior.push_back({ "Nordic Blue", {180, 150, 100}, {{180,150,100}, {230,230,230}, {100,80,60}, {200,180,160}} });
    database["Interior"] = interior;

    // 3. Design Theme
    std::vector<ThemePalette> design;
    design.push_back({ "Professional Blue", {200, 100, 50}, {{200,100,50}, {240,240,240}, {50,50,50}, {100,50,0}} });
    design.push_back({ "Vivid Pop", {50, 50, 200}, {{50,50,200}, {50,200,200}, {200,200,50}, {200,50,200}} });
    design.push_back({ "Dark Mode", {20, 20, 20}, {{30,30,30}, {100,255,100}, {200,100,255}, {50,150,255}} });
    database["Design"] = design;
}
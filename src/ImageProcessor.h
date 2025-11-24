#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class ImageProcessor {
public:
    bool load(const std::string& path);
    cv::Mat getProcessedImage() const { return image; }

    // [Modified] Returns a list of colors instead of a single color
    std::vector<cv::Vec3b> extractDominantColors(int k);

private:
    cv::Mat image;
};
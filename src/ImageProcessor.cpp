#include "ImageProcessor.h"
#include <algorithm> // for std::sort

bool ImageProcessor::load(const std::string& path) {
    image = cv::imread(path);
    if (image.empty()) return false;

    if (image.cols > 600) {
        cv::resize(image, image, cv::Size(600, image.rows * 600 / image.cols));
    }
    return true;
}

std::vector<cv::Vec3b> ImageProcessor::extractDominantColors(int k) {
    if (image.empty()) return {};

    cv::Mat samples = image.reshape(1, image.rows * image.cols);
    samples.convertTo(samples, CV_32F);

    cv::Mat labels, centers;
    cv::kmeans(samples, k, labels,
        cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0),
        3, cv::KMEANS_PP_CENTERS, centers);

    // 1. Count pixels per cluster
    std::vector<int> counts(k, 0);
    for (int i = 0; i < labels.rows; ++i) {
        counts[labels.at<int>(i)]++;
    }

    // 2. Sort clusters by count (Descending order)
    // We store pairs of {count, index} to sort them
    std::vector<std::pair<int, int>> sortedIndices;
    for (int i = 0; i < k; ++i) {
        sortedIndices.push_back({ counts[i], i });
    }

    // Lambda function for sorting
    std::sort(sortedIndices.begin(), sortedIndices.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first > b.first; // Descending (Larger count first)
        });

    // 3. Collect colors in sorted order
    std::vector<cv::Vec3b> results;
    for (int i = 0; i < k; ++i) {
        int idx = sortedIndices[i].second;
        results.push_back(cv::Vec3b(
            (uchar)centers.at<float>(idx, 0),
            (uchar)centers.at<float>(idx, 1),
            (uchar)centers.at<float>(idx, 2)
        ));
    }

    return results;
}
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <cmath>

static std::string toHex(const cv::Vec3b &bgr)
{
    char buffer[8];

    sprintf(buffer, "#%02X%02X%02X", bgr[2], bgr[1], bgr[0]);
    return std::string(buffer);
}

static cv::Vec3b bgrToHsv(const cv::Vec3b &bgr)
{
    cv::Mat bgrMat(1,1,CV_8UC3, cv::Scalar(bgr[0], bgr[1], bgr[2]));
    cv::Mat hsvMat;
    cv::cvtColor(bgrMat, hsvMat, cv::COLOR_BGR2HSV);
    cv::Vec3b hsv = hsvMat.at<cv::Vec3b>(0,0);
    return hsv;
}

static cv::Vec3b hsvToBgr(const cv::Vec3b &hsv)
{
    cv::Mat hsvMat(1,1,CV_8UC3, cv::Scalar(hsv[0], hsv[1], hsv[2]));
    cv::Mat bgrMat;
    cv::cvtColor(hsvMat, bgrMat, cv::COLOR_HSV2BGR);
    cv::Vec3b bgr = bgrMat.at<cv::Vec3b>(0,0);
    return bgr;
}

static cv::Vec3b shiftHue(const cv::Vec3b &bgr, int dh, int dv = 0, int ds = 0)
{
    cv::Vec3b hsv = bgrToHsv(bgr);
    int h = hsv[0];
    int s = hsv[1];
    int v = hsv[2];

    h = (h + dh) % 180; if (h<0) h+=180;
    s = std::clamp(s + ds, 0, 255);
    v = std::clamp(v + dv, 0, 255);
    cv::Vec3b newhsv(h, s, v);
    return hsvToBgr(newhsv);
}

static void checkDuplication(const std::vector<cv::Vec3b> &palette, std::vector<std::string> &hexes)
{
    for (auto &c : palette) {
        std::string h = toHex(c);
        if (std::find(hexes.begin(), hexes.end(), h) == hexes.end())
            hexes.push_back(h);
    }
}

static void printResults(std::vector<std::string> hexes)
{
    std::cout << "palette=";
    for (std::size_t i = 0; i < hexes.size(); ++i) {
        std::cout << hexes[i];
        if (i+1 < hexes.size())
            std::cout << ",";
    }
    std::cout << std::endl;
}

static bool loadAndPreprocess(const std::string &path, cv::Mat &img, cv::Mat &procImg, int maxDim = 600)
{
    img = cv::imread(path, cv::IMREAD_COLOR);
    if (img.empty()) return false;
    procImg = img;
    if (std::max(img.cols, img.rows) > maxDim) {
        double scale = (double)maxDim / (double)std::max(img.cols, img.rows);
        cv::resize(img, procImg, cv::Size(), scale, scale, cv::INTER_AREA);
        std::cerr << "Downscaled image for processing: " << procImg.cols << "x" << procImg.rows << std::endl;
    }
    return true;
}

static void buildSamples(const cv::Mat &procImg, cv::Mat &samples, std::vector<cv::Point> &samplePts, std::size_t maxSamples = 20000)
{
    cv::Mat gcMask(procImg.size(), CV_8UC1, cv::Scalar(cv::GC_PR_BGD));
    cv::Mat bgModel, fgModel;
    int rectW = std::max(1, procImg.cols * 8 / 10);
    int rectH = std::max(1, procImg.rows * 8 / 10);
    int rectX = (procImg.cols - rectW) / 2;
    int rectY = (procImg.rows - rectH) / 2;
    cv::Rect rect(rectX, rectY, rectW, rectH);

    try {
        cv::grabCut(procImg, gcMask, rect, bgModel, fgModel, 3, cv::GC_INIT_WITH_RECT);
    } catch (const cv::Exception &e) {
        gcMask.release();
    }

    cv::Mat foregroundMask;
    if (!gcMask.empty())
        foregroundMask = (gcMask == cv::GC_FGD) | (gcMask == cv::GC_PR_FGD);

    std::vector<cv::Point> fgPts;
    if (!foregroundMask.empty()) {
        fgPts.reserve(foregroundMask.rows * foregroundMask.cols / 8);
        for (int y = 0; y < foregroundMask.rows; ++y) {
            const uchar* row = foregroundMask.ptr<uchar>(y);
            for (int x = 0; x < foregroundMask.cols; ++x) {
                if (row[x])
                    fgPts.emplace_back(x, y);
            }
        }
    }

    if (fgPts.size() >= 50) {
        std::vector<std::size_t> indices(fgPts.size());
        for (std::size_t i = 0; i < fgPts.size(); ++i) indices[i] = i;
        if (fgPts.size() > maxSamples) {
            std::shuffle(indices.begin(), indices.end(), std::mt19937{std::random_device{}()});
            indices.resize(maxSamples);
        }

        samples.create((int)indices.size(), 3, CV_32F);
        samplePts.reserve(indices.size());
        for (std::size_t i = 0; i < indices.size(); ++i) {
            cv::Point p = fgPts[indices[i]];
            samplePts.emplace_back(p);
            cv::Vec3b pix = procImg.at<cv::Vec3b>(p.y, p.x);
            samples.at<float>((int)i, 0) = pix[0];
            samples.at<float>((int)i, 1) = pix[1];
            samples.at<float>((int)i, 2) = pix[2];
        }
        std::cerr << "Using GrabCut foreground (" << fgPts.size() << " pixels, sampled " << samples.rows << ") for clustering" << std::endl;
    } else {
        cv::Mat reshaped = procImg.reshape(1, procImg.rows * procImg.cols);
        reshaped.convertTo(samples, CV_32F);
        samplePts.reserve(procImg.rows * procImg.cols);
        for (int i = 0; i < procImg.rows * procImg.cols; ++i) {
            int y = i / procImg.cols;
            int x = i % procImg.cols;
            samplePts.emplace_back(x, y);
        }
        if (!foregroundMask.empty())
            std::cerr << "GrabCut produced too few pixels (" << fgPts.size() << "); falling back to whole processed image." << std::endl;
        else
            std::cerr << "GrabCut not used; falling back to whole processed image." << std::endl;
    }
}

static void runKMeans(const cv::Mat &samples, int clusters_nb, int attempts, cv::Mat &labels, cv::Mat &centers)
{
    cv::kmeans(samples, clusters_nb, labels,
           cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 10, 1.0),
           attempts, cv::KMEANS_PP_CENTERS, centers);
}

static cv::Vec3b selectDominant(const cv::Mat &centers, const cv::Mat &labels, const std::vector<cv::Point> &samplePts, int clusters_nb, const cv::Mat &procImg)
{
    std::vector<int> counts(clusters_nb,0);
    for (int i = 0; i < labels.rows; ++i) counts[labels.at<int>(i,0)]++;

    std::vector<cv::Point2d> centroids(clusters_nb, cv::Point2d(0,0));
    std::vector<int> clusterCounts(clusters_nb, 0);
    for (int i = 0; i < labels.rows; ++i) {
        int lbl = labels.at<int>(i,0);
        if (lbl < 0 || lbl >= clusters_nb) continue;
        cv::Point p = samplePts.size() > (std::size_t)i ? samplePts[i] : cv::Point(0,0);
        centroids[lbl].x += p.x;
        centroids[lbl].y += p.y;
        clusterCounts[lbl]++;
    }
    for (int i = 0; i < clusters_nb; ++i) {
        if (clusterCounts[i] > 0) {
            centroids[i].x /= clusterCounts[i];
            centroids[i].y /= clusterCounts[i];
        }
    }

    cv::Point2d center((double)procImg.cols/2.0, (double)procImg.rows/2.0);
    int bestIdx = 0;
    double bestScore = 1e300;

    for (int i = 0; i < clusters_nb; ++i) {
        double dist = std::hypot(centroids[i].x - center.x, centroids[i].y - center.y);
        double countNorm = (clusterCounts[i] > 0) ? std::log((double)clusterCounts[i] + 1.0) : 0.0;
        double score = (clusterCounts[i] > 0) ? dist / (1.0 + countNorm) : 1e300;
        if (score < bestScore) {
            bestScore = score;
            bestIdx = i;
        }
    }

    cv::Vec3b c;
    c[0] = (uchar)centers.at<float>(bestIdx,0);
    c[1] = (uchar)centers.at<float>(bestIdx,1);
    c[2] = (uchar)centers.at<float>(bestIdx,2);
    return c;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: ./" << argv[0] << " <image_path> <number of clusters(default 4)>" << std::endl;
        return 1;
    }

    std::string path = argv[1];
    int clusters_nb = 4;
    if (argc >= 3)
        clusters_nb = std::stoi(argv[2]);

    cv::Mat img, procImg;
    if (!loadAndPreprocess(path, img, procImg)) {
        std::cerr << "Could not open or find the image: " << path << std::endl;
        return 2;
    }

    cv::Mat samples;
    std::vector<cv::Point> samplePts;
    buildSamples(procImg, samples, samplePts);

    cv::Mat labels, centers;
    int attempts = 3;
    runKMeans(samples, clusters_nb, attempts, labels, centers);

    cv::Vec3b dominant = selectDominant(centers, labels, samplePts, clusters_nb, procImg);
    std::cout << "dominant color = " << toHex(dominant) << "\n";

    std::vector<cv::Vec3b> palette;
    palette.push_back(dominant);
    palette.push_back(shiftHue(dominant, -15));
    palette.push_back(shiftHue(dominant, 15));
    palette.push_back(shiftHue(dominant, 90));
    palette.push_back(shiftHue(dominant, 60));
    palette.push_back(shiftHue(dominant, -60));
    palette.push_back(shiftHue(dominant, 0, 40));
    palette.push_back(shiftHue(dominant, 0, -60));

    std::vector<std::string> hexes;
    checkDuplication(palette, hexes);
    printResults(hexes);
    return 0;
}

    
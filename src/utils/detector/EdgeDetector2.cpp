/**
 * @file EdgeDetector2.cpp
 * @brief 边缘检测器实现
 */

#include "utils/detector/EdgeDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

EdgeDetector2::EdgeDetector2(QObject* parent)
    : QObject(parent)
{
}

QVector<double> EdgeDetector2::sobel(const QVector<double>& image,
                                      int width, int height) const
{
    QElapsedTimer timer;
    timer.start();

    /* Sobel核 */
    static const double Gx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    static const double Gy[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    int n = width * height;
    QVector<double> result(n, 0.0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            double sumX = 0.0, sumY = 0.0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    double pixel = image[(y + ky) * width + (x + kx)];
                    int ki = (ky + 1) * 3 + (kx + 1);
                    sumX += pixel * Gx[ki];
                    sumY += pixel * Gy[ki];
                }
            }
            result[y * width + x] = qSqrt(sumX * sumX + sumY * sumY);
        }
    }

    m_stats.totalDetections++;
    m_stats.totalPixelsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted("Sobel", n);
    return result;
}

QVector<double> EdgeDetector2::prewitt(const QVector<double>& image,
                                       int width, int height) const
{
    QElapsedTimer timer;
    timer.start();

    /* Prewitt核 */
    static const double Gx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    static const double Gy[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};

    int n = width * height;
    QVector<double> result(n, 0.0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            double sumX = 0.0, sumY = 0.0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    double pixel = image[(y + ky) * width + (x + kx)];
                    int ki = (ky + 1) * 3 + (kx + 1);
                    sumX += pixel * Gx[ki];
                    sumY += pixel * Gy[ki];
                }
            }
            result[y * width + x] = qSqrt(sumX * sumX + sumY * sumY);
        }
    }

    m_stats.totalDetections++;
    m_stats.totalPixelsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted("Prewitt", n);
    return result;
}

QVector<double> EdgeDetector2::canny(const QVector<double>& image,
                                      int width, int height,
                                      const CannyParams& params) const
{
    QElapsedTimer timer;
    timer.start();

    int n = width * height;

    /* 步骤1: 高斯模糊降噪 */
    QVector<double> blurred = gaussianBlur(image, width, height,
        params.gaussianKernelSize, params.gaussianSigma);

    /* 步骤2: 计算梯度幅值和方向(Sobel) */
    static const double Gx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    static const double Gy[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    QVector<double> magnitude(n, 0.0);
    QVector<double> direction(n, 0.0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            double sumX = 0.0, sumY = 0.0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    double pixel = blurred[(y + ky) * width + (x + kx)];
                    int ki = (ky + 1) * 3 + (kx + 1);
                    sumX += pixel * Gx[ki];
                    sumY += pixel * Gy[ki];
                }
            }
            magnitude[y * width + x] = qSqrt(sumX * sumX + sumY * sumY);
            direction[y * width + x] = qAtan2(sumY, sumX);
        }
    }

    /* 步骤3: 非极大值抑制 */
    QVector<double> suppressed = nonMaxSuppression(magnitude, direction,
                                                    width, height);

    /* 步骤4: 双阈值+滞后连接 */
    QVector<double> edges = hysteresisThreshold(suppressed, width, height,
                                                 params.lowThreshold,
                                                 params.highThreshold);

    m_stats.totalDetections++;
    m_stats.totalPixelsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted("Canny", n);
    return edges;
}

QVector<double> EdgeDetector2::gaussianBlur(const QVector<double>& image,
                                             int width, int height,
                                             int kernelSize, double sigma) const
{
    QVector<double> kernel = createGaussianKernel(kernelSize, sigma);
    int half = kernelSize / 2;
    int n = width * height;
    QVector<double> result(n, 0.0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double sum = 0.0;
            for (int ky = -half; ky <= half; ++ky) {
                for (int kx = -half; kx <= half; ++kx) {
                    int py = qBound(0, y + ky, height - 1);
                    int px = qBound(0, x + kx, width - 1);
                    int ki = (ky + half) * kernelSize + (kx + half);
                    sum += image[py * width + px] * kernel[ki];
                }
            }
            result[y * width + x] = sum;
        }
    }
    return result;
}

QVector<double> EdgeDetector2::convolve3x3(const QVector<double>& image,
                                            int width, int height,
                                            const double kernel[9]) const
{
    int n = width * height;
    QVector<double> result(n, 0.0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            double sum = 0.0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    sum += image[(y + ky) * width + (x + kx)]
                        * kernel[(ky + 1) * 3 + (kx + 1)];
                }
            }
            result[y * width + x] = sum;
        }
    }
    return result;
}

QVector<double> EdgeDetector2::createGaussianKernel(int size,
                                                     double sigma) const
{
    QVector<double> kernel(size * size);
    int half = size / 2;
    double sum = 0.0;

    for (int y = -half; y <= half; ++y) {
        for (int x = -half; x <= half; ++x) {
            double val = qExp(-(x * x + y * y) / (2.0 * sigma * sigma));
            kernel[(y + half) * size + (x + half)] = val;
            sum += val;
        }
    }

    /* 归一化 */
    for (auto& v : kernel) v /= sum;
    return kernel;
}

QVector<double> EdgeDetector2::nonMaxSuppression(
    const QVector<double>& magnitude,
    const QVector<double>& direction,
    int width, int height) const
{
    int n = width * height;
    QVector<double> suppressed(n, 0.0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            double angle = direction[y * width + x] * 180.0 / M_PI;
            if (angle < 0) angle += 180.0;

            double mag = magnitude[y * width + x];
            double q = 0.0, r = 0.0;

            /* 量化到4个方向 */
            if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180)) {
                q = magnitude[y * width + (x + 1)];
                r = magnitude[y * width + (x - 1)];
            } else if (angle >= 22.5 && angle < 67.5) {
                q = magnitude[(y - 1) * width + (x + 1)];
                r = magnitude[(y + 1) * width + (x - 1)];
            } else if (angle >= 67.5 && angle < 112.5) {
                q = magnitude[(y - 1) * width + x];
                r = magnitude[(y + 1) * width + x];
            } else {
                q = magnitude[(y - 1) * width + (x - 1)];
                r = magnitude[(y + 1) * width + (x + 1)];
            }

            suppressed[y * width + x] = (mag >= q && mag >= r) ? mag : 0.0;
        }
    }
    return suppressed;
}

QVector<double> EdgeDetector2::hysteresisThreshold(
    const QVector<double>& suppressed,
    int width, int height,
    double lowThresh, double highThresh) const
{
    int n = width * height;
    QVector<double> edges(n, 0.0);

    /* 标记强/弱边缘 */
    QVector<int> labels(n, 0); /* 0=非边缘, 1=弱, 2=强 */
    for (int i = 0; i < n; ++i) {
        if (suppressed[i] >= highThresh) labels[i] = 2;
        else if (suppressed[i] >= lowThresh) labels[i] = 1;
    }

    /* 强边缘直接输出; 弱边缘连接到强边缘则输出 */
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;
            if (labels[idx] == 2) {
                edges[idx] = 255.0;
            } else if (labels[idx] == 1) {
                /* 检查8邻域是否有强边缘 */
                bool connected = false;
                for (int dy = -1; dy <= 1 && !connected; ++dy) {
                    for (int dx = -1; dx <= 1 && !connected; ++dx) {
                        if (labels[(y + dy) * width + (x + dx)] == 2) {
                            connected = true;
                        }
                    }
                }
                if (connected) edges[idx] = 255.0;
            }
        }
    }
    return edges;
}

EdgeDetector2::Stats EdgeDetector2::stats() const
{
    return m_stats;
}

void EdgeDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file PhaseCorrelate.cpp
 * @brief 相位相关算法实现 — 图像/信号配准
 */

#include "utils/signal13/PhaseCorrelate.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PhaseCorrelate::PhaseCorrelate(QObject* parent)
    : QObject(parent)
{
}

/** @brief 补零到2的幂次 */
int PhaseCorrelate::nextPowerOfTwo(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/** @brief 1D FFT(Cooley-Tukey, 就地) @param re 实部 @param im 虚部 @param inverse 是否逆变换 */
void PhaseCorrelate::fft1D(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int n = re.size();
    if (n <= 1) return;

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len *= 2) {
        double angle = sign * 2.0 * M_PI / len;
        double wRe = std::cos(angle), wIm = std::sin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = curRe * re[i + j + len / 2] - curIm * im[i + j + len / 2];
                double tIm = curRe * im[i + j + len / 2] + curIm * re[i + j + len / 2];
                re[i + j + len / 2] = re[i + j] - tRe;
                im[i + j + len / 2] = im[i + j] - tIm;
                re[i + j] += tRe;
                im[i + j] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            re[i] /= n;
            im[i] /= n;
        }
    }
    m_stats.totalFFTsPerformed += 1;
}

/** @brief 2D FFT(行列分解) */
void PhaseCorrelate::fft2D(QVector<double>& re, QVector<double>& im,
                            int rows, int cols, bool inverse) const
{
    /* 按行变换 */
    for (int r = 0; r < rows; ++r) {
        QVector<double> rowRe(cols), rowIm(cols);
        for (int c = 0; c < cols; ++c) {
            rowRe[c] = re[r * cols + c];
            rowIm[c] = im[r * cols + c];
        }
        fft1D(rowRe, rowIm, inverse);
        for (int c = 0; c < cols; ++c) {
            re[r * cols + c] = rowRe[c];
            im[r * cols + c] = rowIm[c];
        }
    }

    /* 按列变换 */
    for (int c = 0; c < cols; ++c) {
        QVector<double> colRe(rows), colIm(rows);
        for (int r = 0; r < rows; ++r) {
            colRe[r] = re[r * cols + c];
            colIm[r] = im[r * cols + c];
        }
        fft1D(colRe, colIm, inverse);
        for (int r = 0; r < rows; ++r) {
            re[r * cols + c] = colRe[r];
            im[r * cols + c] = colIm[r];
        }
    }
}

/** @brief 1D信号相位相关 @param signal1 参考信号 @param signal2 待对齐信号 @return 位移结果 */
PhaseCorrelate::ShiftResult PhaseCorrelate::correlate1D(
    const QVector<double>& signal1, const QVector<double>& signal2)
{
    QElapsedTimer timer;
    timer.start();

    int len = nextPowerOfTwo(std::max(signal1.size(), signal2.size()));

    QVector<double> re1(len, 0.0), im1(len, 0.0);
    QVector<double> re2(len, 0.0), im2(len, 0.0);

    for (int i = 0; i < signal1.size(); ++i) re1[i] = signal1[i];
    for (int i = 0; i < signal2.size(); ++i) re2[i] = signal2[i];

    /* FFT */
    fft1D(re1, im1, false);
    fft1D(re2, im2, false);

    /* 互功率谱: R = F1 * conj(F2) / |F1 * conj(F2)| */
    QVector<double> crossRe(len), crossIm(len);
    for (int i = 0; i < len; ++i) {
        crossRe[i] = re1[i] * re2[i] + im1[i] * im2[i];
        crossIm[i] = im1[i] * re2[i] - re1[i] * im2[i];
        double mag = std::sqrt(crossRe[i] * crossRe[i] + crossIm[i] * crossIm[i]);
        if (mag > 1e-12) {
            crossRe[i] /= mag;
            crossIm[i] /= mag;
        } else {
            crossRe[i] = 0.0;
            crossIm[i] = 0.0;
        }
    }

    /* IFFT得到相位相关 */
    fft1D(crossRe, crossIm, true);

    /* 寻找峰值 */
    int peakIdx = 0;
    double peakVal = 0.0;
    for (int i = 0; i < len; ++i) {
        double v = crossRe[i] * crossRe[i] + crossIm[i] * crossIm[i];
        if (v > peakVal) { peakVal = v; peakIdx = i; }
    }

    /* 亚像素精度 */
    double subPixel = subPixelPeak1D(crossRe, peakIdx);

    /* 处理环绕: 如果峰值在右半部，表示负位移 */
    double dx = static_cast<double>(peakIdx);
    if (peakIdx > len / 2) dx -= len;
    dx += subPixel;

    double confidence = std::sqrt(peakVal);

    ++m_stats.totalCorrelations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;

    emit correlationCompleted(dx, 0.0, confidence);
    return {dx, 0.0, confidence, std::sqrt(peakVal)};
}

/** @brief 2D图像相位相关 @param image1 参考图像 @param image2 待对齐图像 @return 位移结果 */
PhaseCorrelate::ShiftResult PhaseCorrelate::correlate2D(
    const QImage& image1, const QImage& image2)
{
    if (image1.isNull() || image2.isNull()) return {0.0, 0.0, 0.0, 0.0};

    int rows = std::max(image1.height(), image2.height());
    int cols = std::max(image1.width(), image2.width());
    rows = nextPowerOfTwo(rows);
    cols = nextPowerOfTwo(cols);

    /* 转灰度并补零 */
    QVector<double> mat1, mat2;
    mat1.resize(rows * cols); mat2.resize(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            mat1[idx] = (c < image1.width() && r < image1.height())
                ? qGray(image1.pixelColor(c, r).rgb()) / 255.0 : 0.0;
            mat2[idx] = (c < image2.width() && r < image2.height())
                ? qGray(image2.pixelColor(c, r).rgb()) / 255.0 : 0.0;
        }
    }

    return correlate2DMatrix(mat1, mat2, rows, cols);
}

/** @brief 2D矩阵相位相关 */
PhaseCorrelate::ShiftResult PhaseCorrelate::correlate2DMatrix(
    const QVector<double>& mat1, const QVector<double>& mat2,
    int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    int total = rows * cols;
    QVector<double> re1 = mat1, im1(total, 0.0);
    QVector<double> re2 = mat2, im2(total, 0.0);

    /* 补零到2的幂次 */
    re1.resize(total); im1.resize(total);
    re2.resize(total); im2.resize(total);

    /* 2D FFT */
    fft2D(re1, im1, rows, cols, false);
    fft2D(re2, im2, rows, cols, false);

    /* 互功率谱 */
    QVector<double> crossRe(total), crossIm(total);
    for (int i = 0; i < total; ++i) {
        crossRe[i] = re1[i] * re2[i] + im1[i] * im2[i];
        crossIm[i] = im1[i] * re2[i] - re1[i] * im2[i];
        double mag = std::sqrt(crossRe[i] * crossRe[i] + crossIm[i] * crossIm[i]);
        if (mag > 1e-12) {
            crossRe[i] /= mag;
            crossIm[i] /= mag;
        }
    }

    /* IFFT */
    fft2D(crossRe, crossIm, rows, cols, true);

    /* 找峰值 */
    int peakR = 0, peakC = 0;
    double peakVal = 0.0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double v = crossRe[r * cols + c];
            if (v > peakVal) { peakVal = v; peakR = r; peakC = c; }
        }
    }

    double dy = static_cast<double>(peakR);
    if (peakR > rows / 2) dy -= rows;
    double dx = static_cast<double>(peakC);
    if (peakC > cols / 2) dx -= cols;

    double confidence = std::abs(peakVal);

    ++m_stats.totalCorrelations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;

    emit correlationCompleted(dx, dy, confidence);
    return {dx, dy, confidence, std::abs(peakVal)};
}

/** @brief 计算互功率谱 */
QVector<double> PhaseCorrelate::crossPowerSpectrum(
    const QVector<double>& fft1, const QVector<double>& fft2) const
{
    int half = fft1.size() / 2;
    QVector<double> result(fft1.size());
    for (int i = 0; i < half; ++i) {
        double re1 = fft1[i * 2], im1 = fft1[i * 2 + 1];
        double re2 = fft2[i * 2], im2 = fft2[i * 2 + 1];
        double crossRe = re1 * re2 + im1 * im2;
        double crossIm = im1 * re2 - re1 * im2;
        double mag = std::sqrt(crossRe * crossRe + crossIm * crossIm);
        if (mag > 1e-12) {
            result[i * 2] = crossRe / mag;
            result[i * 2 + 1] = crossIm / mag;
        } else {
            result[i * 2] = 0.0;
            result[i * 2 + 1] = 0.0;
        }
    }
    return result;
}

/** @brief 亚像素精度峰值检测(质心法) @param data 相关输出 @param peakIdx 峰值位置 @return 亚像素偏移 */
double PhaseCorrelate::subPixelPeak1D(const QVector<double>& data, int peakIdx) const
{
    int n = data.size();
    if (peakIdx <= 0 || peakIdx >= n - 1) return 0.0;

    /* 拟合抛物线: y = a*x^2 + b*x + c, 峰值偏移 = -b/(2a) */
    double y0 = data[peakIdx - 1];
    double y1 = data[peakIdx];
    double y2 = data[peakIdx + 1];

    double denom = y0 - 2.0 * y1 + y2;
    if (std::abs(denom) < 1e-12) return 0.0;

    double delta = (y0 - y2) / (2.0 * denom);
    return std::max(-0.5, std::min(0.5, delta));
}

/** @brief 重置统计 */
void PhaseCorrelate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

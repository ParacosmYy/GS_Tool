/**
 * @file SymmetricWavelet.cpp
 * @brief 对称小波变换实现 — 边界修正DWT
 */

#include "utils/dwt3/SymmetricWavelet.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SymmetricWavelet::SymmetricWavelet(QObject* parent)
    : QObject(parent)
    , m_originalSize(0)
    , m_timeSum(0.0)
{
}

/** @brief 正向小波变换
 *  @param signal 输入信号
 *  @return 小波系数 */
QVector<double> SymmetricWavelet::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 2) {
        return signal;
    }

    m_originalSize = n;

    /* Haar小波的低通和高通滤波器 */
    static const double lo[] = { 1.0 / std::sqrt(2.0), 1.0 / std::sqrt(2.0) };
    static const double hi[] = { 1.0 / std::sqrt(2.0), -1.0 / std::sqrt(2.0) };

    QVector<double> coeffs = signal;
    int currentLen = n;

    /* 逐层分解 */
    while (currentLen >= 2) {
        QVector<double> temp(currentLen);
        int half = currentLen / 2;

        for (int i = 0; i < half; ++i) {
            int idx0 = 2 * i;
            int idx1 = 2 * i + 1;

            /* 边界检查: 使用对称扩展 */
            double v0 = coeffs[idx0];
            double v1 = (idx1 < currentLen) ? coeffs[idx1] : coeffs[2 * currentLen - 2 - idx1];

            /* 低通(近似系数) */
            double approx = lo[0] * v0 + lo[1] * v1;
            /* 高通(细节系数) */
            double detail = hi[0] * v0 + hi[1] * v1;

            temp[i] = approx;
            temp[half + i] = detail;
        }

        for (int i = 0; i < currentLen; ++i) {
            coeffs[i] = temp[i];
        }
        currentLen = half;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(coeffs.size());
    return coeffs;
}

/** @brief 逆小波变换
 *  @param coeffs 小波系数
 *  @return 重建信号 */
QVector<double> SymmetricWavelet::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_originalSize;
    if (n < 2 || coeffs.size() < n) {
        return coeffs;
    }

    static const double lo[] = { 1.0 / std::sqrt(2.0), 1.0 / std::sqrt(2.0) };
    static const double hi[] = { 1.0 / std::sqrt(2.0), -1.0 / std::sqrt(2.0) };

    QVector<double> reconstructed = coeffs.mid(0, n);

    /* 确定最小层的大小 */
    int halfLen = 1;
    while (halfLen * 2 <= n) halfLen *= 2;
    halfLen /= 2;

    /* 逐层重建 */
    while (halfLen < n) {
        QVector<double> temp(halfLen * 2);

        for (int i = 0; i < halfLen; ++i) {
            double approx = reconstructed[i];
            double detail = reconstructed[halfLen + i];

            temp[2 * i] = lo[0] * approx + hi[0] * detail;
            if (2 * i + 1 < halfLen * 2) {
                temp[2 * i + 1] = lo[1] * approx + hi[1] * detail;
            }
        }

        for (int i = 0; i < halfLen * 2 && i < n; ++i) {
            reconstructed[i] = temp[i];
        }
        halfLen *= 2;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(n);
    return reconstructed;
}

/** @brief 阈值去噪
 *  @param signal 输入信号
 *  @param threshold 软阈值参数
 *  @return 去噪后信号 */
QVector<double> SymmetricWavelet::denoise(const QVector<double>& signal,
                                          double threshold)
{
    QElapsedTimer timer;
    timer.start();

    /* 正变换 */
    QVector<double> coeffs = forward(signal);

    /* 对细节系数(后半部分)施加软阈值 */
    int n = signal.size();
    int currentLen = n;
    int start = currentLen / 2;

    while (currentLen >= 4) {
        int half = currentLen / 2;
        for (int i = half; i < currentLen && i < coeffs.size(); ++i) {
            coeffs[i] = softThreshold(coeffs[i], threshold);
        }
        currentLen = half;
    }

    /* 逆变换 */
    QVector<double> result = inverse(coeffs);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(result.size());
    return result;
}

/** @brief 重置统计 */
void SymmetricWavelet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 对称扩展卷积 */
QVector<double> SymmetricWavelet::symConvolve(const QVector<double>& signal,
                                              const QVector<double>& filter) const
{
    int n = signal.size();
    int fLen = filter.size();
    int halfF = fLen / 2;
    QVector<double> result(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < fLen; ++j) {
            int idx = i - halfF + j;
            /* 对称边界扩展 */
            if (idx < 0) idx = -idx;
            if (idx >= n) idx = 2 * n - 2 - idx;
            idx = qBound(0, idx, n - 1);
            sum += signal[idx] * filter[j];
        }
        result[i] = sum;
    }
    return result;
}

/** @brief 软阈值函数 */
double SymmetricWavelet::softThreshold(double x, double t) const
{
    if (x > t) return x - t;
    if (x < -t) return x + t;
    return 0.0;
}

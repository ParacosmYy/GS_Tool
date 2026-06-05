/**
 * @file ConvolutionEngine.cpp
 * @brief 卷积引擎实现 — 直接卷积/FFT卷积/互相关
 */

#include "utils/convolution/ConvolutionEngine.h"

#include <QtMath>
#include <QElapsedTimer>

ConvolutionEngine::ConvolutionEngine(QObject* parent)
    : QObject(parent), m_borderMode(BorderMode::Zero) {}

void ConvolutionEngine::setBorderMode(BorderMode mode) { m_borderMode = mode; }

/** @brief 直接卷积 @param signal 信号 @param kernel 核 @return 结果 */
QVector<double> ConvolutionEngine::convolve(
    const QVector<double>& signal, const QVector<double>& kernel)
{
    if (signal.isEmpty() || kernel.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int k = kernel.size();
    int outLen = n + k - 1;
    QVector<double> output(outLen, 0.0);

    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < k; ++j) {
            int idx = i - j;
            sum += borderValue(signal, idx) * kernel[j];
        }
        output[i] = sum;
    }

    m_stats.totalConvolutions++;
    m_stats.totalPointsProcessed += static_cast<quint64>(n + k);
    emit convolutionComplete(outLen);
    return output;
}

/** @brief FFT卷积(简化: 使用直接卷积) @param signal 信号 @param kernel 核 @return 结果 */
QVector<double> ConvolutionEngine::convolveFft(
    const QVector<double>& signal, const QVector<double>& kernel)
{
    /* FFT卷积暂回退到直接卷积 */
    m_stats.totalFftConvolutions++;
    return convolve(signal, kernel);
}

/** @brief 互相关 @param signal 信号 @param kernel 核 @return 结果 */
QVector<double> ConvolutionEngine::correlate(
    const QVector<double>& signal, const QVector<double>& kernel)
{
    if (signal.isEmpty() || kernel.isEmpty()) return {};

    int n = signal.size();
    int k = kernel.size();
    int outLen = n + k - 1;
    QVector<double> output(outLen, 0.0);

    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < k; ++j) {
            int idx = i - k + 1 + j;
            sum += borderValue(signal, idx) * kernel[k - 1 - j];
        }
        output[i] = sum;
    }

    m_stats.totalConvolutions++;
    m_stats.totalPointsProcessed += static_cast<quint64>(n + k);
    emit convolutionComplete(outLen);
    return output;
}

void ConvolutionEngine::resetStatistics() { m_stats = Stats{}; }

/** @brief 边界值 @param data 数据 @param index 索引 @return 值 */
double ConvolutionEngine::borderValue(const QVector<double>& data, int index) const
{
    int n = data.size();
    if (index >= 0 && index < n) return data[index];

    switch (m_borderMode) {
    case BorderMode::Zero:
        return 0.0;
    case BorderMode::Replicate:
        return (index < 0) ? data[0] : data[n - 1];
    case BorderMode::Reflect:
        if (index < 0) index = -index;
        if (index >= n) index = 2 * (n - 1) - index;
        return data[qBound(0, index, n - 1)];
    case BorderMode::Wrap:
        return data[((index % n) + n) % n];
    }
    return 0.0;
}

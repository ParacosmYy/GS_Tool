#include "SpectralFlatness6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱平坦度分析器
 * @param parent 父对象指针
 */
SpectralFlatness6::SpectralFlatness6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SpectralFlatness6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算全频段频谱平坦度
 *
 * 频谱平坦度(Wiener熵) = 几何均值 / 算术均值
 * 值域[0, 1]：
 * - 接近1：类噪声信号（频谱平坦）
 * - 接近0：类音调信号（频谱有尖峰）
 *
 * 使用对数域计算几何均值以避免数值溢出。
 *
 * @param powerSpectrum 功率谱序列
 * @return 频谱平坦度值 [0, 1]
 */
double SpectralFlatness6::compute(const QVector<double>& powerSpectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int n = powerSpectrum.size();
    if (n == 0) {
        emit analysisCompleted(0);
        return 0.0;
    }

    /* 计算算术均值 */
    double arithmeticMean = 0.0;
    for (int i = 0; i < n; ++i) {
        arithmeticMean += qMax(powerSpectrum[i], 1e-20);
    }
    arithmeticMean /= n;

    if (arithmeticMean < 1e-20) {
        emit analysisCompleted(0);
        return 0.0;
    }

    /* 计算几何均值（对数域） */
    double logSum = 0.0;
    for (int i = 0; i < n; ++i) {
        logSum += qLn(qMax(powerSpectrum[i], 1e-20));
    }
    double geometricMean = qExp(logSum / n);

    /* 频谱平坦度 */
    double flatness = geometricMean / arithmeticMean;
    flatness = qBound(0.0, flatness, 1.0);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalysisOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysisOps;

    emit analysisCompleted(1);
    return flatness;
}

/**
 * @brief 计算分频段频谱平坦度
 *
 * 将功率谱均匀分成numBands个频段，
 * 分别计算每个频段的平坦度指标。
 *
 * @param powerSpectrum 功率谱序列
 * @param numBands 频段数量
 * @return 各频段的平坦度值
 */
QVector<double> SpectralFlatness6::computePerBand(
    const QVector<double>& powerSpectrum, int numBands)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> bandFlatness;
    const int n = powerSpectrum.size();
    numBands = qBound(1, numBands, n);

    if (n == 0) {
        emit analysisCompleted(0);
        return bandFlatness;
    }

    int bandSize = n / numBands;
    bandFlatness.resize(numBands);

    for (int b = 0; b < numBands; ++b) {
        int start = b * bandSize;
        int end = (b == numBands - 1) ? n : start + bandSize;
        int count = end - start;

        if (count <= 0) {
            bandFlatness[b] = 0.0;
            continue;
        }

        double arithMean = 0.0;
        double logSum = 0.0;
        for (int i = start; i < end; ++i) {
            double val = qMax(powerSpectrum[i], 1e-20);
            arithMean += val;
            logSum += qLn(val);
        }
        arithMean /= count;

        if (arithMean < 1e-20) {
            bandFlatness[b] = 0.0;
        } else {
            double geoMean = qExp(logSum / count);
            bandFlatness[b] = qBound(0.0, geoMean / arithMean, 1.0);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalysisOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysisOps;

    emit analysisCompleted(numBands);
    return bandFlatness;
}

/**
 * @brief 设置分析窗口参数
 * @param fftSize FFT大小
 * @param sampleRate 采样率(Hz)
 */
void SpectralFlatness6::setWindowParameters(int fftSize, double sampleRate)
{
    Q_UNUSED(fftSize)
    Q_UNUSED(sampleRate)
}

/**
 * @file SpectralContrast2.cpp
 * @brief 频谱对比度特征提取实现（第2版）
 *
 * 将频谱划分为多个子带，计算每个子带的峰值能量与谷值能量之差，
 * 作为频谱对比度特征。该特征可反映频谱的谐波结构和峰值突出程度。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal66/SpectralContrast2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱对比度提取器
 * @param parent 父QObject对象指针
 */
SpectralContrast2::SpectralContrast2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void SpectralContrast2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT窗口大小
 */
void SpectralContrast2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 设置子带数量
 * @param bands 频带划分数量
 */
void SpectralContrast2::setNumBands(int bands)
{
    m_numBands = qMax(1, bands);
}

/**
 * @brief 计算频谱对比度特征
 *
 * 将频谱均匀划分为numBands个子带，在每个子带中
 * 找到能量最高的alpha比例峰值和最低的alpha比例谷值，
 * 计算对比度 = log(peak/valley)。
 *
 * @param spectrum 输入的幅度频谱（FFT前半部分）
 * @return 每个子带的对比度值向量
 */
QVector<double> SpectralContrast2::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    m_valleys.clear();
    m_peaks.clear();
    QVector<double> contrast;

    if (spectrum.isEmpty()) {
        emit computed(0, 0.0);
        return contrast;
    }

    int specLen = spectrum.size();
    double binWidth = m_sampleRate / m_fftSize;

    /* 将频谱划分为对数间隔的子带 */
    double minFreq = binWidth;  /* 最低频率bin */
    double maxFreq = m_sampleRate / 2.0;  /* Nyquist频率 */
    if (specLen < m_numBands * 2) {
        /* 频谱太短，直接均匀划分 */
        m_numBands = qMax(1, specLen / 2);
    }

    /* 计算每个子带的频率边界 */
    QVector<int> bandBorders(m_numBands + 1);
    double logMin = qLn(qMax(1.0, minFreq));
    double logMax = qLn(qMax(2.0, maxFreq));

    for (int i = 0; i <= m_numBands; ++i) {
        double logFreq = logMin + (logMax - logMin) * i / m_numBands;
        double freq = qExp(logFreq);
        int bin = static_cast<int>(freq / binWidth);
        bandBorders[i] = qBound(0, bin, specLen - 1);
    }

    /* 确保边界单调递增 */
    for (int i = 1; i <= m_numBands; ++i) {
        if (bandBorders[i] <= bandBorders[i - 1]) {
            bandBorders[i] = bandBorders[i - 1] + 1;
        }
        if (bandBorders[i] >= specLen) {
            bandBorders[i] = specLen - 1;
        }
    }

    /* 峰值/谷值比例参数 */
    double alpha = 0.02;  /* 取最高/最低2%的bin */

    contrast.resize(m_numBands);
    m_peaks.resize(m_numBands);
    m_valleys.resize(m_numBands);

    for (int b = 0; b < m_numBands; ++b) {
        int lo = bandBorders[b];
        int hi = bandBorders[b + 1];
        int bandSize = hi - lo;

        if (bandSize <= 0) {
            contrast[b] = 0.0;
            m_peaks[b] = 1e-10;
            m_valleys[b] = 1e-10;
            continue;
        }

        /* 收集子带内所有频谱值并排序 */
        QVector<double> bandVals;
        bandVals.reserve(bandSize);
        for (int i = lo; i < hi; ++i) {
            bandVals.append(qMax(1e-10, spectrum[i]));
        }
        std::sort(bandVals.begin(), bandVals.end());

        /* 取最低alpha比例为谷值，最高alpha比例为峰值 */
        int valleyCount = qMax(1, static_cast<int>(bandSize * alpha));
        int peakCount = qMax(1, static_cast<int>(bandSize * alpha));

        double valleySum = 0.0;
        for (int i = 0; i < valleyCount && i < bandVals.size(); ++i) {
            valleySum += bandVals[i];
        }
        double valley = valleySum / valleyCount;

        double peakSum = 0.0;
        for (int i = bandVals.size() - 1;
             i >= bandVals.size() - peakCount && i >= 0; --i) {
            peakSum += bandVals[i];
        }
        double peak = peakSum / peakCount;

        m_peaks[b] = peak;
        m_valleys[b] = valley;

        /* 计算对比度：log(peak/valley) */
        contrast[b] = qLn(peak / qMax(1e-10, valley));
    }

    /* 计算平均对比度 */
    double avgContrast = 0.0;
    for (double c : contrast) avgContrast += c;
    avgContrast /= contrast.size();

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_numBands, avgContrast);
    return contrast;
}

/**
 * @brief 获取当前统计信息
 * @return 计算统计结构
 */
SpectralContrast2::Stats SpectralContrast2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralContrast2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

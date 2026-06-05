/**
 * @file SpectralFlatness2.cpp
 * @brief 频谱平坦度计算实现 — Wiener熵度量
 *
 * 计算功率谱的几何平均与算术平均之比(频谱平坦度/Wiener熵)，
 * 用于区分音调信号(低平坦度)与噪声信号(高平坦度)。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal64/SpectralFlatness2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
SpectralFlatness2::SpectralFlatness2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置FFT大小
 * @param n FFT大小，必须 >= 2
 */
void SpectralFlatness2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 设置频带范围
 * @param loBin 低频bin索引(包含)
 * @param hiBin 高频bin索引(包含)
 */
void SpectralFlatness2::setBandRange(int loBin, int hiBin)
{
    m_loBin = qMax(0, loBin);
    m_hiBin = qMax(m_loBin, hiBin);
}

/**
 * @brief 计算频谱平坦度
 *
 * 频谱平坦度 = 几何平均 / 算术平均
 * 值接近1表示类噪声信号，值接近0表示音调信号。
 * 使用对数空间计算几何平均以避免数值溢出。
 *
 * @param spectrum 功率谱(幅度平方)
 * @return 频谱平坦度值，范围[0, 1]
 */
double SpectralFlatness2::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int totalBins = spectrum.size();
    if (totalBins == 0) {
        m_flatness = 0.0;
        m_geoMean = 0.0;
        m_ariMean = 0.0;
        return 0.0;
    }

    /* 确定有效的bin范围 */
    int lo = qBound(0, m_loBin, totalBins - 1);
    int hi = qBound(lo, m_hiBin, totalBins - 1);
    const int numBins = hi - lo + 1;

    if (numBins == 0) {
        m_flatness = 0.0;
        m_geoMean = 0.0;
        m_ariMean = 0.0;
        return 0.0;
    }

    /* 计算算术平均 */
    double sum = 0.0;
    for (int i = lo; i <= hi; ++i) {
        sum += qMax(1e-20, spectrum[i]);
    }
    m_ariMean = sum / static_cast<double>(numBins);

    /* 计算几何平均 (使用对数求和避免溢出) */
    double logSum = 0.0;
    int validCount = 0;
    for (int i = lo; i <= hi; ++i) {
        double val = qMax(1e-20, spectrum[i]);
        logSum += qLn(val);
        ++validCount;
    }

    if (validCount > 0) {
        m_geoMean = qExp(logSum / static_cast<double>(validCount));
    } else {
        m_geoMean = 0.0;
    }

    /* 计算平坦度 */
    m_flatness = 0.0;
    if (m_ariMean > 1e-20) {
        m_flatness = m_geoMean / m_ariMean;
    }

    /* 确保结果在[0, 1]范围内 */
    m_flatness = qBound(0.0, m_flatness, 1.0);

    /* 更新统计信息 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_flatness, -m_flatness); /* Wiener熵 = -flatness (负对数形式) */
    return m_flatness;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralFlatness2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算子带平坦度
 *
 * 将频谱均匀分割为numBands个子带，分别计算每个子带的平坦度。
 * 用于多分辨率频谱分析，识别不同频段的音调/噪声特性。
 *
 * @param spectrum 功率谱
 * @param numBands 子带数量
 * @return 每个子带的平坦度值
 */
QVector<double> SpectralFlatness2::computeSubbandFlatness(
    const QVector<double>& spectrum, int numBands) const
{
    const int totalBins = spectrum.size();
    QVector<double> flatness(numBands, 0.0);

    if (totalBins == 0 || numBands <= 0) return flatness;

    /* 计算每个子带的bin范围 */
    int binsPerBand = totalBins / numBands;
    if (binsPerBand < 1) binsPerBand = 1;

    for (int b = 0; b < numBands; ++b) {
        int lo = b * binsPerBand;
        int hi = qMin((b + 1) * binsPerBand - 1, totalBins - 1);
        if (lo >= totalBins) break;

        int count = hi - lo + 1;
        if (count <= 0) continue;

        /* 算术平均 */
        double arSum = 0.0;
        for (int i = lo; i <= hi; ++i) {
            arSum += qMax(1e-20, spectrum[i]);
        }
        double am = arSum / static_cast<double>(count);

        /* 几何平均 (对数空间) */
        double logSum = 0.0;
        for (int i = lo; i <= hi; ++i) {
            logSum += qLn(qMax(1e-20, spectrum[i]));
        }
        double gm = qExp(logSum / static_cast<double>(count));

        if (am > 1e-20) {
            flatness[b] = qBound(0.0, gm / am, 1.0);
        }
    }
    return flatness;
}

/**
 * @brief 批量计算多帧频谱平坦度
 *
 * 对多帧频谱数据逐一计算平坦度，返回每帧的结果。
 * 适用于实时音频处理中连续帧的分析。
 *
 * @param frames 多帧频谱数据
 * @return 每帧的平坦度值
 */
QVector<double> SpectralFlatness2::computeBatch(
    const QVector<QVector<double>>& frames)
{
    const int numFrames = frames.size();
    QVector<double> results(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        results[f] = compute(frames[f]);
    }
    return results;
}

/**
 * @brief 计算频谱的峰值因子
 *
 * 峰值因子 = 最大值 / 算术平均，用于补充平坦度分析。
 * 高峰值因子表示存在强音调成分。
 *
 * @param spectrum 功率谱
 * @return 峰值因子
 */
double SpectralFlatness2::crestFactor(const QVector<double>& spectrum) const
{
    const int totalBins = spectrum.size();
    if (totalBins == 0) return 0.0;

    int lo = qBound(0, m_loBin, totalBins - 1);
    int hi = qBound(lo, m_hiBin, totalBins - 1);
    int count = hi - lo + 1;
    if (count <= 0) return 0.0;

    double maxVal = 0.0;
    double sum = 0.0;
    for (int i = lo; i <= hi; ++i) {
        double val = qMax(0.0, spectrum[i]);
        maxVal = qMax(maxVal, val);
        sum += val;
    }

    double mean = sum / static_cast<double>(count);
    return (mean > 1e-20) ? maxVal / mean : 0.0;
}

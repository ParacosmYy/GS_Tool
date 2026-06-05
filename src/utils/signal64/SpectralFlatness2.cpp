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

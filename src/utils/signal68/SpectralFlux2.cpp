/**
 * @file SpectralFlux2.cpp
 * @brief 频谱通量计算实现（第2版）
 *
 * 计算连续频谱帧之间的通量（变化量），用于音符起始检测
 * 和音乐节拍分析。支持正向通量（仅计算增加部分）、
 * 全通量和绝对差三种模式。同时检测通量峰值位置。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal68/SpectralFlux2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱通量计算器
 * @param parent 父QObject对象指针
 */
SpectralFlux2::SpectralFlux2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置FFT大小
 * @param n FFT窗口大小
 */
void SpectralFlux2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 设置帧移（跳跃）大小
 * @param hop 相邻帧之间的采样数
 */
void SpectralFlux2::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置通量类型
 * @param type 通量计算方式："positive"仅正向, "full"全通量, "abs"绝对差
 */
void SpectralFlux2::setFluxType(const QString& type)
{
    if (type == "positive" || type == "full" || type == "abs") {
        m_type = type;
    }
}

/**
 * @brief 计算信号的频谱通量序列
 *
 * 将信号分帧，对每帧计算FFT得到频谱，然后计算相邻帧之间的
 * 频谱差异作为通量值。最后检测通量序列中的峰值位置。
 *
 * @param signal 输入的时域信号
 * @return 每帧的通量值序列
 */
QVector<double> SpectralFlux2::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    m_totalFlux = 0.0;
    m_peaks.clear();
    QVector<double> fluxValues;

    if (signal.isEmpty()) {
        emit computed(0.0, 0);
        return fluxValues;
    }

    int n = signal.size();
    int numFrames = (n - m_fftSize) / m_hopSize + 1;
    if (numFrames <= 0) numFrames = 1;

    /* 逐帧计算频谱 */
    QVector<QVector<double>> spectra;
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        if (start + m_fftSize > n) break;

        /* 加Hann窗并计算FFT幅度谱 */
        QVector<double> spectrum(m_fftSize / 2 + 1, 0.0);
        for (int k = 0; k <= m_fftSize / 2; ++k) {
            double re = 0.0, im = 0.0;
            for (int i = 0; i < m_fftSize; ++i) {
                double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
                double sample = signal[start + i] * w;
                double angle = 2.0 * M_PI * k * i / m_fftSize;
                re += sample * qCos(angle);
                im -= sample * qSin(angle);
            }
            spectrum[k] = qSqrt(re * re + im * im);
        }
        spectra.append(spectrum);
    }

    /* 计算相邻帧之间的频谱通量 */
    if (spectra.size() < 2) {
        fluxValues.resize(spectra.size(), 0.0);
    } else {
        fluxValues.resize(spectra.size() - 1);
        for (int f = 0; f < static_cast<int>(spectra.size()) - 1; ++f) {
            double flux = 0.0;
            int specLen = spectra[f].size();

            for (int k = 0; k < specLen; ++k) {
                double diff = spectra[f + 1][k] - spectra[f][k];

                if (m_type == "positive") {
                    /* 仅计算频谱增加的部分 */
                    flux += (diff > 0) ? diff * diff : 0.0;
                } else if (m_type == "full") {
                    /* 计算差的平方 */
                    flux += diff * diff;
                } else {
                    /* 绝对差 */
                    flux += qAbs(diff);
                }
            }

            if (m_type == "abs") {
                fluxValues[f] = flux;
            } else {
                fluxValues[f] = qSqrt(flux);
            }
        }
    }

    /* 计算总通量 */
    for (double f : fluxValues) m_totalFlux += f;

    /* 检测峰值 */
    double threshold = 0.0;
    for (double f : fluxValues) threshold += f;
    threshold = (fluxValues.size() > 0) ? threshold / fluxValues.size() : 0.0;
    threshold *= 1.5; /* 峰值阈值为均值的1.5倍 */

    for (int i = 1; i < fluxValues.size() - 1; ++i) {
        if (fluxValues[i] > fluxValues[i - 1] &&
            fluxValues[i] > fluxValues[i + 1] &&
            fluxValues[i] > threshold) {
            m_peaks.append(i);
        }
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalFrames += numFrames;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_totalFlux, m_peaks.size());
    return fluxValues;
}

/**
 * @brief 获取当前统计信息
 * @return 计算统计结构
 */
SpectralFlux2::Stats SpectralFlux2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralFlux2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

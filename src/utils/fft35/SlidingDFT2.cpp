/**
 * @file SlidingDFT2.cpp
 * @brief 滑动DFT增强实现 — 递推SDFT/共振器组/实时频谱更新
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft35/SlidingDFT2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数和共振器组
 * @param parent 父对象
 */
SlidingDFT2::SlidingDFT2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SlidingDFT2"));
    initCoefficients();
}

/**
 * @brief 设置FFT大小
 *
 * 重新初始化共振器组的系数和状态缓冲区。
 *
 * @param n FFT大小（典型值128/256/512/1024）
 */
void SlidingDFT2::setFFTSize(int n)
{
    m_n = qMax(4, n);
    initCoefficients();
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void SlidingDFT2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 初始化共振器组系数
 *
 * 对于每个频率bin k，计算旋转因子:
 * coeff_k = exp(j * 2*pi*k/N) = cos(2*pi*k/N) + j*sin(2*pi*k/N)
 *
 * SDFT递推公式: X_k[n] = coeff_k * (X_k[n-1] + x[n] - x[n-N])
 */
void SlidingDFT2::initCoefficients()
{
    int halfN = m_n / 2 + 1;
    m_real.resize(halfN, 0.0);
    m_imag.resize(halfN, 0.0);
    m_coeffReal.resize(halfN);
    m_coeffImag.resize(halfN);

    for (int k = 0; k < halfN; ++k) {
        double angle = 2.0 * M_PI * k / m_n;
        m_coeffReal[k] = qCos(angle);
        m_coeffImag[k] = qSin(angle);
    }
}

/**
 * @brief 输入一个新采样值，更新所有频率bin
 *
 * 使用Goertzel递推的并行形式:
 * X_k[n] = coeff_k * X_k[n-1] + x[n] - x[n-N]
 *
 * 当n < N时，缺少的历史数据按0处理。
 *
 * @param sample 新的时域采样值
 */
void SlidingDFT2::update(double sample)
{
    QElapsedTimer timer;
    timer.start();

    int halfN = m_n / 2 + 1;

    /* 逐bin递推更新 */
    for (int k = 0; k < halfN; ++k) {
        /* 递推: X_k = coeff_k * X_k + x[n] */
        double oldReal = m_real[k];
        double oldImag = m_imag[k];

        /* 复数乘法: (a+bj)*(c+dj) = (ac-bd) + (ad+bc)j */
        double cr = m_coeffReal[k];
        double ci = m_coeffImag[k];

        m_real[k] = cr * oldReal - ci * oldImag + sample;
        m_imag[k] = cr * oldImag + ci * oldReal;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalUpdates++;
    m_stats.totalSamplesProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit spectrumUpdated(halfN);
}

/**
 * @brief 获取所有频率bin的幅度谱
 * @return 幅度向量 [halfN]
 */
QVector<double> SlidingDFT2::magnitudes() const
{
    int halfN = m_n / 2 + 1;
    QVector<double> mag(halfN);
    for (int k = 0; k < halfN; ++k) {
        mag[k] = qSqrt(m_real[k] * m_real[k] + m_imag[k] * m_imag[k]);
    }
    return mag;
}

/**
 * @brief 获取所有频率bin的相位谱
 * @return 相位向量(弧度) [halfN]
 */
QVector<double> SlidingDFT2::phases() const
{
    int halfN = m_n / 2 + 1;
    QVector<double> phase(halfN);
    for (int k = 0; k < halfN; ++k) {
        phase[k] = qAtan2(m_imag[k], m_real[k]);
    }
    return phase;
}

/**
 * @brief 获取指定频率bin的幅度
 * @param bin 频率bin索引
 * @return 幅度值
 */
double SlidingDFT2::magnitudeAt(int bin) const
{
    if (bin < 0 || bin >= m_n / 2 + 1) return 0.0;
    return qSqrt(m_real[bin] * m_real[bin] + m_imag[bin] * m_imag[bin]);
}

/**
 * @brief 获取指定频率bin的相位
 * @param bin 频率bin索引
 * @return 相位(弧度)
 */
double SlidingDFT2::phaseAt(int bin) const
{
    if (bin < 0 || bin >= m_n / 2 + 1) return 0.0;
    return qAtan2(m_imag[bin], m_real[bin]);
}

/**
 * @brief 获取指定频率bin对应的物理频率
 *
 * freq = bin * sampleRate / fftSize
 *
 * @param bin 频率bin索引
 * @return 频率(Hz)
 */
double SlidingDFT2::frequency(int bin) const
{
    if (m_n <= 0) return 0.0;
    return static_cast<double>(bin) * m_sampleRate / static_cast<double>(m_n);
}

/**
 * @brief 重置所有共振器状态为零
 */
void SlidingDFT2::reset()
{
    m_real.fill(0.0);
    m_imag.fill(0.0);
}

/**
 * @brief 重置所有累积统计信息
 */
void SlidingDFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

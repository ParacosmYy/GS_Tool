/**
 * @file SlidingDFT2.cpp
 * @brief 滑动DFT增强实现 — 递推SDFT/共振器组/实时频谱更新
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 滑动DFT(Sliding DFT)是一种O(N)逐采样更新的频谱分析方法。
 * 与传统FFT需要收集完整帧不同，SDFT每收到一个新采样就立即
 * 更新所有频率bin，适合实时频谱监测场景。
 *
 * 核心原理: 每个频率bin维护一个共振器，通过递推关系
 * X_k[n] = e^(j*2pi*k/N) * X_k[n-1] + x[n]
 * 实现O(1)每bin的更新复杂度。
 *
 * 稳定性: 标准SDFT存在极点在单位圆上的稳定性问题。
 * 本实现使用简化递推（不加x[n-N]减法项），适合短时频谱分析。
 * 对于长时间运行，建议定期调用reset()防止数值漂移。
 */

#include "utils/fft35/SlidingDFT2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数和共振器组
 * @param parent 父对象
 *
 * 默认FFT大小256，采样率44100Hz。构造后共振器组处于零状态。
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
 * 调用此方法会清空所有累积的频谱状态。
 *
 * @param n FFT大小（典型值128/256/512/1024，建议为2的幂次）
 */
void SlidingDFT2::setFFTSize(int n)
{
    m_n = qMax(4, n);
    initCoefficients();
}

/**
 * @brief 设置采样率
 *
 * 采样率用于将频率bin索引转换为物理频率(Hz)。
 * 不影响内部递推计算。
 *
 * @param rate 采样率(Hz)，如44100、48000
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
 * SDFT递推公式: X_k[n] = coeff_k * X_k[n-1] + x[n]
 *
 * 只计算正频率部分(N/2+1个bin)，因为实信号的频谱具有共轭对称性。
 * DC分量(k=0)和Nyquist分量(k=N/2)是纯实数。
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
 * 使用一阶递推形式:
 * X_k[n] = coeff_k * X_k[n-1] + x[n]
 *
 * 展开为实部和虚部:
 * Re(X_k[n]) = cos(theta_k) * Re(X_k[n-1]) - sin(theta_k) * Im(X_k[n-1]) + x[n]
 * Im(X_k[n]) = cos(theta_k) * Im(X_k[n-1]) + sin(theta_k) * Re(X_k[n-1])
 *
 * 计算复杂度: O(N/2) 每采样点，其中N为FFT大小。
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
        /* 保存当前状态用于复数乘法 */
        double oldReal = m_real[k];
        double oldImag = m_imag[k];

        /* 旋转因子 */
        double cr = m_coeffReal[k];
        double ci = m_coeffImag[k];

        /* 复数乘法 (oldReal + j*oldImag) * (cr + j*ci)
         * = (oldReal*cr - oldImag*ci) + j*(oldReal*ci + oldImag*cr) */
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
 *
 * 幅度 = sqrt(Re^2 + Im^2)，表示每个频率分量的能量。
 *
 * @return 幅度向量 [N/2+1]
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
 *
 * 相位 = atan2(Im, Re)，表示每个频率分量的初始相位。
 * 相位范围 [-pi, pi]。
 *
 * @return 相位向量(弧度) [N/2+1]
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
 * @param bin 频率bin索引 (0 ~ N/2)
 * @return 幅度值，无效索引返回0
 */
double SlidingDFT2::magnitudeAt(int bin) const
{
    if (bin < 0 || bin >= m_n / 2 + 1) return 0.0;
    return qSqrt(m_real[bin] * m_real[bin] + m_imag[bin] * m_imag[bin]);
}

/**
 * @brief 获取指定频率bin的相位
 * @param bin 频率bin索引 (0 ~ N/2)
 * @return 相位(弧度)，无效索引返回0
 */
double SlidingDFT2::phaseAt(int bin) const
{
    if (bin < 0 || bin >= m_n / 2 + 1) return 0.0;
    return qAtan2(m_imag[bin], m_real[bin]);
}

/**
 * @brief 获取指定频率bin对应的物理频率
 *
 * 频率分辨率 = sampleRate / fftSize。
 * bin 0 = DC(0Hz), bin N/2 = Nyquist(sampleRate/2 Hz)。
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
 *
 * 清空所有频率bin的累积状态。建议在长时间运行后调用，
 * 以防止递推过程中的数值漂移累积。
 * 统计信息(totalUpdates等)不会被清除。
 */
void SlidingDFT2::reset()
{
    m_real.fill(0.0);
    m_imag.fill(0.0);
}

/**
 * @brief 重置所有累积统计信息
 *
 * 清零统计计数器(总更新次数、总采样数、平均耗时)。
 * 不影响共振器组的频谱状态。
 */
void SlidingDFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

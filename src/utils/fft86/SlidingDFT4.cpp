#include "SlidingDFT4.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @class SlidingDFT4
 * @brief 滑动DFT处理器实现
 *
 * 滑动DFT(Sliding DFT)是一种递推式频谱计算方法。
 * 每输入一个新采样时，通过简单的复数乘法和加法更新所有频率bin，
 * 避免了完整FFT的重计算。复杂度O(N)/sample，适合实时频谱显示。
 *
 * 递推公式: X_k[n] = (X_k[n-1] - x[n-N]) * e^(j2πk/N) + x[n]
 * 其中N为窗口长度，k为频率bin索引。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SlidingDFT4::SlidingDFT4(QObject* parent)
    : QObject(parent)
    , m_binCount(0)
{
}

/**
 * @brief 初始化滑动DFT
 *
 * 分配频率bin的复数状态缓冲区和环形采样缓冲区。
 * 每个bin对应一个频率: f_k = k * sampleRate / binCount
 *
 * @param binCount 频率bin数量(对应FFT的点数)
 * @param sampleRate 采样率(Hz)，用于计算实际频率
 */
void SlidingDFT4::init(int binCount, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(sampleRate)

    m_binCount = qMax(1, binCount);

    /* 初始化复数状态(实部和虚部) */
    m_realState.resize(m_binCount, 0.0);
    m_imagState.resize(m_binCount, 0.0);

    /* 预计算旋转因子系数 */
    m_cosCoeff.resize(m_binCount);
    m_sinCoeff.resize(m_binCount);
    for (int k = 0; k < m_binCount; ++k) {
        double angle = 2.0 * M_PI * k / m_binCount;
        m_cosCoeff[k] = qCos(angle);
        m_sinCoeff[k] = qSin(angle);
    }

    /* 环形采样缓冲区 */
    m_ringBuffer.resize(m_binCount, 0.0);
    m_ringIndex = 0;

    m_timeSum += timer.elapsed();
}

/**
 * @brief 输入单个采样值，更新所有频率bin
 *
 * 使用滑动DFT递推公式，每输入一个采样通过复数乘法更新所有bin。
 * 时间复杂度O(binCount)，远优于每采样执行一次完整FFT。
 *
 * @param sample 输入采样值
 */
void SlidingDFT4::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    if (m_binCount <= 0) {
        return;
    }

    /* 取出最旧的采样 */
    double oldest = m_ringBuffer[m_ringIndex];

    /* 更新环形缓冲区 */
    m_ringBuffer[m_ringIndex] = sample;
    m_ringIndex = (m_ringIndex + 1) % m_binCount;

    /* 更新每个频率bin */
    double dominantMag = 0.0;
    int dominantIdx = 0;

    for (int k = 0; k < m_binCount; ++k) {
        /* 递推: X_k = (X_k - x_old) * e^(j*angle) + x_new */
        double realPrev = m_realState[k] - oldest;
        double imagPrev = m_imagState[k];

        /* 复数乘以旋转因子 */
        m_realState[k] = realPrev * m_cosCoeff[k] - imagPrev * m_sinCoeff[k] + sample;
        m_imagState[k] = realPrev * m_sinCoeff[k] + imagPrev * m_cosCoeff[k];

        /* 追踪主导频率 */
        double mag = m_realState[k] * m_realState[k] + m_imagState[k] * m_imagState[k];
        if (mag > dominantMag) {
            dominantMag = mag;
            dominantIdx = k;
        }
    }

    m_stats.totalSamplesProcessed++;
    m_stats.totalBinsUpdated += m_binCount;

    double dominantFreq = static_cast<double>(dominantIdx);
    emit spectrumUpdated(m_binCount, dominantFreq);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalSamplesProcessed);
}

/**
 * @brief 获取当前所有频率bin的幅值
 *
 * 计算每个频率bin的幅值 |X_k| = sqrt(Re^2 + Im^2)。
 *
 * @return 各bin的幅值向量
 */
QVector<double> SlidingDFT4::magnitudes() const
{
    QVector<double> mags;
    mags.resize(m_binCount);
    for (int k = 0; k < m_binCount; ++k) {
        double re = m_realState[k];
        double im = m_imagState[k];
        mags[k] = qSqrt(re * re + im * im);
    }
    return mags;
}

/**
 * @brief 重置所有统计数据
 *
 * 将采样计数、bin更新计数和计时归零，不影响当前频谱状态。
 */
void SlidingDFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

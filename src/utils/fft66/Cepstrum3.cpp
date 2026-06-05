/**
 * @file Cepstrum3.cpp
 * @brief 倒频谱分析实现（第3版）
 *
 * 实现倒频谱（Cepstrum）分析，用于基频检测和语音处理。
 * 倒频谱 = IFFT(log(|FFT(signal)|^2))，其峰值位置
 * 对应信号的基频周期。同时计算音质（peakiness）指标。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft66/Cepstrum3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化倒频谱分析器
 * @param parent 父QObject对象指针
 */
Cepstrum3::Cepstrum3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置FFT大小
 * @param n FFT窗口大小
 */
void Cepstrum3::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void Cepstrum3::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 计算信号的倒频谱
 *
 * 算法流程：
 * 1. 对输入帧计算FFT得到频谱
 * 2. 取幅度谱的对数
 * 3. 对对数幅度谱做IFFT得到倒频谱
 * 4. 在有效范围内搜索峰值以确定基频
 *
 * @param frame 输入的时域信号帧
 * @return 倒频谱实部（quefrency域）
 */
QVector<double> Cepstrum3::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    m_fundFreq = 0.0;
    m_quality = 0.0;
    m_cepstrum.clear();

    if (frame.isEmpty()) {
        emit computed(0.0, 0.0);
        return m_cepstrum;
    }

    int N = qMin(frame.size(), m_fftSize);

    /* 步骤1：计算FFT幅度谱 */
    QVector<double> magnitude(N / 2 + 1, 0.0);
    for (int k = 0; k <= N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im -= frame[n] * qSin(angle);
        }
        magnitude[k] = re * re + im * im; /* 功率谱 */
    }

    /* 步骤2：取对数 */
    QVector<double> logSpec(N / 2 + 1);
    for (int k = 0; k <= N / 2; ++k) {
        logSpec[k] = qLn(qMax(1e-10, magnitude[k]));
    }

    /* 步骤3：对对数功率谱做DCT（类似IFFT的实数版本） */
    int cepLen = N / 2 + 1;
    m_cepstrum.resize(cepLen);
    for (int q = 0; q < cepLen; ++q) {
        double val = 0.0;
        for (int k = 0; k <= N / 2; ++k) {
            val += logSpec[k] * qCos(M_PI * q * k / (N / 2));
        }
        m_cepstrum[q] = val;
    }

    /* 步骤4：搜索基频峰值 */
    /* 基频搜索范围：50Hz~1000Hz对应的quefrency范围 */
    int minQ = static_cast<int>(m_sampleRate / 1000.0);
    int maxQ = static_cast<int>(m_sampleRate / 50.0);
    minQ = qMax(1, minQ);
    maxQ = qMin(cepLen - 1, maxQ);

    double peakVal = -std::numeric_limits<double>::max();
    int peakQ = 0;

    for (int q = minQ; q <= maxQ; ++q) {
        if (m_cepstrum[q] > peakVal) {
            peakVal = m_cepstrum[q];
            peakQ = q;
        }
    }

    /* 计算基频 */
    if (peakQ > 0) {
        m_fundFreq = m_sampleRate / peakQ;
    }

    /* 计算音质指标：峰值与平均值之比 */
    double meanCep = 0.0;
    for (int q = minQ; q <= maxQ; ++q) {
        meanCep += qAbs(m_cepstrum[q]);
    }
    meanCep /= (maxQ - minQ + 1);

    if (meanCep > 1e-10) {
        m_quality = peakVal / meanCep;
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_fundFreq, m_quality);
    return m_cepstrum;
}

/**
 * @brief 获取当前统计信息
 * @return 计算统计结构
 */
Cepstrum3::Stats Cepstrum3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Cepstrum3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

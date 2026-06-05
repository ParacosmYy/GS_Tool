/**
 * @file SpectralGate.cpp
 * @brief 频谱门实现 — 频域噪声抑制+谱减法增强
 *
 * 基于STFT的频谱门控降噪处理器。支持噪声轮廓学习和
 * 自适应增益计算，结合谱减法进行噪声抑制。
 */

#include "utils/dsp45/SpectralGate.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数并构建窗函数
 * @param parent 父QObject
 */
SpectralGate::SpectralGate(QObject* parent)
    : QObject(parent)
{
    buildWindow();
}

/**
 * @brief 设置采样率
 * @param sampleRate 目标采样率(Hz)
 */
void SpectralGate::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置FFT大小
 * @param fftSize FFT窗口大小，应为2的幂次
 */
void SpectralGate::setFFTSize(int fftSize)
{
    m_fftSize = qMax(64, fftSize);
    m_hopSize = m_fftSize / 4;
    buildWindow();
    m_overlapBuffer.clear();
    m_noiseProfile.clear();
    m_profileLearned = false;
}

/**
 * @brief 设置噪声门阈值(dB)
 * @param thresholdDb 门控阈值(dB)，低于此值的频谱分量将被抑制
 */
void SpectralGate::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置噪声抑制量(dB)
 * @param reductionDb 抑制量(dB)，控制噪声频谱的衰减程度
 */
void SpectralGate::setReduction(double reductionDb)
{
    m_reduction = reductionDb;
}

/**
 * @brief 从噪声片段学习噪声频谱轮廓
 * @param noise 纯噪声信号片段
 *
 * 对噪声信号进行STFT分析，取各频率bin的均值作为噪声估计。
 */
void SpectralGate::learnNoiseProfile(const QVector<double>& noise)
{
    if (noise.size() < m_fftSize) return;

    const int halfN = m_fftSize / 2 + 1;
    m_noiseProfile.resize(halfN);
    m_noiseProfile.fill(0.0);

    int frameCount = 0;
    for (int start = 0; start + m_fftSize <= noise.size(); start += m_hopSize) {
        QVector<double> real(m_fftSize, 0.0);
        QVector<double> imag(m_fftSize, 0.0);

        /* 加窗 */
        for (int i = 0; i < m_fftSize; ++i) {
            real[i] = noise[start + i] * m_window[i];
        }

        fft(real, imag);

        /* 累积功率谱 */
        for (int i = 0; i < halfN; ++i) {
            double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
            m_noiseProfile[i] += mag;
        }
        frameCount++;
    }

    if (frameCount > 0) {
        for (int i = 0; i < halfN; ++i) {
            m_noiseProfile[i] /= frameCount;
        }
    }

    /* 计算噪声底噪 */
    double sumPower = 0.0;
    for (int i = 0; i < halfN; ++i) {
        sumPower += m_noiseProfile[i] * m_noiseProfile[i];
    }
    m_noiseFloor = 10.0 * std::log10(sumPower / halfN + 1e-12);
    m_profileLearned = true;
}

/**
 * @brief 处理输入信号，进行频谱门控降噪
 * @param input 输入时域信号
 * @return 降噪后的时域信号
 *
 * 流程: STFT -> 谱减法 -> 增益门控 -> IFFT -> 重叠相加
 */
QVector<double> SpectralGate::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_profileLearned || input.isEmpty()) {
        return input;
    }

    const int halfN = m_fftSize / 2 + 1;
    const double reductionLinear = qPow(10.0, m_reduction / 20.0);
    const double thresholdLinear = qPow(10.0, m_threshold / 20.0);

    /* 初始化输出和重叠缓冲 */
    QVector<double> output(input.size(), 0.0);
    QVector<double> overlapSum(input.size(), 0.0);

    int framesProcessed = 0;

    for (int start = 0; start + m_fftSize <= input.size(); start += m_hopSize) {
        QVector<double> real(m_fftSize, 0.0);
        QVector<double> imag(m_fftSize, 0.0);

        /* 加窗 */
        for (int i = 0; i < m_fftSize; ++i) {
            real[i] = input[start + i] * m_window[i];
        }

        fft(real, imag);

        /* ---- 频谱门控 + 谱减法 ---- */
        for (int i = 0; i < halfN; ++i) {
            double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
            double phase = qAtan2(imag[i], real[i]);

            /* 谱减法: 减去噪声估计 */
            double cleanedMag = mag - m_noiseProfile[i];
            cleanedMag = qMax(0.0, cleanedMag);

            /* 信噪比计算 */
            double snr = (m_noiseProfile[i] > 1e-12)
                             ? (cleanedMag / m_noiseProfile[i])
                             : 1e6;

            /* 门控增益 */
            double gain = 1.0;
            if (snr < thresholdLinear) {
                /* 低于阈值: 应用抑制 */
                gain = reductionLinear * (snr / thresholdLinear);
            }

            /* 应用增益 */
            double finalMag = cleanedMag * gain;
            real[i] = finalMag * qCos(phase);
            imag[i] = finalMag * qSin(phase);

            /* 镜像对称 */
            if (i > 0 && i < halfN - 1) {
                int mirror = m_fftSize - i;
                real[mirror] = real[i];
                imag[mirror] = -imag[i];
            }
        }

        /* IFFT */
        ifft(real, imag);

        /* 重叠相加 */
        for (int i = 0; i < m_fftSize && start + i < output.size(); ++i) {
            output[start + i] += real[i] * m_window[i];
            overlapSum[start + i] += m_window[i] * m_window[i];
        }

        framesProcessed++;
    }

    /* 归一化重叠相加 */
    for (int i = 0; i < output.size(); ++i) {
        if (overlapSum[i] > 1e-8) {
            output[i] /= overlapSum[i];
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += input.size();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    double noiseReduction = m_noiseFloor;
    emit processingCompleted(input.size(), noiseReduction);
    return output;
}

/**
 * @brief 构建Hann窗函数
 */
void SpectralGate::buildWindow()
{
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i) {
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
    }
}

/**
 * @brief 原地FFT (Cooley-Tukey基2)
 * @param real 实部数组
 * @param imag 虚部数组
 */
void SpectralGate::fft(QVector<double>& real, QVector<double>& imag) const
{
    const int n = real.size();

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wReal = qCos(angle);
        double wImag = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];

                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;

                double newCurReal = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
            }
        }
    }
}

/**
 * @brief 原地IFFT
 * @param real 实部数组
 * @param imag 虚部数组
 */
void SpectralGate::ifft(QVector<double>& real, QVector<double>& imag) const
{
    const int n = real.size();

    /* 共轭 */
    for (int i = 0; i < n; ++i) {
        imag[i] = -imag[i];
    }

    fft(real, imag);

    /* 缩放 */
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
}

/**
 * @brief 重置所有统计信息
 */
void SpectralGate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

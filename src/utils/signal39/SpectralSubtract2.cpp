/**
 * @file SpectralSubtract2.cpp
 * @brief 谱减法降噪实现 - 维纳滤波与过减法增强
 *
 * 在频域中估计噪声功率谱，通过过减法和维纳滤波
 * 对含噪信号的频谱进行降噪处理，保留语音可懂度。
 */

#include "utils/signal39/SpectralSubtract2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 *
 * 默认参数: 采样率44100Hz, FFT大小2048,
 * 过减法系数2.0, 频谱下限因子0.01
 */
SpectralSubtract2::SpectralSubtract2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void SpectralSubtract2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置FFT大小
 * @param size FFT长度，建议为2的幂
 */
void SpectralSubtract2::setFFTSize(int size)
{
    m_fftSize = qMax(16, size);
}

/**
 * @brief 设置过减法系数
 * @param alpha 过减法因子(通常1.0~3.0)，越大降噪越激进
 */
void SpectralSubtract2::setOversubtraction(double alpha)
{
    m_alpha = qMax(0.0, alpha);
}

/**
 * @brief 设置频谱下限因子
 * @param beta 下限因子(通常0.01~0.1)，防止过度衰减产生音乐噪声
 */
void SpectralSubtract2::setFloor(double beta)
{
    m_beta = qMax(0.0, qMin(beta, 1.0));
}

/**
 * @brief 从纯噪声段估计噪声功率谱
 *
 * 对噪声信号做FFT并计算平均功率谱，作为降噪的参考基线。
 *
 * @param noise 纯噪声信号采样
 */
void SpectralSubtract2::estimateNoise(const QVector<double>& noise)
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_fftSize;
    const int halfN = N / 2 + 1;
    m_noiseSpectrum.resize(halfN, 0.0);

    if (noise.size() < N) {
        /* 信号太短，直接计算 */
        int len = qMin(noise.size(), halfN);
        for (int k = 0; k < len; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < noise.size(); ++n) {
                double angle = -2.0 * M_PI * k * n / noise.size();
                re += noise[n] * qCos(angle);
                im += noise[n] * qSin(angle);
            }
            m_noiseSpectrum[k] = (re * re + im * im) / (noise.size() * noise.size());
        }
        return;
    }

    /* 分帧计算噪声功率谱 */
    int numFrames = noise.size() / N;
    QVector<double> avgSpectrum(halfN, 0.0);

    /* 汉宁窗 */
    QVector<double> window(N, 0.0);
    for (int i = 0; i < N; ++i)
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));

    double winSum = 0.0;
    for (int i = 0; i < N; ++i) winSum += window[i] * window[i];

    for (int f = 0; f < numFrames; ++f) {
        int offset = f * N;

        for (int k = 0; k < halfN; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < N; ++n) {
                double sample = noise[offset + n] * window[n];
                double angle = -2.0 * M_PI * k * n / N;
                re += sample * qCos(angle);
                im += sample * qSin(angle);
            }
            avgSpectrum[k] += (re * re + im * im) / (winSum * winSum);
        }
    }

    /* 平均 */
    for (int k = 0; k < halfN; ++k)
        m_noiseSpectrum[k] = avgSpectrum[k] / numFrames;

    Q_UNUSED(timer);
}

/**
 * @brief 对含噪信号执行谱减法降噪
 *
 * 处理流程:
 * 1. 对输入信号分帧并加窗
 * 2. 计算每帧的FFT
 * 3. 谱减法: |Y|^2 = max(|X|^2 - alpha*|N|^2, beta*|X|^2)
 * 4. 维纳滤波: G = max(1 - alpha*|N|^2/|X|^2, beta)
 * 5. 逆FFT重建时域信号
 * 6. 重叠相加法(overlap-add)合成输出
 *
 * @param input 含噪输入信号
 * @return 降噪后的信号
 */
QVector<double> SpectralSubtract2::reduce(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_fftSize;
    const int halfN = N / 2 + 1;
    const int hopSize = N / 2; /* 50%重叠 */

    if (input.size() < N || m_noiseSpectrum.size() < halfN) {
        m_stats.totalReductions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReductions;
        return input;
    }

    /* 汉宁窗 */
    QVector<double> window(N, 0.0);
    for (int i = 0; i < N; ++i)
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));

    double winSumSq = 0.0;
    for (int i = 0; i < N; ++i) winSumSq += window[i] * window[i];

    /* 输出缓冲区 */
    int outLen = input.size();
    QVector<double> output(outLen, 0.0);
    QVector<double> overlap(outLen, 0.0);

    int numFrames = (input.size() - N) / hopSize + 1;
    double totalSNR = 0.0;
    int validFrames = 0;

    for (int f = 0; f < numFrames; ++f) {
        int offset = f * hopSize;

        /* 加窗并计算FFT */
        QVector<double> re(halfN, 0.0);
        QVector<double> im(halfN, 0.0);

        for (int k = 0; k < halfN; ++k) {
            double reVal = 0.0, imVal = 0.0;
            for (int n = 0; n < N; ++n) {
                double sample = input[offset + n] * window[n];
                double angle = -2.0 * M_PI * k * n / N;
                reVal += sample * qCos(angle);
                imVal += sample * qSin(angle);
            }
            re[k] = reVal;
            im[k] = imVal;
        }

        /* 计算功率谱并应用谱减法 */
        QVector<double> mag(halfN, 0.0);
        double frameSignalPower = 0.0;
        double frameNoisePower = 0.0;

        for (int k = 0; k < halfN; ++k) {
            double power = re[k] * re[k] + im[k] * im[k];
            double noisePower = m_noiseSpectrum[k] * winSumSq * winSumSq;

            frameSignalPower += power;
            frameNoisePower += noisePower;

            /* 谱减法: Y = max(X - alpha*N, beta*X) */
            double subtracted = power - m_alpha * noisePower;
            double floor = m_beta * power;
            double cleanPower = qMax(subtracted, floor);

            /* 维纳滤波增益 */
            double wienerGain = qMax(1.0 - m_alpha * noisePower / qMax(power, 1e-30), m_beta);
            wienerGain = qMax(wienerGain, 0.0);

            /* 综合增益: 取谱减法和维纳滤波的较小增益 */
            double gain = qSqrt(cleanPower / qMax(power, 1e-30));
            gain = qMin(gain, wienerGain);

            /* 应用增益 */
            re[k] *= gain;
            im[k] *= gain;
        }

        /* 逆FFT重建时域帧 */
        QVector<double> frame(N, 0.0);
        for (int n = 0; n < N; ++n) {
            double sample = 0.0;
            for (int k = 0; k < halfN; ++k) {
                double angle = 2.0 * M_PI * k * n / N;
                sample += re[k] * qCos(angle) - im[k] * qSin(angle);
            }
            frame[n] = sample * window[n] / N;
        }

        /* 重叠相加 */
        for (int n = 0; n < N && (offset + n) < outLen; ++n)
            output[offset + n] += frame[n];

        /* 计算帧SNR */
        if (frameNoisePower > 1e-30) {
            totalSNR += 10.0 * qLn(frameSignalPower / frameNoisePower) / qLn(10.0);
            validFrames++;
        }
    }

    /* 归一化重叠相加 */
    for (int i = 0; i < outLen; ++i)
        output[i] /= 2.0;

    double avgSNR = (validFrames > 0) ? totalSNR / validFrames : 0.0;

    m_stats.totalReductions++;
    m_stats.totalFramesProcessed += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReductions;

    emit reductionComplete(input.size(), avgSNR);
    return output;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralSubtract2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

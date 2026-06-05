/**
 * @file SpectralSubtract4.cpp
 * @brief 谱减法降噪实现
 *
 * 实现基于频谱减法的单通道语音降噪，支持过减因子
 * 和频谱下限控制。通过从带噪信号频谱中减去噪声估计
 * 实现降噪。
 */

#include "utils/signal74/SpectralSubtract4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SpectralSubtract4::SpectralSubtract4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置FFT大小
 * @param n FFT点数
 */
void SpectralSubtract4::setFFTSize(int n)
{
    m_fftSize = qBound(64, n, 8192);
}

/**
 * @brief 设置过减因子
 * @param factor 过减因子，>1增强降噪力度
 */
void SpectralSubtract4::setOverSubtraction(double factor)
{
    m_overSub = qBound(0.5, factor, 10.0);
}

/**
 * @brief 设置频谱下限
 * @param floor 频谱下限，防止过度减谱导致音乐噪声
 */
void SpectralSubtract4::setSpectralFloor(double floor)
{
    m_floor = qBound(0.0, floor, 1.0);
}

/**
 * @brief 设置噪声频谱估计
 * @param noise 噪声功率谱估计
 */
void SpectralSubtract4::setNoiseEstimate(const QVector<double>& noise)
{
    m_noise = noise;
}

/**
 * @brief 对信号进行谱减法降噪
 * @param signal 输入带噪信号
 * @return 降噪后的信号
 */
QVector<double> SpectralSubtract4::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return QVector<double>();

    const int N = signal.size();
    const int fftSize = m_fftSize;
    const int hopSize = fftSize / 2;
    const int numBins = fftSize / 2 + 1;

    // 如果没有噪声估计，使用信号前几帧估计
    if (m_noise.size() != numBins) {
        m_noise.resize(numBins, 0.0);
        int noiseFrames = qMin(5, N / fftSize);
        if (noiseFrames > 0) {
            for (int f = 0; f < noiseFrames; ++f) {
                for (int k = 0; k < numBins; ++k) {
                    double re = 0.0, im = 0.0;
                    for (int n = 0; n < fftSize; ++n) {
                        int idx = f * fftSize + n;
                        if (idx < N) {
                            double angle = 2.0 * M_PI * k * n / fftSize;
                            re += signal[idx] * qCos(angle);
                            im -= signal[idx] * qSin(angle);
                        }
                    }
                    m_noise[k] += (re * re + im * im) / fftSize;
                }
            }
            for (double& v : m_noise) v /= noiseFrames;
        }
    }

    // 分帧处理
    int numFrames = qMax(1, (N - fftSize) / hopSize + 1);
    QVector<double> output(N, 0.0);
    QVector<double> windowSum(N, 0.0);

    // Hann窗
    QVector<double> window(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (fftSize - 1)));
    }

    double totalSNR = 0.0;
    int validFrames = 0;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;

        // FFT分析
        QVector<double> re(numBins, 0.0), im(numBins, 0.0);
        for (int k = 0; k < numBins; ++k) {
            for (int n = 0; n < fftSize; ++n) {
                int idx = start + n;
                if (idx < N) {
                    double sample = signal[idx] * window[n];
                    double angle = 2.0 * M_PI * k * n / fftSize;
                    re[k] += sample * qCos(angle);
                    im[k] -= sample * qSin(angle);
                }
            }
        }

        // 功率谱
        QVector<double> power(numBins);
        for (int k = 0; k < numBins; ++k) {
            power[k] = (re[k] * re[k] + im[k] * im[k]) / fftSize;
        }

        // 谱减法：减去过减因子*噪声功率
        QVector<double> cleanPower(numBins);
        for (int k = 0; k < numBins; ++k) {
            cleanPower[k] = power[k] - m_overSub * m_noise[k];
            // 频谱下限
            double floorLevel = m_floor * power[k];
            cleanPower[k] = qMax(cleanPower[k], floorLevel);
        }

        // 计算SNR
        double sigPow = 0.0, noisePow = 0.0;
        for (int k = 0; k < numBins; ++k) {
            sigPow += power[k];
            noisePow += m_noise[k];
        }
        if (noisePow > 1e-10) {
            totalSNR += 10.0 * qLn(sigPow / noisePow) / qLn(10.0);
            validFrames++;
        }

        // 幅度重建
        for (int k = 0; k < numBins; ++k) {
            double mag = qSqrt(qMax(cleanPower[k], 0.0) * fftSize);
            double phase = qAtan2(im[k], (qAbs(re[k]) > 1e-15) ? re[k] : 1e-15);
            re[k] = mag * qCos(phase);
            im[k] = mag * qSin(phase);
        }

        // IFFT重建（对称扩展）
        for (int n = 0; n < fftSize; ++n) {
            double sample = 0.0;
            for (int k = 0; k < numBins; ++k) {
                double angle = 2.0 * M_PI * k * n / fftSize;
                sample += re[k] * qCos(angle) - im[k] * qSin(angle);
            }
            sample /= fftSize;

            int idx = start + n;
            if (idx < N) {
                output[idx] += sample * window[n];
                windowSum[idx] += window[n] * window[n];
            }
        }
    }

    // 重叠相加归一化
    for (int i = 0; i < N; ++i) {
        if (windowSum[i] > 1e-10) {
            output[i] /= windowSum[i];
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDenoisings++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoisings;

    double avgSNR = (validFrames > 0) ? totalSNR / validFrames : 0.0;
    emit denoisingCompleted(numFrames, avgSNR);
    return output;
}

/**
 * @brief 重置统计信息
 */
void SpectralSubtract4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

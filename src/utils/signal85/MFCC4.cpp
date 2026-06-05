#include "MFCC4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化MFCC特征提取器
 * @param parent 父QObject对象指针
 */
MFCC4::MFCC4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 提取单帧音频的MFCC系数
 *
 * MFCC提取流程：
 * 1. 预加重（高频增强）
 * 2. 加汉宁窗
 * 3. FFT计算功率谱
 * 4. Mel滤波器组滤波并取对数
 * 5. DCT变换得到倒谱系数
 *
 * @param frame 单帧音频采样
 * @param sampleRate 采样率(Hz)
 * @param numCoeffs 输出系数个数，默认13
 * @return MFCC系数向量
 */
QVector<double> MFCC4::extract(const QVector<double>& frame, double sampleRate, int numCoeffs)
{
    QElapsedTimer timer;
    timer.start();

    const int N = frame.size();
    if (N == 0) return {};

    numCoeffs = qBound(1, numCoeffs, N / 2);
    m_numCoeffs = numCoeffs;

    /// 步骤1：预加重 y[n] = x[n] - 0.97*x[n-1]
    QVector<double> preemph(N);
    preemph[0] = frame[0];
    for (int i = 1; i < N; ++i) {
        preemph[i] = frame[i] - 0.97 * frame[i - 1];
    }

    /// 步骤2：加汉宁窗
    for (int i = 0; i < N; ++i) {
        preemph[i] *= 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
    }

    /// 步骤3：计算功率谱（简化DFT）
    int spectrumSize = N / 2 + 1;
    QVector<double> powerSpectrum(spectrumSize, 0.0);
    for (int k = 0; k < spectrumSize; ++k) {
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            real += preemph[n] * std::cos(angle);
            imag -= preemph[n] * std::sin(angle);
        }
        powerSpectrum[k] = (real * real + imag * imag) / N;
    }

    /// 步骤4：Mel滤波器组
    const int melBands = 26;  ///< 标准Mel滤波器数
    double melMin = 2595.0 * std::log10(1.0 + 0.0 / 700.0);
    double melMax = 2595.0 * std::log10(1.0 + sampleRate / 2.0 / 700.0);

    QVector<double> melEnergies(melBands, 0.0);
    for (int m = 0; m < melBands; ++m) {
        double melCenter = melMin + (melMax - melMin) * (m + 1) / (melBands + 1);
        double hzCenter = 700.0 * (std::pow(10.0, melCenter / 2595.0) - 1.0);
        int binCenter = static_cast<int>(hzCenter / (sampleRate / 2.0) * spectrumSize);
        int halfWidth = qMax(1, spectrumSize / melBands / 2);

        for (int k = qMax(0, binCenter - halfWidth);
             k < qMin(spectrumSize, binCenter + halfWidth); ++k) {
            double dist = 1.0 - std::abs(k - binCenter) / static_cast<double>(halfWidth);
            melEnergies[m] += powerSpectrum[k] * qMax(0.0, dist);
        }

        /// 取对数
        melEnergies[m] = (melEnergies[m] > 1e-10) ? std::log(melEnergies[m]) : -20.0;
    }

    /// 步骤5：DCT-II变换得到MFCC
    QVector<double> mfcc(numCoeffs);
    for (int i = 0; i < numCoeffs; ++i) {
        double sum = 0.0;
        for (int j = 0; j < melBands; ++j) {
            sum += melEnergies[j] * std::cos(M_PI * i * (j + 0.5) / melBands);
        }
        mfcc[i] = sum;
    }

    /// 更新统计信息
    m_stats.totalFramesExtracted++;
    m_stats.totalCoefficients += numCoeffs;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesExtracted;

    emit frameExtracted(m_stats.totalFramesExtracted - 1, numCoeffs);
    return mfcc;
}

/**
 * @brief 批量提取MFCC特征序列
 *
 * 将长音频分帧后逐帧提取MFCC，适用于语音识别前端处理。
 *
 * @param samples 完整音频采样数据
 * @param sampleRate 采样率(Hz)
 * @param frameSize 帧长(采样点数)，典型256或512
 * @param hopSize 帧移(采样点数)，典型为frameSize/2
 * @return MFCC特征矩阵(帧数 x numCoeffs)
 */
QVector<QVector<double>> MFCC4::extractSequence(const QVector<double>& samples,
                                                  double sampleRate, int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    const int numFrames = (n - frameSize) / hopSize + 1;

    if (numFrames <= 0) return {};

    QVector<QVector<double>> features;
    features.reserve(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        QVector<double> frame(frameSize);
        int start = f * hopSize;
        for (int i = 0; i < frameSize; ++i) {
            frame[i] = (start + i < n) ? samples[start + i] : 0.0;
        }
        features.append(extract(frame, sampleRate, m_numCoeffs));
    }

    return features;
}

/**
 * @brief 获取当前统计数据
 * @return 包含提取帧数、系数总数和平均耗时的Stats结构
 */
MFCC4::Stats MFCC4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void MFCC4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

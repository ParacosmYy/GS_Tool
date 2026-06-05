#include "Tonality3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化音调分析器
 * @param parent 父对象指针
 */
Tonality3::Tonality3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析帧大小
 * @param size 帧大小(采样点数)
 */
void Tonality3::setFrameSize(int size)
{
    m_frameSize = qMax(2, size);
}

/**
 * @brief 对输入信号帧计算音调指标
 *
 * 通过频谱峰值与背景噪声的比值评估音调强度，
 * 值接近1.0表示强音调信号，接近0.0表示类噪声信号。
 *
 * @param frame 输入信号帧
 * @return 音调指标值(0.0~1.0)
 */
double Tonality3::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.size() < 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0.0);
        return 0.0;
    }

    const int N = frame.size();
    int halfN = N / 2;

    /* 计算功率谱 */
    QVector<double> powerSpec(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * std::cos(angle);
            im += frame[n] * std::sin(angle);
        }
        powerSpec[k] = (re * re + im * im) / (N * N);
    }

    /* 计算频谱平均值作为背景 */
    double meanPower = 0.0;
    for (double v : powerSpec) meanPower += v;
    meanPower /= powerSpec.size();

    /* 寻找频谱峰值 */
    double peakPower = 0.0;
    for (int k = 1; k < halfN - 1; ++k) {
        if (powerSpec[k] > powerSpec[k - 1] && powerSpec[k] > powerSpec[k + 1]) {
            peakPower = qMax(peakPower, powerSpec[k]);
        }
    }

    /* 音调指标 = 峰值与背景的比值归一化 */
    double tonality = 0.0;
    if (meanPower > 1e-30) {
        double snr = 10.0 * std::log10(qMax(1e-30, peakPower) / meanPower);
        /* 映射SNR到[0,1]: 典型范围 -10dB ~ +30dB */
        tonality = qBound(0.0, (snr + 10.0) / 40.0, 1.0);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
    emit computed(tonality);
    return tonality;
}

/**
 * @brief 计算信号的频谱质心
 *
 * 频谱质心是频谱幅度的加权平均频率，
 * 反映信号频谱能量的集中位置。
 *
 * @param frame 输入信号帧
 * @return 频谱质心(归一化频率0~1)
 */
double Tonality3::spectralCentroid(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;

    const int N = frame.size();
    int halfN = N / 2;
    double weightedSum = 0.0;
    double totalPower = 0.0;

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * std::cos(angle);
            im += frame[n] * std::sin(angle);
        }
        double mag = std::sqrt(re * re + im * im);
        weightedSum += k * mag;
        totalPower += mag;
    }

    return (totalPower > 1e-10) ? weightedSum / (totalPower * halfN) : 0.0;
}

/**
 * @brief 重置统计数据
 */
void Tonality3::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

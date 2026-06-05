/**
 * @file ShortTimeFFT4.cpp
 * @brief 短时傅里叶变换实现 — STFT正向变换 + 逆变换 + 窗函数生成
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 STFT（Short-Time Fourier Transform）正向和逆变换。
 * 支持 Hann、Hamming、Blackman、Rectangular 四种窗函数。
 * 正变换将时域信号分帧加窗后进行 DFT；
 * 逆变换通过重叠相加法（overlap-add）重建时域信号。
 */

#include "utils/fft56/ShortTimeFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 STFT 参数
 *
 * 默认窗口大小 1024，跳步大小 512（50% 重叠），Hann 窗。
 *
 * @param parent 父QObject对象
 */
ShortTimeFFT4::ShortTimeFFT4(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ShortTimeFFT4"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置分析窗口大小
 *
 * 窗口越大，频率分辨率越高，时间分辨率越低。
 * 自动调整跳步大小为窗口的一半（50% 重叠）。
 *
 * @param n 窗口大小（必须为 2 的幂次）
 */
void ShortTimeFFT4::setWindowSize(int n)
{
    m_winSize = qMax(4, n);
    // 确保为 2 的幂次
    int power = 1;
    while (power < m_winSize) power <<= 1;
    m_winSize = power;
    m_hopSize = m_winSize / 2;
}

/**
 * @brief 设置跳步大小
 *
 * 跳步决定帧之间的重叠量。典型值为窗口大小的 1/2 或 1/4。
 *
 * @param hop 跳步采样数
 */
void ShortTimeFFT4::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置窗函数类型
 *
 * 支持 "hann"（默认）、"hamming"、"blackman"、"rectangular"。
 *
 * @param type 窗函数类型名称
 */
void ShortTimeFFT4::setWindowType(const QString& type)
{
    m_winType = type.toLower();
}

// ──────────────────────────────────────────────
// 正向 STFT
// ──────────────────────────────────────────────

/**
 * @brief 对时域信号执行正向短时傅里叶变换
 *
 * 将输入信号分帧、加窗后逐帧进行 DFT。
 * 输出为复数频谱的幅度谱，每行对应一帧的频谱。
 *
 * @param signal 输入时域信号
 * @return 二维数组，每行为一帧的频谱幅度（长度 = windowSize/2 + 1）
 */
QVector<QVector<double>> ShortTimeFFT4::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    if (n == 0) {
        return {};
    }

    // 生成窗函数
    QVector<double> window = generateWindow(m_winSize, m_winType);

    // 计算帧数
    const int numFrames = (n - m_winSize) / m_hopSize + 1;
    if (numFrames <= 0) {
        return {};
    }

    const int numBins = m_winSize / 2 + 1;
    QVector<QVector<double>> result(numFrames, QVector<double>(numBins, 0.0));

    // 逐帧处理
    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * m_hopSize;

        // 加窗 + DFT
        for (int k = 0; k < numBins; ++k) {
            double re = 0.0;
            double im = 0.0;
            for (int i = 0; i < m_winSize; ++i) {
                int idx = start + i;
                double sample = (idx < n) ? signal[idx] * window[i] : 0.0;
                double angle = -2.0 * M_PI * k * i / m_winSize;
                re += sample * qCos(angle);
                im += sample * qSin(angle);
            }
            result[frame][k] = qSqrt(re * re + im * im);
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, numBins);
    return result;
}

// ──────────────────────────────────────────────
// 逆 STFT
// ──────────────────────────────────────────────

/**
 * @brief 对 STFT 频谱执行逆变换，重建时域信号
 *
 * 使用重叠相加法（overlap-add）将各帧逆 DFT 结果拼接。
 * 假设输入频谱为幅度谱，逆变换时忽略相位信息（使用替代相位）。
 *
 * @param stft STFT 频谱数据，每行为一帧的幅度谱
 * @return 重建的时域信号
 */
QVector<double> ShortTimeFFT4::inverse(const QVector<QVector<double>>& stft)
{
    QElapsedTimer timer;
    timer.start();

    const int numFrames = stft.size();
    if (numFrames == 0) {
        return {};
    }

    const int numBins = stft[0].size();
    const int winSize = (numBins - 1) * 2;
    const int outputLen = (numFrames - 1) * m_hopSize + winSize;

    QVector<double> output(outputLen, 0.0);
    QVector<double> window = generateWindow(winSize, m_winType);

    // 计算窗函数平方和用于归一化
    QVector<double> winSum(outputLen, 0.0);

    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * m_hopSize;

        // 逆 DFT（忽略相位，仅用幅度）
        for (int i = 0; i < winSize; ++i) {
            double sample = 0.0;
            for (int k = 0; k < numBins; ++k) {
                double angle = 2.0 * M_PI * k * i / winSize;
                sample += stft[frame][k] * qCos(angle);
            }
            // 对称部分
            sample /= winSize;

            if (start + i < outputLen) {
                output[start + i] += sample * window[i];
                winSum[start + i] += window[i] * window[i];
            }
        }
    }

    // 归一化（重叠相加）
    for (int i = 0; i < outputLen; ++i) {
        if (winSum[i] > 1e-10) {
            output[i] /= winSum[i];
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, numBins);
    return output;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含变换次数、总帧数和平均耗时的Stats结构
 */
ShortTimeFFT4::Stats ShortTimeFFT4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void ShortTimeFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 窗函数生成
// ──────────────────────────────────────────────

/**
 * @brief 生成指定类型的窗函数
 *
 * 支持的窗函数：
 * - hann:    w(n) = 0.5 * (1 - cos(2*pi*n/(N-1)))
 * - hamming: w(n) = 0.54 - 0.46 * cos(2*pi*n/(N-1))
 * - blackman: w(n) = 0.42 - 0.5*cos(...) + 0.08*cos(...)
 * - rectangular: w(n) = 1.0
 *
 * @param n 窗口长度
 * @param type 窗函数类型名称
 * @return 窗函数系数数组
 */
QVector<double> ShortTimeFFT4::generateWindow(int n, const QString& type)
{
    QVector<double> w(n, 1.0);

    if (type == QStringLiteral("hann")) {
        for (int i = 0; i < n; ++i) {
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        }
    } else if (type == QStringLiteral("hamming")) {
        for (int i = 0; i < n; ++i) {
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1));
        }
    } else if (type == QStringLiteral("blackman")) {
        for (int i = 0; i < n; ++i) {
            w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (n - 1))
                   + 0.08 * qCos(4.0 * M_PI * i / (n - 1));
        }
    }
    // rectangular: 默认全1

    return w;
}

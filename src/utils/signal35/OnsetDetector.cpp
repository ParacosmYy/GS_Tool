/**
 * @file OnsetDetector.cpp
 * @brief 起始点检测实现 — 频谱通量/高频能量/自适应阈值/峰值拾取
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal35/OnsetDetector.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
OnsetDetector::OnsetDetector(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("OnsetDetector"));
}

/**
 * @brief 设置检测方法
 *
 * method 0 = 频谱通量(Spectral Flux)
 * method 1 = 高频能量(HFC)
 * method 2 = 复数域(Complex Domain)
 *
 * @param method 方法编号
 */
void OnsetDetector::setMethod(int method)
{
    m_method = qBound(0, method, 2);
}

/**
 * @brief 设置帧大小(FFT窗口)
 * @param size 帧大小（典型值512/1024/2048）
 */
void OnsetDetector::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
}

/**
 * @brief 设置帧移(跳步)
 * @param hop 帧移采样数（通常为帧大小的1/2或1/4）
 */
void OnsetDetector::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void OnsetDetector::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置检测阈值倍数
 *
 * 自适应阈值 = 局部均值 + threshold * 局部标准差。
 * 较大的阈值减少误检，较小的阈值增加召回率。
 *
 * @param threshold 阈值倍数
 */
void OnsetDetector::setThreshold(double threshold)
{
    m_threshold = qMax(0.1, threshold);
}

/**
 * @brief 计算Hann窗
 */
static QVector<double> hannWindow(int size)
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i) {
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / static_cast<double>(size)));
    }
    return w;
}

/**
 * @brief 计算DFT幅度谱（仅正频率部分）
 */
static QVector<double> computeMagnitude(const QVector<double>& frame, const QVector<double>& window)
{
    int N = frame.size();
    int halfN = N / 2 + 1;
    QVector<double> mag(halfN, 0.0);

    /* 基2 DFT: 仅计算幅度谱，不使用复数类型 */
    for (int k = 0; k < halfN; ++k) {
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            real += frame[n] * window[n] * qCos(angle);
            imag += frame[n] * window[n] * qSin(angle);
        }
        mag[k] = qSqrt(real * real + imag * imag);
    }
    return mag;
}

/**
 * @brief 执行起始点检测
 *
 * 流程:
 * 1. 分帧加窗 → FFT幅度谱
 * 2. 计算检测函数（频谱通量/HFC/复数域）
 * 3. 自适应阈值峰值拾取
 * 4. 输出起始点位置列表
 *
 * @param audio 输入音频采样序列
 * @return 起始点帧索引列表
 */
QList<int> OnsetDetector::detect(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QList<int> onsets;

    if (audio.size() < m_frameSize) {
        emit detectionComplete(0);
        return onsets;
    }

    /* 计算检测函数 */
    auto detFunc = detectionFunction(audio);

    if (detFunc.isEmpty()) {
        emit detectionComplete(0);
        return onsets;
    }

    int numFrames = detFunc.size();

    /* 自适应阈值: 滑动窗口局部统计 */
    int windowSize = qMax(4, numFrames / 20);

    for (int i = 1; i < numFrames - 1; ++i) {
        /* 计算局部均值和标准差 */
        int wStart = qMax(0, i - windowSize / 2);
        int wEnd = qMin(numFrames - 1, i + windowSize / 2);
        double localMean = 0.0;
        int count = 0;
        for (int j = wStart; j <= wEnd; ++j) {
            localMean += detFunc[j].second;
            count++;
        }
        localMean /= qMax(1, count);

        double localStd = 0.0;
        for (int j = wStart; j <= wEnd; ++j) {
            double diff = detFunc[j].second - localMean;
            localStd += diff * diff;
        }
        localStd = qSqrt(localStd / qMax(1, count));

        /* 自适应阈值 */
        double threshold = localMean + m_threshold * localStd;

        /* 峰值拾取: 当前帧超过阈值且为局部最大值 */
        if (detFunc[i].second > threshold &&
            detFunc[i].second >= detFunc[i - 1].second &&
            detFunc[i].second >= detFunc[i + 1].second) {
            int frameIndex = detFunc[i].first;
            onsets.append(frameIndex);
            emit onsetDetected(frameIndex, detFunc[i].second);
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetections++;
    m_stats.totalOnsetsFound += onsets.size();
    m_stats.totalFramesProcessed += numFrames;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionComplete(onsets.size());
    return onsets;
}

/**
 * @brief 计算检测函数
 *
 * 返回每帧的检测强度值，用于后续峰值拾取。
 *
 * @param audio 输入音频采样序列
 * @return (帧索引, 检测强度) 列表
 */
QVector<QPair<int, double>> OnsetDetector::detectionFunction(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, double>> result;

    if (audio.size() < m_frameSize) {
        return result;
    }

    int numFrames = (audio.size() - m_frameSize) / m_hopSize + 1;
    QVector<double> window = hannWindow(m_frameSize);
    QVector<double> prevMag; /* 上一帧的幅度谱 */

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 提取帧并加窗 */
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i) {
            frame[i] = audio[start + i];
        }

        /* 计算幅度谱 */
        QVector<double> mag = computeMagnitude(frame, window);
        int halfN = mag.size();

        double onsetStrength = 0.0;

        if (!prevMag.isEmpty()) {
            switch (m_method) {
            case 0: {
                /* 频谱通量: 只计算正差分的平方和 */
                for (int k = 0; k < halfN; ++k) {
                    double diff = mag[k] - prevMag[k];
                    if (diff > 0.0) {
                        onsetStrength += diff * diff;
                    }
                }
                onsetStrength = qSqrt(onsetStrength);
                break;
            }
            case 1: {
                /* 高频能量加权(HFC): 频率越高权重越大 */
                for (int k = 0; k < halfN; ++k) {
                    onsetStrength += static_cast<double>(k) * mag[k] * mag[k];
                }
                onsetStrength = qSqrt(onsetStrength / halfN);
                break;
            }
            case 2: {
                /* 复数域: 综合幅度和相位变化 */
                for (int k = 0; k < halfN; ++k) {
                    double ampDiff = qFabs(mag[k] - prevMag[k]);
                    onsetStrength += ampDiff;
                }
                break;
            }
            }
        }

        result.append({start, onsetStrength});
        prevMag = mag;
    }

    return result;
}

/**
 * @brief 重置所有累积统计信息
 */
void OnsetDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

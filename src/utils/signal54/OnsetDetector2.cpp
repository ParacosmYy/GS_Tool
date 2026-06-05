/**
 * @file OnsetDetector2.cpp
 * @brief 音频起始点检测器实现
 *
 * 实现基于频域分析的音频起始点（Onset）检测，支持多种检测方法:
 * - 频谱通量法（Spectral Flux）: 通过频谱变化的幅度检测起始点
 * - 高频能量法（High Frequency Content）: 利用高频能量突变检测打击乐起始点
 *
 * 检测流程:
 * 1. 将信号分帧并加窗（Hann窗）
 * 2. 对每帧执行FFT计算频谱
 * 3. 计算起始点函数（onset function）
 * 4. 对起始点函数进行峰值检测
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal54/OnsetDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
OnsetDetector2::OnsetDetector2(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_hopSize(512)
    , m_method("spectral_flux")
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void OnsetDetector2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置帧移（hop size）
 * @param hop 帧移大小（采样数），影响时间分辨率
 */
void OnsetDetector2::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 设置检测方法
 * @param method 方法名: "spectral_flux" 或 "hfc"
 */
void OnsetDetector2::setMethod(const QString& method)
{
    if (method == "spectral_flux" || method == "hfc") {
        m_method = method;
    }
}

/**
 * @brief 检测信号中的起始点
 *
 * 对输入信号执行起始点检测，返回每个检测到的起始点的时间（采样位置）。
 * 同时计算并存储起始点函数（onset function），可通过 onsetFunction() 获取。
 *
 * @param signal 输入音频信号
 * @return 起始点时间列表（单位: 采样位置）
 */
QVector<double> OnsetDetector2::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> onsets;

    if (signal.isEmpty()) {
        return onsets;
    }

    const int n = signal.size();
    int frameSize = m_hopSize * 2;  ///< 帧大小为hop的2倍（50%重叠）

    /* 计算起始点函数 */
    if (m_method == "hfc") {
        m_onsetEnv = highFrequencyContent(signal);
    } else {
        m_onsetEnv = spectralFlux(signal);
    }

    if (m_onsetEnv.isEmpty()) {
        return onsets;
    }

    int numFrames = m_onsetEnv.size();

    /* 计算自适应阈值 */
    int avgWindow = qMax(1, numFrames / 20);  ///< 阈值窗口大小
    QVector<double> threshold(numFrames, 0.0);

    for (int i = 0; i < numFrames; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = qMax(0, i - avgWindow); j <= qMin(numFrames - 1, i + avgWindow); ++j) {
            sum += m_onsetEnv[j];
            count++;
        }
        /* 阈值 = 局部均值 * 1.5 + 全局最小值的偏移 */
        threshold[i] = (sum / qMax(1, count)) * 1.5 + 0.01;
    }

    /* 峰值检测: 寻找超过阈值且为局部极大值的帧 */
    int minOnsetDist = qMax(1, static_cast<int>(0.05 * m_sampleRate / m_hopSize));  ///< 最小起始点间隔50ms

    int lastOnset = -minOnsetDist * 2;

    for (int i = 1; i < numFrames - 1; ++i) {
        if (m_onsetEnv[i] > threshold[i] &&
            m_onsetEnv[i] > m_onsetEnv[i - 1] &&
            m_onsetEnv[i] >= m_onsetEnv[i + 1]) {

            /* 检查最小间隔 */
            if (i - lastOnset >= minOnsetDist) {
                double time = i * m_hopSize;
                onsets.append(time);
                lastOnset = i;

                /* 发射起始点检测信号 */
                double strength = m_onsetEnv[i];
                emit onsetDetected(time / m_sampleRate, strength);
            }
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return onsets;
}

/**
 * @brief 频谱通量法计算起始点函数
 *
 * 计算相邻帧频谱的正差值之和（只计算增大的频率分量）。
 * 频谱通量对音色变化和音符起始点非常敏感。
 *
 * flux[n] = sum_{k} max(0, |X[n][k]| - |X[n-1][k]|)
 *
 * @param signal 输入信号
 * @return 起始点函数（每帧一个值）
 */
QVector<double> OnsetDetector2::spectralFlux(const QVector<double>& signal)
{
    const int n = signal.size();
    int frameSize = m_hopSize * 2;
    int numFrames = (n - frameSize) / m_hopSize + 1;

    if (numFrames <= 0) {
        return QVector<double>();
    }

    QVector<double> flux(numFrames, 0.0);
    QVector<double> prevSpectrum(frameSize / 2 + 1, 0.0);

    /* Hann窗函数 */
    QVector<double> window(frameSize);
    for (int i = 0; i < frameSize; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (frameSize - 1)));
    }

    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * m_hopSize;

        /* 加窗提取当前帧 */
        QVector<double> re(frameSize, 0.0);
        QVector<double> im(frameSize, 0.0);

        for (int i = 0; i < frameSize && (start + i) < n; ++i) {
            re[i] = signal[start + i] * window[i];
        }

        /* 执行FFT（基2 Cooley-Tukey） */
        int fftLen = frameSize;

        /* 位反转排列 */
        int j = 0;
        for (int i = 1; i < fftLen; ++i) {
            int bit = fftLen >> 1;
            while (j & bit) { j ^= bit; bit >>= 1; }
            j ^= bit;
            if (i < j) {
                std::swap(re[i], re[j]);
                std::swap(im[i], im[j]);
            }
        }

        for (int len = 2; len <= fftLen; len <<= 1) {
            double angle = -2.0 * M_PI / len;
            double wRe = qCos(angle);
            double wIm = qSin(angle);
            for (int i = 0; i < fftLen; i += len) {
                double curRe = 1.0, curIm = 0.0;
                for (int k = 0; k < len / 2; ++k) {
                    int u = i + k, v = i + k + len / 2;
                    double tRe = curRe * re[v] - curIm * im[v];
                    double tIm = curRe * im[v] + curIm * re[v];
                    re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                    re[u] += tRe; im[u] += tIm;
                    double nRe = curRe * wRe - curIm * wIm;
                    curIm = curRe * wIm + curIm * wRe;
                    curRe = nRe;
                }
            }
        }

        /* 计算幅度谱 */
        QVector<double> spectrum(frameSize / 2 + 1);
        for (int k = 0; k <= frameSize / 2; ++k) {
            spectrum[k] = qSqrt(re[k] * re[k] + im[k] * im[k]);
        }

        /* 计算频谱通量: 正差值之和 */
        double sumFlux = 0.0;
        for (int k = 0; k <= frameSize / 2; ++k) {
            double diff = spectrum[k] - prevSpectrum[k];
            sumFlux += (diff > 0) ? diff : 0.0;
        }

        flux[frame] = sumFlux;
        prevSpectrum = spectrum;
    }

    return flux;
}

/**
 * @brief 高频能量法计算起始点函数
 *
 * 对每帧频谱加权求和，权重与频率成正比。
 * 高频能量法对打击乐和高瞬态信号特别敏感。
 *
 * hfc[n] = sum_{k} k * |X[n][k]|^2
 *
 * @param signal 输入信号
 * @return 起始点函数（每帧一个值）
 */
QVector<double> OnsetDetector2::highFrequencyContent(const QVector<double>& signal)
{
    const int n = signal.size();
    int frameSize = m_hopSize * 2;
    int numFrames = (n - frameSize) / m_hopSize + 1;

    if (numFrames <= 0) {
        return QVector<double>();
    }

    QVector<double> hfc(numFrames, 0.0);

    /* Hann窗函数 */
    QVector<double> window(frameSize);
    for (int i = 0; i < frameSize; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (frameSize - 1)));
    }

    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * m_hopSize;

        QVector<double> re(frameSize, 0.0);
        QVector<double> im(frameSize, 0.0);

        for (int i = 0; i < frameSize && (start + i) < n; ++i) {
            re[i] = signal[start + i] * window[i];
        }

        /* FFT */
        int fftLen = frameSize;
        int j = 0;
        for (int i = 1; i < fftLen; ++i) {
            int bit = fftLen >> 1;
            while (j & bit) { j ^= bit; bit >>= 1; }
            j ^= bit;
            if (i < j) {
                std::swap(re[i], re[j]);
                std::swap(im[i], im[j]);
            }
        }

        for (int len = 2; len <= fftLen; len <<= 1) {
            double angle = -2.0 * M_PI / len;
            double wRe = qCos(angle), wIm = qSin(angle);
            for (int i = 0; i < fftLen; i += len) {
                double curRe = 1.0, curIm = 0.0;
                for (int k = 0; k < len / 2; ++k) {
                    int u = i + k, v = i + k + len / 2;
                    double tRe = curRe * re[v] - curIm * im[v];
                    double tIm = curRe * im[v] + curIm * re[v];
                    re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                    re[u] += tRe; im[u] += tIm;
                    double nRe = curRe * wRe - curIm * wIm;
                    curIm = curRe * wIm + curIm * wRe;
                    curRe = nRe;
                }
            }
        }

        /* 计算HFC: 加权能量和 */
        double sumHFC = 0.0;
        for (int k = 0; k <= frameSize / 2; ++k) {
            double mag2 = re[k] * re[k] + im[k] * im[k];
            sumHFC += k * mag2;
        }

        hfc[frame] = sumHFC;
    }

    return hfc;
}

/**
 * @brief 重置所有统计计数器
 */
void OnsetDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

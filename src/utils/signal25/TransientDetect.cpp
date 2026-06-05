/**
 * @file TransientDetect.cpp
 * @brief 瞬态检测实现 — 频谱通量/高频能量/自适应阈值/峰值拾取
 */

#include "utils/signal25/TransientDetect.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
TransientDetect::TransientDetect(QObject* parent)
    : QObject(parent)
    , m_func(DetectFunction::SpectralFlux)
    , m_frameSize(1024)
    , m_hopSize(512)
    , m_sampleRate(44100.0)
    , m_thresholdMultiplier(1.5)
    , m_medianWindow(10)
{
}

/** @brief 设置检测函数 @param func 函数类型 */
void TransientDetect::setDetectFunction(DetectFunction func)
{
    m_func = func;
}

/** @brief 设置帧大小 @param size 帧大小 */
void TransientDetect::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
}

/** @brief 设置hop大小 @param size hop大小 */
void TransientDetect::setHopSize(int size)
{
    m_hopSize = qMax(1, size);
}

/** @brief 设置采样率 @param rate 采样率 */
void TransientDetect::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置阈值参数 @param multiplier 乘数 @param windowSize 中值窗口 */
void TransientDetect::setThresholdParams(double multiplier, int windowSize)
{
    m_thresholdMultiplier = qMax(0.1, multiplier);
    m_medianWindow = qMax(3, windowSize);
}

/** @brief 检测瞬态 @param data 时域数据 @return 瞬态事件列表 */
QList<TransientDetect::TransientEvent> TransientDetect::detect(
    const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<TransientEvent> events;
    if (data.size() < m_frameSize) return events;

    /* 计算检测函数曲线 */
    auto funcValues = detectionFunction(data);
    if (funcValues.isEmpty()) return events;

    /* 提取检测值 */
    QVector<double> detectCurve(funcValues.size());
    for (int i = 0; i < funcValues.size(); ++i) {
        detectCurve[i] = funcValues[i].second;
    }

    /* 自适应阈值峰值拾取 */
    QList<int> peaks = adaptivePeakPick(detectCurve);

    /* 计算最大检测值用于置信度归一化 */
    double maxVal = *std::max_element(detectCurve.begin(), detectCurve.end());
    if (maxVal < 1e-10) maxVal = 1.0;

    /* 构建事件列表 */
    for (int peakIdx : peaks) {
        TransientEvent ev;
        ev.frameIndex = funcValues[peakIdx].first;
        ev.timePosition = static_cast<double>(ev.frameIndex * m_hopSize)
            / m_sampleRate;
        ev.strength = detectCurve[peakIdx];
        ev.confidence = qBound(0.0, detectCurve[peakIdx] / maxVal, 1.0);
        events.append(ev);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalTransientsFound += events.size();
    int numFrames = (data.size() - m_frameSize) / m_hopSize + 1;
    m_stats.totalFramesProcessed += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit detectionComplete(events.size(), numFrames);
    return events;
}

/** @brief 计算检测函数曲线 @param data 时域数据 @return (帧索引, 检测值)列表 */
QVector<QPair<int, double>> TransientDetect::detectionFunction(
    const QVector<double>& data)
{
    QVector<QPair<int, double>> result;
    if (data.size() < m_frameSize) return result;

    int numFrames = (data.size() - m_frameSize) / m_hopSize + 1;
    result.reserve(numFrames);

    QVector<double> prevFrame, curFrame;
    QVector<double> prevMag, curMag;
    QVector<double> prevPhase, curPhase;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 提取并加窗 */
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i) {
            frame[i] = data[start + i];
        }
        frame = applyWindow(frame);

        /* FFT */
        QVector<double> real = frame, imag(m_frameSize, 0.0);
        forwardFFT(real, imag);

        /* 计算幅度和相位 */
        int halfN = m_frameSize / 2;
        curMag.resize(halfN);
        curPhase.resize(halfN);
        for (int i = 0; i < halfN; ++i) {
            curMag[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
            curPhase[i] = qAtan2(imag[i], real[i]);
        }

        double value = 0.0;
        if (f == 0) {
            value = 0.0;
        } else {
            switch (m_func) {
            case DetectFunction::SpectralFlux: {
                auto flux = computeSpectralFlux(prevMag, curMag);
                for (double v : flux) value += v;
                value /= flux.size();
                break;
            }
            case DetectFunction::HighFrequency: {
                auto hfc = computeHighFreqContent(curMag);
                value = hfc.value(hfc.size() / 2);
                for (int i = hfc.size() / 2; i < hfc.size(); ++i) value += hfc[i];
                break;
            }
            case DetectFunction::ComplexDomain:
                value = computeComplexDeviation(prevMag, curMag,
                                                prevPhase, curPhase);
                break;
            case DetectFunction::PhaseDeviation: {
                double phaseSum = 0.0;
                for (int i = 0; i < halfN; ++i) {
                    double dev = curPhase[i] - prevPhase[i];
                    while (dev > M_PI) dev -= 2.0 * M_PI;
                    while (dev < -M_PI) dev += 2.0 * M_PI;
                    phaseSum += qAbs(dev);
                }
                value = phaseSum / halfN;
                break;
            }
            }
        }

        result.append({f, value});
        prevMag = curMag;
        prevPhase = curPhase;
    }

    return result;
}

/** @brief 自适应阈值峰值拾取 @param detectFunc 检测值 @return 峰值帧索引 */
QList<int> TransientDetect::adaptivePeakPick(const QVector<double>& detectFunc)
{
    QList<int> peaks;
    int n = detectFunc.size();
    if (n < 3) return peaks;

    /* 计算中值滤波阈值 */
    QVector<double> threshold(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int halfW = m_medianWindow / 2;
        QVector<double> window;
        for (int j = qMax(0, i - halfW); j <= qMin(n - 1, i + halfW); ++j) {
            window.append(detectFunc[j]);
        }
        std::sort(window.begin(), window.end());
        double median = window[window.size() / 2];
        /* 计算局部标准差 */
        double mean = 0.0;
        for (double v : window) mean += v;
        mean /= window.size();
        double var = 0.0;
        for (double v : window) var += (v - mean) * (v - mean);
        double stdDev = qSqrt(var / window.size());

        threshold[i] = median + m_thresholdMultiplier * stdDev;
        threshold[i] = qMax(threshold[i], 1e-10);
    }

    /* 寻找超过阈值的局部峰值 */
    for (int i = 1; i < n - 1; ++i) {
        if (detectFunc[i] > threshold[i]
            && detectFunc[i] > detectFunc[i - 1]
            && detectFunc[i] >= detectFunc[i + 1]) {
            peaks.append(i);
        }
    }

    /* 最小间距过滤 */
    int minGap = m_frameSize / m_hopSize;
    QList<int> filtered;
    for (int p : peaks) {
        if (filtered.isEmpty() || p - filtered.last() >= minGap) {
            filtered.append(p);
        } else if (detectFunc[p] > detectFunc[filtered.last()]) {
            filtered.last() = p;
        }
    }

    for (int p : filtered) {
        emit transientFound(p, detectFunc[p]);
    }

    return filtered;
}

/** @brief 重置统计 */
void TransientDetect::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算频谱通量 @param prevFrame 前帧幅度 @param curFrame 当前帧幅度 @return 通量值 */
QVector<double> TransientDetect::computeSpectralFlux(
    const QVector<double>& prevFrame, const QVector<double>& curFrame)
{
    int n = qMin(prevFrame.size(), curFrame.size());
    QVector<double> flux(n);
    for (int i = 0; i < n; ++i) {
        double diff = curFrame[i] - prevFrame[i];
        flux[i] = qMax(0.0, diff); /* 半波整流 */
    }
    return flux;
}

/** @brief 计算高频能量 @param frame 频谱帧 @return 高频能量 */
QVector<double> TransientDetect::computeHighFreqContent(
    const QVector<double>& frame)
{
    int n = frame.size();
    QVector<double> hfc(n);
    for (int i = 0; i < n; ++i) {
        hfc[i] = static_cast<double>(i) * frame[i] * frame[i];
    }
    return hfc;
}

/** @brief 计算复数域偏差 @param prevMag 前帧幅度 @param curMag 当前幅度 @param prevPhase 前帧相位 @param curPhase 当前相位 @return 偏差值 */
double TransientDetect::computeComplexDeviation(
    const QVector<double>& prevMag, const QVector<double>& curMag,
    const QVector<double>& prevPhase, const QVector<double>& curPhase)
{
    int n = qMin(qMin(prevMag.size(), curMag.size()),
                 qMin(prevPhase.size(), curPhase.size()));
    double deviation = 0.0;
    for (int i = 0; i < n; ++i) {
        double expectedRe = prevMag[i] * qCos(prevPhase[i]);
        double expectedIm = prevMag[i] * qSin(prevPhase[i]);
        double actualRe = curMag[i] * qCos(curPhase[i]);
        double actualIm = curMag[i] * qSin(curPhase[i]);
        double dr = actualRe - expectedRe;
        double di = actualIm - expectedIm;
        deviation += qSqrt(dr * dr + di * di);
    }
    return deviation / n;
}

/** @brief 应用汉宁窗 @param data 输入数据 @return 加窗后数据 */
QVector<double> TransientDetect::applyWindow(const QVector<double>& data) const
{
    int n = data.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        windowed[i] = data[i] * w;
    }
    return windowed;
}

/** @brief 基2 FFT @param real 实部 @param imag 虚部 */
void TransientDetect::forwardFFT(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    imag.fill(0.0);

    /* 位反转 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = e + len / 2;
                double tRe = cRe * real[o] - cIm * imag[o];
                double tIm = cRe * imag[o] + cIm * real[o];
                real[o] = real[e] - tRe;
                imag[o] = imag[e] - tIm;
                real[e] += tRe;
                imag[e] += tIm;
                double nRe = cRe * wRe - cIm * wIm;
                double nIm = cRe * wIm + cIm * wRe;
                cRe = nRe; cIm = nIm;
            }
        }
    }
}

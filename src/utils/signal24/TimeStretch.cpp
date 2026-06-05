/**
 * @file TimeStretch.cpp
 * @brief 时间拉伸引擎实现 — 相位声码器+WSOLA波形相似匹配
 */

#include "utils/signal24/TimeStretch.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
TimeStretch::TimeStretch(QObject* parent)
    : QObject(parent)
    , m_algorithm(Algorithm::PhaseVocoder)
    , m_stretchFactor(1.0)
    , m_windowSize(2048)
    , m_hopAnalyse(512)
    , m_sampleRate(44100.0)
    , m_pvWritePos(0)
    , m_wsolaReadPos(0)
    , m_outputWritePos(0)
{
}

void TimeStretch::setAlgorithm(Algorithm algorithm) { m_algorithm = algorithm; }

void TimeStretch::setStretchFactor(double factor)
{
    m_stretchFactor = qBound(0.25, factor, 4.0);
}

void TimeStretch::setWindowSize(int size)
{
    m_windowSize = qMax(64, size);
    m_hopAnalyse = m_windowSize / 4;
}

void TimeStretch::setHopAnalyse(int hop) { m_hopAnalyse = qMax(1, hop); }
void TimeStretch::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); }

/**
 * @brief 处理输入音频数据
 * @param input 输入采样
 * @return 时间拉伸后的采样
 */
QVector<double> TimeStretch::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty() || m_stretchFactor <= 0.0) return output;

    switch (m_algorithm) {
    case Algorithm::PhaseVocoder:
        output = processPhaseVocoder(input);
        break;
    case Algorithm::WSOLA:
        output = processWSOLA(input);
        break;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFramesProcessed;
    m_stats.totalSamplesInput += static_cast<quint64>(input.size());
    m_stats.totalSamplesOutput += static_cast<quint64>(output.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFramesProcessed);

    emit frameProcessed(input.size(), output.size());
    return output;
}

/**
 * @brief 刷新剩余缓冲区数据
 * @return 剩余输出采样
 */
QVector<double> TimeStretch::flush()
{
    QVector<double> result;

    if (m_algorithm == Algorithm::PhaseVocoder && !m_pvBuffer.isEmpty()) {
        /* 输出相位声码器缓冲区中所有剩余数据 */
        int totalSize = m_pvBuffer.size();
        result.reserve(totalSize);
        for (int i = 0; i < totalSize; ++i) {
            result.append(m_pvBuffer[i]);
        }
        m_pvBuffer.clear();
        m_pvWritePos = 0;
    } else if (m_algorithm == Algorithm::WSOLA && !m_wsolaBuffer.isEmpty()) {
        /* 输出WSOLA缓冲区中所有剩余数据 */
        result.reserve(m_wsolaBuffer.size());
        for (int i = m_wsolaReadPos; i < m_wsolaBuffer.size(); ++i) {
            result.append(m_wsolaBuffer[i]);
        }
        m_wsolaBuffer.clear();
        m_wsolaReadPos = 0;
    }

    return result;
}

/** @brief 重置统计信息 */
void TimeStretch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_pvBuffer.clear();
    m_pvWritePos = 0;
    m_prevPhase.clear();
    m_phaseCumul.clear();
    m_wsolaBuffer.clear();
    m_wsolaReadPos = 0;
    m_outputBuffer.clear();
    m_outputWritePos = 0;
}

/**
 * @brief 相位声码器处理
 * @param input 输入采样
 * @return 拉伸后采样
 */
QVector<double> TimeStretch::processPhaseVocoder(const QVector<double>& input)
{
    int hopSynth = static_cast<int>(m_hopAnalyse * m_stretchFactor);
    if (hopSynth < 1) hopSynth = 1;

    int fftSize = m_windowSize;
    int halfSize = fftSize / 2;

    /* 初始化相位状态 */
    if (m_prevPhase.size() != halfSize + 1) {
        m_prevPhase.assign(halfSize + 1, 0.0);
        m_phaseCumul.assign(halfSize + 1, 0.0);
        m_pvBuffer.resize(hopSynth * 4 + fftSize);
        m_pvBuffer.fill(0.0);
        m_pvWritePos = 0;
    }

    QVector<double> window = hannWindow(fftSize);
    QVector<double> output;
    output.reserve(static_cast<int>(input.size() * m_stretchFactor));

    int numFrames = (input.size() - fftSize) / m_hopAnalyse + 1;
    if (numFrames < 0) numFrames = 0;

    /* 扩展输出缓冲区 */
    int neededSize = m_pvWritePos + numFrames * hopSynth + fftSize;
    if (m_pvBuffer.size() < neededSize) {
        m_pvBuffer.resize(neededSize);
    }

    for (int frame = 0; frame < numFrames; ++frame) {
        int pos = frame * m_hopAnalyse;

        /* 加窗 */
        QVector<double> real(fftSize, 0.0);
        QVector<double> imag(fftSize, 0.0);
        for (int i = 0; i < fftSize && pos + i < input.size(); ++i) {
            real[i] = input[pos + i] * window[i];
        }

        /* 简化DFT(仅计算前半部分) */
        int bins = halfSize + 1;
        QVector<double> mag(bins, 0.0);
        QVector<double> phase(bins, 0.0);

        for (int k = 0; k < bins; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < fftSize; ++n) {
                double angle = -2.0 * M_PI * k * n / fftSize;
                re += real[n] * qCos(angle);
                im += real[n] * qSin(angle);
            }
            mag[k] = qSqrt(re * re + im * im);
            phase[k] = qAtan2(im, re);
        }

        /* 相位校正: 保持频率连续性 */
        double omega = 2.0 * M_PI * m_hopAnalyse / fftSize;
        for (int k = 0; k < bins; ++k) {
            double deltaPhase = phase[k] - m_prevPhase[k];
            /* 期望相位增量 */
            double expected = omega * k;
            /* 去除期望增量，得到偏差 */
            double deviation = deltaPhase - expected;
            /* 包裹到[-pi, pi] */
            while (deviation > M_PI) deviation -= 2.0 * M_PI;
            while (deviation < -M_PI) deviation += 2.0 * M_PI;
            /* 真实频率 */
            double trueFreq = expected + deviation;
            /* 累积相位(按合成跳跃推进) */
            m_phaseCumul[k] += trueFreq * m_stretchFactor;
            m_prevPhase[k] = phase[k];
        }

        /* 逆变换(简化: 使用累积相位重建) */
        QVector<double> synth(fftSize, 0.0);
        for (int n = 0; n < fftSize; ++n) {
            double val = 0.0;
            for (int k = 0; k < bins; ++k) {
                val += mag[k] * qCos(m_phaseCumul[k] + 2.0 * M_PI * k * n / fftSize);
            }
            synth[n] = val / bins * window[n];
        }

        /* 重叠叠加到输出缓冲区 */
        int writeBase = m_pvWritePos + frame * hopSynth;
        for (int i = 0; i < fftSize && writeBase + i < m_pvBuffer.size(); ++i) {
            m_pvBuffer[writeBase + i] += synth[i];
        }
    }

    /* 提取输出 */
    int outputSamples = numFrames * hopSynth;
    for (int i = 0; i < outputSamples && i < m_pvBuffer.size(); ++i) {
        output.append(m_pvBuffer[i]);
    }

    /* 移除已输出的数据，保留尾部用于重叠 */
    int keepFrom = outputSamples;
    int keepLen = m_pvBuffer.size() - keepFrom;
    QVector<double> remaining(keepLen);
    for (int i = 0; i < keepLen; ++i) remaining[i] = m_pvBuffer[keepFrom + i];
    m_pvBuffer = remaining;
    m_pvWritePos = 0;

    return output;
}

/**
 * @brief WSOLA处理
 * @param input 输入采样
 * @return 拉伸后采样
 */
QVector<double> TimeStretch::processWSOLA(const QVector<double>& input)
{
    int hopSynth = static_cast<int>(m_hopAnalyse * m_stretchFactor);
    if (hopSynth < 1) hopSynth = 1;

    int halfWin = m_windowSize / 2;
    QVector<double> window = hannWindow(m_windowSize);

    /* 追加到输入缓冲区 */
    m_wsolaBuffer.append(input);

    QVector<double> output;
    output.reserve(static_cast<int>(input.size() * m_stretchFactor));

    /* 确保输出缓冲区足够大 */
    int neededOutput = m_outputWritePos + m_wsolaBuffer.size();
    if (m_outputBuffer.size() < neededOutput) {
        m_outputBuffer.resize(neededOutput);
    }

    int searchRange = m_hopAnalyse / 2;

    while (m_wsolaReadPos + m_windowSize <= m_wsolaBuffer.size()) {
        /* 在理想位置附近搜索最佳匹配 */
        int bestPos = findBestMatch(m_wsolaBuffer, m_wsolaReadPos, searchRange);
        if (bestPos < 0) bestPos = m_wsolaReadPos;

        /* 提取窗口内数据并加窗 */
        for (int i = 0; i < m_windowSize; ++i) {
            int srcIdx = bestPos + i;
            if (srcIdx < m_wsolaBuffer.size()) {
                int dstIdx = m_outputWritePos + i;
                if (dstIdx >= m_outputBuffer.size()) {
                    m_outputBuffer.resize(dstIdx + 1);
                }
                m_outputBuffer[dstIdx] += m_wsolaBuffer[srcIdx] * window[i];
            }
        }

        m_wsolaReadPos += m_hopAnalyse;
        m_outputWritePos += hopSynth;
    }

    /* 提取输出 */
    int outLen = qMin(m_outputWritePos, m_outputBuffer.size());
    for (int i = 0; i < outLen; ++i) {
        output.append(m_outputBuffer[i]);
    }

    /* 清理已消费的缓冲区 */
    if (m_wsolaReadPos > 0) {
        int keepLen = m_wsolaBuffer.size() - m_wsolaReadPos;
        QVector<double> remaining(keepLen);
        for (int i = 0; i < keepLen; ++i) remaining[i] = m_wsolaBuffer[m_wsolaReadPos + i];
        m_wsolaBuffer = remaining;
        m_wsolaReadPos = 0;
    }

    /* 清理输出缓冲区 */
    if (m_outputWritePos > 0) {
        int keepLen = m_outputBuffer.size() - outLen;
        QVector<double> remaining(keepLen);
        for (int i = 0; i < keepLen; ++i) remaining[i] = m_outputBuffer[outLen + i];
        m_outputBuffer = remaining;
        m_outputWritePos = 0;
    }

    return output;
}

/**
 * @brief 生成Hann窗函数
 * @param size 窗口大小
 * @return 窗函数系数
 */
QVector<double> TimeStretch::hannWindow(int size) const
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i) {
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
    }
    return w;
}

/**
 * @brief WSOLA最佳匹配位置搜索
 * @param buffer 输入缓冲区
 * @param position 理想位置
 * @param searchRange 搜索范围
 * @return 最佳匹配位置
 */
double TimeStretch::findBestMatch(const QVector<double>& buffer,
                                  int position, int searchRange) const
{
    int halfWin = m_windowSize / 2;
    int start = qMax(0, position - searchRange);
    int end = qMin(static_cast<int>(buffer.size()) - m_windowSize,
                   position + searchRange);
    if (end < start) return position;

    int bestPos = position;
    double bestCorr = -1e300;

    for (int pos = start; pos <= end; ++pos) {
        double corr = 0.0;
        for (int i = 0; i < m_windowSize && pos + i < buffer.size(); ++i) {
            corr += buffer[pos + i] * buffer[position + i];
        }
        if (corr > bestCorr) {
            bestCorr = corr;
            bestPos = pos;
        }
    }
    return bestPos;
}

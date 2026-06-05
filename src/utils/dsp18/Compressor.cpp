/**
 * @file Compressor.cpp
 * @brief 动态范围压缩器实现 — 包络跟随/膝函数/侧链检测
 */

#include "utils/dsp18/Compressor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Compressor::Compressor(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_envelopeDb(-120.0)
    , m_attackCoeff(0.0)
    , m_releaseCoeff(0.0)
    , m_rmsWindowIdx(0)
    , m_grSum(0.0)
    , m_grCount(0)
    , m_timeSum(0.0)
{
    /* 初始化RMS窗口 */
    int windowSize = static_cast<int>(m_sampleRate * 0.01); /* 10ms */
    m_rmsWindow.resize(qMax(1, windowSize), 0.0);

    /* 计算包络系数 */
    updateEnvelope(m_envelopeDb);
    m_attackCoeff = qExp(-1.0 / (m_params.attackMs * 0.001 * m_sampleRate));
    m_releaseCoeff = qExp(-1.0 / (m_params.releaseMs * 0.001 * m_sampleRate));
}

/** @brief 设置参数 @param params 压缩器参数 */
void Compressor::setParameters(const Parameters& params)
{
    m_params = params;

    /* 重新计算包络系数 */
    m_attackCoeff = qExp(-1.0 / (qMax(0.01, m_params.attackMs) * 0.001 * m_sampleRate));
    m_releaseCoeff = qExp(-1.0 / (qMax(0.01, m_params.releaseMs) * 0.001 * m_sampleRate));

    /* RMS窗口大小 */
    int windowSize = static_cast<int>(m_sampleRate * 0.01);
    m_rmsWindow.resize(qMax(1, windowSize), 0.0);
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void Compressor::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    m_attackCoeff = qExp(-1.0 / (qMax(0.01, m_params.attackMs) * 0.001 * m_sampleRate));
    m_releaseCoeff = qExp(-1.0 / (qMax(0.01, m_params.releaseMs) * 0.001 * m_sampleRate));

    int windowSize = static_cast<int>(m_sampleRate * 0.01);
    m_rmsWindow.resize(qMax(1, windowSize), 0.0);
}

/** @brief 处理单个样本 @param input 输入样本 @return 输出样本 */
double Compressor::processSample(double input)
{
    return processSampleSidechain(input, input);
}

/** @brief 处理带侧链输入的样本 @param input 主输入 @param sidechain 侧链输入 @return 输出 */
double Compressor::processSampleSidechain(double input, double sidechain)
{
    /* 检测侧链电平 */
    double absSidechain = qAbs(sidechain);

    /* 更新RMS窗口 */
    m_rmsWindow[m_rmsWindowIdx] = sidechain;
    m_rmsWindowIdx = (m_rmsWindowIdx + 1) % m_rmsWindow.size();

    /* 电平检测 */
    double levelLin = 0.0;
    switch (m_params.detection) {
    case DetectionMode::RMS: {
        double sumSq = 0.0;
        for (double s : m_rmsWindow) sumSq += s * s;
        levelLin = qSqrt(sumSq / m_rmsWindow.size());
        break;
    }
    case DetectionMode::Peak:
        levelLin = absSidechain;
        break;
    case DetectionMode::Hybrid: {
        double sumSq = 0.0;
        for (double s : m_rmsWindow) sumSq += s * s;
        double rms = qSqrt(sumSq / m_rmsWindow.size());
        levelLin = 0.5 * rms + 0.5 * absSidechain;
        break;
    }
    }

    /* 转换为dB */
    double levelDb = (levelLin > 1e-10) ? 20.0 * qLn(levelLin) / qLn(10.0) : -120.0;

    /* 更新包络 */
    double targetDb = levelDb;
    if (targetDb > m_envelopeDb) {
        m_envelopeDb = m_attackCoeff * m_envelopeDb + (1.0 - m_attackCoeff) * targetDb;
    } else {
        m_envelopeDb = m_releaseCoeff * m_envelopeDb + (1.0 - m_releaseCoeff) * targetDb;
    }

    /* 计算增益衰减 */
    double grDb = computeGainReduction(m_envelopeDb);

    /* 增益补偿 */
    double totalGainDb = grDb + m_params.makeupGainDb;

    /* 干湿混合 */
    double drySignal = input;
    double wetSignal = input * qPow(10.0, totalGainDb / 20.0);
    double output = m_params.mix * wetSignal + (1.0 - m_params.mix) * drySignal;

    /* 更新分析 */
    m_lastAnalysis.inputLevelDb = levelDb;
    m_lastAnalysis.outputLevelDb = levelDb + totalGainDb;
    m_lastAnalysis.gainReductionDb = grDb;
    m_lastAnalysis.envelopeDb = m_envelopeDb;

    /* 统计 */
    ++m_stats.totalSamplesProcessed;
    m_grSum += qAbs(grDb);
    ++m_grCount;
    m_stats.avgGainReductionDb = m_grSum / m_grCount;
    if (qAbs(grDb) > m_stats.peakGainReductionDb) {
        m_stats.peakGainReductionDb = qAbs(grDb);
    }

    if (levelDb > m_params.thresholdDb) {
        ++m_stats.totalSidechainTriggers;
        emit sidechainTriggered(levelDb);
    }

    emit gainReductionChanged(grDb);
    return output;
}

/** @brief 批量处理 @param input 输入缓冲区 @return 输出缓冲区 */
QVector<double> Compressor::process(const QVector<double>& input)
{
    return processSidechain(input, input);
}

/** @brief 批量处理带侧链 @param input 主输入 @param sidechain 侧链 @return 输出 */
QVector<double> Compressor::processSidechain(const QVector<double>& input,
                                              const QVector<double>& sidechain)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), sidechain.size());
    QVector<double> output(n);
    for (int i = 0; i < n; ++i) {
        output[i] = processSampleSidechain(input[i], sidechain[i]);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum;

    return output;
}

/** @brief 计算静态传输特性 @param inputDb 输入电平(dB) @return 输出电平(dB) */
double Compressor::transferCharacteristic(double inputDb) const
{
    double threshold = m_params.thresholdDb;
    double ratio = m_params.ratio;

    if (m_params.kneeType == KneeType::Hard) {
        /* 硬膝 */
        if (inputDb <= threshold) return inputDb;
        return threshold + (inputDb - threshold) / ratio;
    }

    /* 软膝 */
    double halfKnee = m_params.kneeDb / 2.0;
    if (inputDb < threshold - halfKnee) return inputDb;
    if (inputDb > threshold + halfKnee) {
        return threshold + (inputDb - threshold) / ratio;
    }

    /* 膝区域内的二次插值 */
    double x = inputDb - threshold + halfKnee;
    double kneeFactor = (x * x) / (2.0 * m_params.kneeDb);
    return inputDb + kneeFactor * (1.0 / ratio - 1.0);
}

/** @brief 重置内部状态 */
void Compressor::reset()
{
    m_envelopeDb = -120.0;
    m_rmsWindow.fill(0.0);
    m_rmsWindowIdx = 0;
    m_lastAnalysis = GainAnalysis{};
}

/** @brief 重置统计 */
void Compressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_grSum = 0.0;
    m_grCount = 0;
}

/**
 * @brief 计算增益衰减量 @param inputDb 输入电平(dB)
 * @return 增益衰减(dB，负值表示衰减)
 */
double Compressor::computeGainReduction(double inputDb)
{
    double threshold = m_params.thresholdDb;
    double ratio = m_params.ratio;

    if (m_params.kneeType == KneeType::Hard) {
        if (inputDb <= threshold) return 0.0;
        return threshold + (inputDb - threshold) / ratio - inputDb;
    }

    /* 软膝 */
    double halfKnee = m_params.kneeDb / 2.0;
    if (inputDb < threshold - halfKnee) return 0.0;
    if (inputDb > threshold + halfKnee) {
        return threshold + (inputDb - threshold) / ratio - inputDb;
    }

    double outputDb = transferCharacteristic(inputDb);
    return outputDb - inputDb;
}

/** @brief 检测电平 @param window 检测窗口 @return 线性电平 */
double Compressor::detectLevel(const QVector<double>& window) const
{
    if (window.isEmpty()) return 0.0;

    switch (m_params.detection) {
    case DetectionMode::RMS: {
        double sumSq = 0.0;
        for (double s : window) sumSq += s * s;
        return qSqrt(sumSq / window.size());
    }
    case DetectionMode::Peak: {
        double peak = 0.0;
        for (double s : window) peak = qMax(peak, qAbs(s));
        return peak;
    }
    case DetectionMode::Hybrid: {
        double sumSq = 0.0;
        double peak = 0.0;
        for (double s : window) {
            sumSq += s * s;
            peak = qMax(peak, qAbs(s));
        }
        return 0.5 * qSqrt(sumSq / window.size()) + 0.5 * peak;
    }
    }
    return 0.0;
}

/** @brief 更新包络 @param targetDb 目标电平 */
void Compressor::updateEnvelope(double targetDb)
{
    if (targetDb > m_envelopeDb) {
        m_envelopeDb = m_attackCoeff * m_envelopeDb + (1.0 - m_attackCoeff) * targetDb;
    } else {
        m_envelopeDb = m_releaseCoeff * m_envelopeDb + (1.0 - m_releaseCoeff) * targetDb;
    }
}

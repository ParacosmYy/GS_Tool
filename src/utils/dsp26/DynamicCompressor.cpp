/**
 * @file DynamicCompressor.cpp
 * @brief 动态压缩器实现
 */

#include "utils/dsp26/DynamicCompressor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

DynamicCompressor::DynamicCompressor(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_mode(Mode::Compressor)
    , m_threshold(-20.0)
    , m_ratio(4.0)
    , m_attackMs(10.0)
    , m_releaseMs(100.0)
    , m_kneeWidth(6.0)
    , m_makeupGain(0.0)
    , m_envelopeState(0.0)
    , m_timeSum(0.0)
{
}

void DynamicCompressor::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

void DynamicCompressor::setMode(Mode mode)
{
    m_mode = mode;
}

void DynamicCompressor::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

void DynamicCompressor::setRatio(double ratio)
{
    m_ratio = qMax(1.0, ratio);
}

void DynamicCompressor::setAttack(double attackMs)
{
    m_attackMs = qMax(0.01, attackMs);
}

void DynamicCompressor::setRelease(double releaseMs)
{
    m_releaseMs = qMax(0.01, releaseMs);
}

void DynamicCompressor::setKneeWidth(double kneeDb)
{
    m_kneeWidth = qMax(0.0, kneeDb);
}

void DynamicCompressor::setMakeupGain(double gainDb)
{
    m_makeupGain = gainDb;
}

void DynamicCompressor::setBands(const QList<BandParam>& bands)
{
    m_bands = bands;
}

DynamicCompressor::ProcessResult DynamicCompressor::process(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    if (input.isEmpty()) return result;

    int n = input.size();
    result.output.resize(n);

    double peakIn = 0.0, peakOut = 0.0, maxGainReduction = 0.0;
    double totalGainReduction = 0.0;

    /* attack/release系数 */
    double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate));

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        /* 检测削波 */
        if (qAbs(sample) > 1.0) {
            emit clippingDetected(i, sample);
        }

        /* 输入电平(dB) */
        double inputDb = 20.0 * qLog10(qMax(qAbs(sample), 1e-10));
        if (qAbs(inputDb) > peakIn) peakIn = inputDb;

        /* 包络跟随 */
        double targetEnv = inputDb;
        double coeff = (targetEnv > m_envelopeState) ? attackCoeff : releaseCoeff;
        m_envelopeState = coeff * m_envelopeState + (1.0 - coeff) * targetEnv;

        /* 增益计算 */
        double gainDb = staticCharacteristic(m_envelopeState);
        double gainReduction = gainDb - m_envelopeState;
        if (gainReduction < maxGainReduction) maxGainReduction = gainReduction;
        totalGainReduction += gainReduction;

        /* 应用增益+补偿 */
        double finalGain = gainDb + m_makeupGain;
        double outputSample = sample * qPow(10.0, finalGain / 20.0);
        result.output[i] = outputSample;

        double outDb = 20.0 * qLog10(qMax(qAbs(outputSample), 1e-10));
        if (qAbs(outDb) > peakOut) peakOut = outDb;
    }

    result.peakInputDb = peakIn;
    result.peakOutputDb = peakOut;
    result.gainReductionDb = maxGainReduction;

    ++m_stats.totalFrames;
    m_stats.totalSamples += n;
    double avgGain = (n > 0) ? totalGainReduction / n : 0.0;
    m_stats.avgGainReductionDb =
        (m_stats.avgGainReductionDb * (m_stats.totalFrames - 1) + avgGain)
        / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(maxGainReduction, peakOut);
    return result;
}

QVector<double> DynamicCompressor::gainCurve(const QVector<double>& inputDb) const
{
    QVector<double> curve;
    curve.reserve(inputDb.size());
    for (double db : inputDb) {
        double gainDb = staticCharacteristic(db);
        curve.append(gainDb + m_makeupGain);
    }
    return curve;
}

double DynamicCompressor::staticCharacteristic(double inputDb) const
{
    double outputDb = inputDb;
    double halfKnee = m_kneeWidth / 2.0;
    bool inKnee = (m_kneeWidth > 0.0)
        && (inputDb > m_threshold - halfKnee)
        && (inputDb < m_threshold + halfKnee);

    switch (m_mode) {
    case Mode::Compressor:
        if (inKnee) {
            /* 软拐点: 二次插值 */
            double x = inputDb - m_threshold + halfKnee;
            double t = x / m_kneeWidth;
            outputDb = inputDb + (1.0 / m_ratio - 1.0) * t * x / 2.0;
        } else if (inputDb > m_threshold) {
            outputDb = m_threshold + (inputDb - m_threshold) / m_ratio;
        }
        break;
    case Mode::Expander:
        if (inputDb < m_threshold) {
            outputDb = m_threshold + (inputDb - m_threshold) * m_ratio;
        }
        break;
    case Mode::NoiseGate:
        if (inputDb < m_threshold) {
            outputDb = -120.0; /* 硬静音 */
        } else if (inKnee) {
            double x = inputDb - m_threshold + halfKnee;
            double t = x / m_kneeWidth;
            outputDb = inputDb + (-120.0 - inputDb) * (1.0 - t);
        }
        break;
    case Mode::Limiter:
        if (inputDb > m_threshold) {
            outputDb = m_threshold;
        } else if (inKnee && inputDb > m_threshold - halfKnee) {
            double x = inputDb - m_threshold + halfKnee;
            double t = x / m_kneeWidth;
            outputDb = m_threshold - halfKnee + t * halfKnee;
        }
        break;
    }
    return outputDb;
}

double DynamicCompressor::envelopeFollow(double inputDb, double currentGainDb)
{
    Q_UNUSED(currentGainDb)
    double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate));
    double coeff = (inputDb > m_envelopeState) ? attackCoeff : releaseCoeff;
    m_envelopeState = coeff * m_envelopeState + (1.0 - coeff) * inputDb;
    return m_envelopeState;
}

void DynamicCompressor::applyBandFilter(const QVector<double>& input,
    int bandIndex, QVector<double>& lowOut, QVector<double>& highOut)
{
    Q_UNUSED(bandIndex)
    /* 简单一阶低通/高通分离 */
    int n = input.size();
    lowOut.resize(n);
    highOut.resize(n);
    double cutoff = (m_bands.isEmpty()) ? 1000.0 : m_bands.first().lowFreq;
    double rc = 1.0 / (2.0 * M_PI * cutoff);
    double dt = 1.0 / m_sampleRate;
    double alpha = dt / (rc + dt);
    double prevLow = 0.0;
    for (int i = 0; i < n; ++i) {
        lowOut[i] = alpha * input[i] + (1.0 - alpha) * prevLow;
        highOut[i] = input[i] - lowOut[i];
        prevLow = lowOut[i];
    }
}

DynamicCompressor::ProcessResult DynamicCompressor::processMultiband(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    if (input.isEmpty() || m_bands.isEmpty()) {
        return process(input);
    }

    int n = input.size();
    int numBands = m_bands.size();

    /* 频段分离: 级联分频 */
    QList<QVector<double>> bandSignals;
    QVector<double> remaining = input;
    for (int b = 0; b < numBands; ++b) {
        QVector<double> low, high;
        applyBandFilter(remaining, b, low, high);
        if (b < numBands - 1) {
            bandSignals.append(low);
            remaining = high;
        } else {
            bandSignals.append(low);
            bandSignals.append(high);
        }
    }

    /* 对每个频段独立处理 */
    double peakIn = 0.0, peakOut = 0.0, maxGainReduction = 0.0;
    double totalGainReduction = 0.0;

    QVector<double> output(n, 0.0);
    for (int b = 0; b < bandSignals.size() && b < numBands; ++b) {
        const BandParam& param = m_bands[b];
        double bandAttackCoeff = qExp(-1.0 / (param.attackMs * 0.001 * m_sampleRate));
        double bandReleaseCoeff = qExp(-1.0 / (param.releaseMs * 0.001 * m_sampleRate));
        double bandEnvelope = 0.0;

        /* 临时设置当前频段参数 */
        double savedThreshold = m_threshold;
        double savedRatio = m_ratio;
        double savedKnee = m_kneeWidth;
        double savedMakeup = m_makeupGain;
        m_threshold = param.threshold;
        m_ratio = param.ratio;
        m_kneeWidth = param.kneeWidth;
        m_makeupGain = param.makeupGain;

        for (int i = 0; i < n; ++i) {
            double sample = bandSignals[b][i];
            double inputDb = 20.0 * qLog10(qMax(qAbs(sample), 1e-10));
            if (qAbs(inputDb) > peakIn) peakIn = inputDb;

            double coeff = (inputDb > bandEnvelope) ? bandAttackCoeff : bandReleaseCoeff;
            bandEnvelope = coeff * bandEnvelope + (1.0 - coeff) * inputDb;

            double gainDb = staticCharacteristic(bandEnvelope);
            double reduction = gainDb - bandEnvelope;
            if (reduction < maxGainReduction) maxGainReduction = reduction;
            totalGainReduction += reduction;

            double finalGain = gainDb + param.makeupGain;
            output[i] += sample * qPow(10.0, finalGain / 20.0);

            double outDb = 20.0 * qLog10(qMax(qAbs(output[i]), 1e-10));
            if (qAbs(outDb) > peakOut) peakOut = outDb;
        }

        /* 恢复全局参数 */
        m_threshold = savedThreshold;
        m_ratio = savedRatio;
        m_kneeWidth = savedKnee;
        m_makeupGain = savedMakeup;
    }

    result.output = output;
    result.peakInputDb = peakIn;
    result.peakOutputDb = peakOut;
    result.gainReductionDb = maxGainReduction;

    ++m_stats.totalFrames;
    m_stats.totalSamples += n;
    double avgGain = (n > 0) ? totalGainReduction / (n * bandSignals.size()) : 0.0;
    m_stats.avgGainReductionDb =
        (m_stats.avgGainReductionDb * (m_stats.totalFrames - 1) + avgGain)
        / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(maxGainReduction, peakOut);
    return result;
}

double DynamicCompressor::computeAutoMakeupGain(const QVector<double>& input)
{
    if (input.isEmpty()) return 0.0;
    double peakInput = 0.0;
    for (double s : input) {
        double absVal = qAbs(s);
        if (absVal > peakInput) peakInput = absVal;
    }
    double inputDb = 20.0 * qLog10(qMax(peakInput, 1e-10));
    /* 自动补偿: 使输出峰值接近0dBFS */
    double targetDb = -1.0;
    double neededGain = targetDb - inputDb;
    /* 考虑压缩对峰值的影响 */
    if (inputDb > m_threshold) {
        double compressedDb = m_threshold + (inputDb - m_threshold) / m_ratio;
        neededGain = targetDb - compressedDb;
    }
    return qBound(0.0, neededGain, 30.0);
}

void DynamicCompressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelopeState = 0.0;
}

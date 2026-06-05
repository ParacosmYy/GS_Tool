/**
 * @file MultibandGate.cpp
 * @brief MultibandGate 实现
 *
 * 实现6段噪声门：Linkwitz-Riley分频滤波、包络检测、
 * 迟滞门控判断、attack/release增益平滑。
 */

#include "utils/dsp161/MultibandGate.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
MultibandGate::MultibandGate(QObject* parent)
    : QObject(parent)
{
    m_bands.resize(BAND_COUNT);
    m_states.resize(BAND_COUNT);
    initDefaultBands();
}

void MultibandGate::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    /* 重新设计滤波器 */
    for (int i = 0; i < BAND_COUNT; ++i) {
        designCrossoverFilter(i, m_bands[i].lowFreq, m_bands[i].highFreq);
    }
}

void MultibandGate::setBandConfig(int band, const BandConfig& config)
{
    if (band < 0 || band >= BAND_COUNT) return;
    m_bands[band] = config;
    designCrossoverFilter(band, config.lowFreq, config.highFreq);
}

/**
 * @brief 初始化默认6段频率划分
 *
 * 频段划分(近似): 0-80, 80-250, 250-1000, 1000-4000, 4000-8000, 8000-22050
 */
void MultibandGate::initDefaultBands()
{
    double boundaries[BAND_COUNT + 1] = {0, 80, 250, 1000, 4000, 8000, 22050};

    for (int i = 0; i < BAND_COUNT; ++i) {
        m_bands[i].lowFreq = boundaries[i];
        m_bands[i].highFreq = boundaries[i + 1];
        m_bands[i].threshold = -40.0 + i * 2.0;
        m_bands[i].hysteresis = 6.0;
        m_bands[i].attack = 1.0;
        m_bands[i].release = 50.0;
        m_bands[i].hold = 10.0;
        m_bands[i].range = -80.0;
        designCrossoverFilter(i, m_bands[i].lowFreq, m_bands[i].highFreq);
    }

    /* 重置状态 */
    for (auto& state : m_states) {
        state.gateOpen = false;
        state.envelopeLevel = -120.0;
        state.gain = 1.0;
        state.holdTimer = 0.0;
        for (int j = 0; j < 4; ++j) state.filterState[j] = 0.0;
    }
}

/**
 * @brief 设计2阶Butterworth带通滤波器
 *
 * 使用双线性变换从模拟原型设计数字滤波器。
 */
void MultibandGate::designCrossoverFilter(int band, double lowFreq, double highFreq)
{
    if (band < 0 || band >= BAND_COUNT) return;

    /* 2阶Butterworth带通 */
    double fs = m_sampleRate;
    double fLow = qMax(20.0, lowFreq);
    double fHigh = qMin(fs * 0.49, highFreq);

    if (fLow >= fHigh) {
        /* 退化为全通 */
        m_states[band].biquadB = {1.0, 0.0, 0.0};
        m_states[band].biquadA = {1.0, 0.0, 0.0};
        return;
    }

    double wLow = 2.0 * M_PI * fLow / fs;
    double wHigh = 2.0 * M_PI * fHigh / fs;

    /* 预扭曲 */
    double wL = 2.0 * qTan(wLow / 2.0);
    double wH = 2.0 * qTan(wHigh / 2.0);
    double w0 = qSqrt(wL * wH);
    double bw = wH - wL;

    /* 归一化 */
    double alpha = bw / (2.0 * w0);

    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * qCos(2.0 * qAtan2(w0, 2.0));
    double a2 = 1.0 - alpha;

    m_states[band].biquadB = {b0 / a0, b1 / a0, b2 / a0};
    m_states[band].biquadA = {1.0, a1 / a0, a2 / a0};
}

/**
 * @brief 应用双二阶滤波器
 */
double MultibandGate::applyFilter(int band, double sample)
{
    auto& s = m_states[band];
    const auto& b = s.biquadB;
    const auto& a = s.biquadA;

    double y = b[0] * sample + b[1] * s.filterState[0] + b[2] * s.filterState[1]
             - a[1] * s.filterState[2] - a[2] * s.filterState[3];

    s.filterState[1] = s.filterState[0];
    s.filterState[0] = sample;
    s.filterState[3] = s.filterState[2];
    s.filterState[2] = y;

    return y;
}

/**
 * @brief 处理一帧音频
 *
 * 对每个采样点：
 * 1) 分频滤波到6个频段
 * 2) 各频段包络检测
 * 3) 迟滞门控判断
 * 4) 增益平滑(attack/release)
 * 5) 重新合成输出
 */
QVector<double> MultibandGate::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int numSamples = input.size();
    if (numSamples == 0) return QVector<double>();

    QVector<double> output(numSamples, 0.0);
    double sampleDuration = 1000.0 / m_sampleRate; /* ms per sample */

    for (int s = 0; s < numSamples; ++s) {
        double sample = input[s];
        double reconstructed = 0.0;

        for (int b = 0; b < BAND_COUNT; ++b) {
            /* 带通滤波 */
            double filtered = applyFilter(b, sample);
            double absVal = qAbs(filtered);

            /* 包络检测(RMS平滑) */
            double levelDB = toDB(absVal);
            double smoothCoeff = 0.1;
            m_states[b].envelopeLevel = smoothCoeff * levelDB
                + (1.0 - smoothCoeff) * m_states[b].envelopeLevel;

            /* 迟滞门控 */
            double thresh = m_bands[b].threshold;
            double hyst = m_bands[b].hysteresis;
            bool wasOpen = m_states[b].gateOpen;

            if (!wasOpen && m_states[b].envelopeLevel > thresh) {
                m_states[b].gateOpen = true;
                m_states[b].holdTimer = m_bands[b].hold;
                m_stats.totalGateEvents++;
            } else if (wasOpen && m_states[b].envelopeLevel < (thresh - hyst)) {
                m_states[b].holdTimer -= sampleDuration;
                if (m_states[b].holdTimer <= 0.0) {
                    m_states[b].gateOpen = false;
                }
            }

            /* 增益平滑 */
            double targetGain = m_states[b].gateOpen ? 1.0 : fromDB(m_bands[b].range);
            double coeff;
            if (targetGain > m_states[b].gain) {
                /* Attack */
                coeff = (m_bands[b].attack > 0.0)
                    ? 1.0 - qExp(-sampleDuration / m_bands[b].attack) : 1.0;
            } else {
                /* Release */
                coeff = (m_bands[b].release > 0.0)
                    ? 1.0 - qExp(-sampleDuration / m_bands[b].release) : 1.0;
            }
            m_states[b].gain += coeff * (targetGain - m_states[b].gain);

            /* 应用增益 */
            reconstructed += filtered * m_states[b].gain;
        }

        output[s] = reconstructed;
    }

    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit frameProcessed(numSamples);
    return output;
}

void MultibandGate::reset()
{
    for (auto& state : m_states) {
        state.gateOpen = false;
        state.envelopeLevel = -120.0;
        state.gain = 1.0;
        state.holdTimer = 0.0;
        for (int j = 0; j < 4; ++j) state.filterState[j] = 0.0;
    }
}

void MultibandGate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

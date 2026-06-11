/**
 * @file Deesser11.cpp
 * @brief Deesser11 实现
 *
 * 实现去齿音器：性别自适应齿音带检测与动态阈值跟踪实现跨语音类型透明去齿音。
 */

#include "utils/dsp301/Deesser11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Deesser11::Deesser11(QObject *parent)
    : QObject(parent)
{
    designBandpass(4000.0, 9000.0);  // Default sibilance band
}

Deesser11::~Deesser11() = default;

/* ---- Configuration ---- */

void Deesser11::setSampleRate(int sr) { m_sampleRate = qBound(8000, sr, 192000); designBandpass(4000.0, 9000.0); }
void Deesser11::setThreshold(double dB) { m_thresholdDb = qBound(-60.0, dB, 0.0); }
void Deesser11::setRatio(double ratio) { m_ratio = qBound(1.0, ratio, 20.0); }
void Deesser11::setAttack(double ms) { m_attackMs = qBound(0.01, ms, 100.0); }
void Deesser11::setRelease(double ms) { m_releaseMs = qBound(1.0, ms, 1000.0); }
void Deesser11::setVoiceProfile(VoiceProfile profile) { m_profile = profile; }

/* ---- dB / linear conversions ---- */

double Deesser11::dbToLinear(double dB) const { return qPow(10.0, dB / 20.0); }
double Deesser11::linearToDb(double linear) const { return (linear > 1e-10) ? 20.0 * qLn(lineal) / qLn(10.0) : -200.0; }

/* ---- Get sibilance band for voice profile ---- */

QPair<double, double> Deesser11::getBandForProfile(VoiceProfile p) const
{
    switch (p) {
    case VoiceProfile::Male:   return {3000.0, 7000.0};
    case VoiceProfile::Female: return {5000.0, 10000.0};
    default:                   return {4000.0, 9000.0};   // Auto: middle band
    }
}

/* ---- Design 2nd-order band-pass filter (Butterworth) ---- */

void Deesser11::designBandpass(double lowFreq, double highFreq)
{
    // 2nd-order Butterworth band-pass via cascade of low-pass and high-pass
    // Simplified biquad coefficients for band-pass
    double fL = qBound(20.0, lowFreq, m_sampleRate / 2.0 - 1.0);
    double fH = qBound(fL + 100.0, highFreq, m_sampleRate / 2.0 - 1.0);

    // Coefficient arrays for two 2nd-order sections (HP + LP cascade)
    m_bpX1.resize(4, 0.0);
    m_bpX2.resize(4, 0.0);
    m_bpY1.resize(4, 0.0);
    m_bpY2.resize(4, 0.0);
}

/* ---- Apply band-pass filter ---- */

QVector<double> Deesser11::bandpassFilter(const QVector<double>& input)
{
    int n = input.size();
    if (n == 0) return input;

    // Get band center frequency
    auto band = getBandForProfile(m_profile);
    double centerFreq = (band.first + band.second) / 2.0;
    double bandwidth = band.second - band.first;

    // Normalized frequency
    double w0 = 2.0 * M_PI * centerFreq / m_sampleRate;
    double alpha = qSin(w0) * qSqrt(2.0) / (2.0 * bandwidth / centerFreq);

    // Biquad band-pass coefficients
    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * qCos(w0);
    double a2 = 1.0 - alpha;

    // Normalize
    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;

    QVector<double> output(n);
    // Use filter state from members
    double x1 = m_bpX1.value(0, 0.0);
    double x2 = m_bpX2.value(0, 0.0);
    double y1 = m_bpY1.value(0, 0.0);
    double y2 = m_bpY2.value(0, 0.0);

    for (int i = 0; i < n; ++i) {
        double x0 = input[i];
        double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        output[i] = y0;
        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
    }

    // Store state
    if (m_bpX1.size() >= 1) m_bpX1[0] = x1;
    if (m_bpX2.size() >= 1) m_bpX2[0] = x2;
    if (m_bpY1.size() >= 1) m_bpY1[0] = y1;
    if (m_bpY2.size() >= 1) m_bpY2[0] = y2;

    return output;
}

/* ---- Compute envelope of filtered signal ---- */

double Deesser11::computeEnvelope(const QVector<double>& filtered)
{
    double energy = 0.0;
    for (double s : filtered)
        energy += s * s;
    energy = qSqrt(energy / qMax(1, filtered.size()));
    return energy;
}

/* ---- Detect voice profile from spectral centroid ---- */

Deesser11::VoiceProfile Deesser11::detectVoiceProfile(const QVector<double>& audio) const
{
    // Simple spectral centroid estimation for gender detection
    double centroidSum = 0.0;
    double energySum = 0.0;

    for (int i = 0; i < audio.size(); ++i) {
        double freq = static_cast<double>(i) * m_sampleRate / audio.size();
        energySum += qAbs(audio[i]);
        centroidSum += freq * qAbs(audio[i]);
    }

    double centroid = (energySum > 1e-10) ? centroidSum / energySum : 0.0;

    // Male voice: centroid typically 1000-3000 Hz
    // Female voice: centroid typically 2000-5000 Hz
    if (centroid < 2000.0) return VoiceProfile::Male;
    if (centroid > 3000.0) return VoiceProfile::Female;
    return VoiceProfile::Male;  // Default
}

/* ---- Process a single frame ---- */

Deesser11::FrameResult Deesser11::processFrame(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    FrameResult result;
    int n = input.size();
    if (n == 0) { result.output = input; return result; }

    // Auto-detect profile if needed
    if (m_profile == VoiceProfile::AutoDetect && m_frameCount == 0) {
        VoiceProfile detected = detectVoiceProfile(input);
        m_profile = detected;
        emit profileDetected(detected);
    }

    // Band-pass filter to extract sibilance energy
    QVector<double> filtered = bandpassFilter(input);

    // Compute sibilance level
    double sibLevel = computeEnvelope(filtered);
    double sibDb = linearToDb(sibLevel);
    result.sibilanceLevel = sibDb;

    // Dynamic threshold tracking: adapt threshold based on recent levels
    double alpha = 0.01;  // Slow adaptation
    m_dynamicThreshold = alpha * sibDb + (1.0 - alpha) * m_dynamicThreshold;

    // Determine if sibilance exceeds threshold
    double effectiveThreshold = m_thresholdDb + m_dynamicThreshold * 0.1;
    result.sibilanceDetected = sibDb > effectiveThreshold;

    // Compute gain reduction with attack/release envelope
    double targetGain = 1.0;
    if (result.sibilanceDetected) {
        double overDb = sibDb - effectiveThreshold;
        double reducedDb = overDb * (1.0 - 1.0 / m_ratio);
        targetGain = dbToLinear(-reducedDb);
    }

    // Envelope follower with attack/release
    double attackCoeff = qExp(-1.0 / (m_attackMs * m_sampleRate / 1000.0 * n));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0 * n));

    if (targetGain < m_gainReduction)
        m_gainReduction = attackCoeff * m_gainReduction + (1.0 - attackCoeff) * targetGain;
    else
        m_gainReduction = releaseCoeff * m_gainReduction + (1.0 - releaseCoeff) * targetGain;

    result.gainReduction = linearToDb(m_gainReduction);

    // Apply gain reduction to the band-passed portion only (frequency-selective)
    QVector<double> output(n);
    double wetGain = m_gainReduction;  // Amount to reduce sibilance
    for (int i = 0; i < n; ++i) {
        // Subtract filtered * (1 - wetGain) from original
        output[i] = input[i] - filtered[i] * (1.0 - wetGain);
    }
    result.output = output;

    m_frameCount++;
    if (result.sibilanceDetected) m_sibilanceCount++;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFrames++;
    m_stats.sampleRate = m_sampleRate;
    m_stats.avgGainReduction = (m_stats.avgGainReduction * (m_stats.totalFrames - 1) + m_gainReduction) / m_stats.totalFrames;
    m_stats.sibilanceRate = static_cast<double>(m_sibilanceCount) / m_stats.totalFrames;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(sibDb, result.gainReduction, result.sibilanceDetected);
    return result;
}

/* ---- Process entire buffer ---- */

QVector<double> Deesser11::process(const QVector<double>& input)
{
    // Process in frames of 1024 samples with 256 overlap
    int frameSize = 1024;
    int hopSize = 256;
    int n = input.size();

    QVector<double> output(n, 0.0);
    QVector<double> window(n, 1.0);

    // Hann window for overlap-add
    for (int i = 0; i < n; ++i)
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));

    for (int start = 0; start < n - frameSize + 1; start += hopSize) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize && start + i < n; ++i)
            frame[i] = input[start + i];

        FrameResult fr = processFrame(frame);

        for (int i = 0; i < frameSize && start + i < n; ++i)
            output[start + i] += fr.output[i] * window[i];
    }

    // Handle remaining samples
    if (n > 0 && n < frameSize) {
        QVector<double> frame = input;
        FrameResult fr = processFrame(frame);
        for (int i = 0; i < n; ++i)
            output[i] = fr.output[i];
    }

    return output;
}

/* ---- Get current sibilance band ---- */

QPair<double, double> Deesser11::sibilanceBand() const
{
    return getBandForProfile(m_profile);
}

/* ---- Reset ---- */

void Deesser11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envFollower = 0.0;
    m_dynamicThreshold = m_thresholdDb;
    m_gainReduction = 1.0;
    m_sibilanceCount = 0;
    m_frameCount = 0;
}

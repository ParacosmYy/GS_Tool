/**
 * @file SpectralGate3.cpp
 * @brief SpectralGate3 实现
 *
 * 实现频谱门控：STFT分析、Wiener滤波增益、MMSE估计、重叠相加重建。
 */

#include "utils/dsp203/SpectralGate3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralGate3::SpectralGate3(QObject *parent) : QObject(parent) { precomputeWindow(); }
SpectralGate3::~SpectralGate3() = default;

/* ---- Configuration ---- */

void SpectralGate3::setFrameSize(int size) { m_frameSize = qMax(64, size); precomputeWindow(); }
void SpectralGate3::setHopSize(int hop) { m_hopSize = qMax(1, hop); }
void SpectralGate3::setNoiseFloor(double floor) { m_noiseFloor = qMax(1e-10, floor); }

/* ---- Precompute window ---- */

void SpectralGate3::precomputeWindow()
{
    m_window.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_frameSize - 1)));
}

/* ---- Signal power ---- */

double SpectralGate3::signalPower(const QVector<double>& signal)
{
    if (signal.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : signal) sum += s * s;
    return sum / signal.size();
}

/* ---- DFT of a real frame ---- */

void SpectralGate3::dftFrame(const QVector<double>& frame,
                                QVector<double>& magnitude, QVector<double>& phase) const
{
    int N = frame.size();
    int bins = N / 2 + 1;
    magnitude.resize(bins);
    phase.resize(bins);

    for (int k = 0; k < bins; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
        phase[k] = qAtan2(im, re);
    }
}

/* ---- Inverse DFT ---- */

QVector<double> SpectralGate3::idftFrame(const QVector<double>& magnitude,
                                            const QVector<double>& phase) const
{
    int bins = magnitude.size();
    int N = (bins - 1) * 2;
    QVector<double> frame(N, 0.0);

    for (int n = 0; n < N; ++n) {
        double re = 0.0;
        for (int k = 0; k < bins; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            re += magnitude[k] * qCos(phase[k] + angle);
        }
        // Mirror frequencies (conjugate symmetry)
        for (int k = 1; k < bins - 1; ++k) {
            double angle = -2.0 * M_PI * k * n / N;
            re += magnitude[k] * qCos(-phase[k] + angle);
        }
        frame[n] = re / N;
    }
    return frame;
}

/* ---- Estimate noise spectrum ---- */

QVector<double> SpectralGate3::estimateNoise(const QVector<double>& signal, int noiseFrames) const
{
    int bins = m_frameSize / 2 + 1;
    QVector<double> noisePower(bins, 0.0);
    int count = 0;

    for (int f = 0; f < noiseFrames; ++f) {
        int start = f * m_hopSize;
        if (start + m_frameSize > signal.size()) break;

        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = signal[start + i] * m_window[i];

        QVector<double> mag, ph;
        dftFrame(frame, mag, ph);

        for (int k = 0; k < bins; ++k)
            noisePower[k] += mag[k] * mag[k];
        count++;
    }

    if (count > 0)
        for (double& p : noisePower) p /= count;

    return noisePower;
}

/* ---- Wiener gain ---- */

QVector<double> SpectralGate3::wienerGain(const QVector<double>& signalPower,
                                            const QVector<double>& noisePower) const
{
    int bins = signalPower.size();
    QVector<double> gain(bins);

    for (int k = 0; k < bins; ++k) {
        double snr = signalPower[k] / qMax(m_noiseFloor, noisePower[k]);
        // Wiener gain: max(0, 1 - 1/SNR)
        gain[k] = qMax(0.0, 1.0 - 1.0 / qMax(snr, 1e-10));
    }
    return gain;
}

/* ---- MMSE gain ---- */

QVector<double> SpectralGate3::mmseGain(const QVector<double>& signalPower,
                                           const QVector<double>& noisePower,
                                           double snrPrior) const
{
    int bins = signalPower.size();
    QVector<double> gain(bins);
    Q_UNUSED(snrPrior)

    for (int k = 0; k < bins; ++k) {
        double xi = signalPower[k] / qMax(m_noiseFloor, noisePower[k]);
        double gamma = qMax(xi, 0.01);

        // MMSE-STSA gain approximation (Ephraim-Malah simplified)
        double v = gamma * xi / (1.0 + xi);
        if (v > 20.0) {
            gain[k] = xi / (1.0 + xi);
        } else {
            // Simplified MMSE gain: G = sqrt(v/(1+v)) * approximate correction
            double ratio = v / (1.0 + v);
            gain[k] = qSqrt(qMax(ratio, 0.0)) * qExp(-0.5 * qMax(0.0, 1.0 - v));
            gain[k] = qBound(0.0, gain[k], 1.0);
        }
    }
    return gain;
}

/* ---- Denoise ---- */

QVector<double> SpectralGate3::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < m_frameSize) return signal;

    // Estimate noise from first few frames
    int noiseFrames = qMax(1, qMin(5, n / m_hopSize));
    QVector<double> noisePower = estimateNoise(signal, noiseFrames);

    // Overlap-add synthesis
    QVector<double> output(n, 0.0);
    QVector<double> winSum(n, 0.0);

    double inPower = signalPower(signal);

    int frames = (n - m_frameSize) / m_hopSize + 1;
    for (int f = 0; f < frames; ++f) {
        int start = f * m_hopSize;

        // Window frame
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = signal[start + i] * m_window[i];

        // DFT
        QVector<double> mag, ph;
        dftFrame(frame, mag, ph);

        // Compute power spectrum
        QVector<double> power(mag.size());
        for (int k = 0; k < mag.size(); ++k)
            power[k] = mag[k] * mag[k];

        // Wiener gain
        QVector<double> gain = wienerGain(power, noisePower);

        // Apply gain to magnitude
        QVector<double> cleanMag(mag.size());
        for (int k = 0; k < mag.size(); ++k)
            cleanMag[k] = mag[k] * gain[k];

        // Inverse DFT
        QVector<double> cleanFrame = idftFrame(cleanMag, ph);

        // Overlap-add
        for (int i = 0; i < m_frameSize && start + i < n; ++i) {
            output[start + i] += cleanFrame[i] * m_window[i];
            winSum[start + i] += m_window[i] * m_window[i];
        }
    }

    // Normalize by window sum
    for (int i = 0; i < n; ++i)
        if (winSum[i] > 1e-10) output[i] /= winSum[i];

    double outPower = signalPower(output);
    m_stats.totalOps++;
    m_stats.signalLength = n;
    m_stats.inputSNR = 10.0 * qLn(qMax(inPower, 1e-10) / qMax(m_noiseFloor, 1e-10)) / qLn(10.0);
    m_stats.outputSNR = 10.0 * qLn(qMax(outPower, 1e-10) / qMax(m_noiseFloor, 1e-10)) / qLn(10.0);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoisingCompleted(n, m_stats.inputSNR, m_stats.outputSNR, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void SpectralGate3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

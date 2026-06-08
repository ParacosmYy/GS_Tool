/**
 * @file SpectralGate5.cpp
 * @brief SpectralGate5 实现
 *
 * 实现谱门控：MLP噪声轮廓估计、Wiener抑制增益映射、谱域降噪。
 */

#include "utils/dsp223/SpectralGate5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

SpectralGate5::SpectralGate5(QObject *parent) : QObject(parent) { initNeuralWeights(); }
SpectralGate5::~SpectralGate5() = default;

/* ---- Configuration ---- */

void SpectralGate5::setParameters(int frameSize, double thresholdDb,
                                    double reductionDb, int hiddenDim)
{
    m_frameSize = qMax(64, frameSize);
    m_fftSize = m_frameSize * 2;
    m_thresholdDb = thresholdDb;
    m_reductionDb = reductionDb;
    m_hiddenDim = qMax(8, hiddenDim);
    initNeuralWeights();
}

/* ---- Initialize neural weights ---- */

void SpectralGate5::initNeuralWeights()
{
    std::mt19937 rng(42);
    std::normal_distribution<double> dist(0.0, 0.1);
    int halfFft = m_fftSize / 2 + 1;

    m_w1.resize(m_hiddenDim);
    m_b1.resize(m_hiddenDim, 0.0);
    for (int h = 0; h < m_hiddenDim; ++h) {
        m_w1[h].resize(halfFft);
        for (int i = 0; i < halfFft; ++i)
            m_w1[h][i] = dist(rng);
    }
    m_w2.resize(m_hiddenDim);
    for (int h = 0; h < m_hiddenDim; ++h)
        m_w2[h] = dist(rng);
}

/* ---- Neural forward pass ---- */

QVector<double> SpectralGate5::neuralForward(const QVector<double>& input) const
{
    // Hidden layer with ReLU
    QVector<double> hidden(m_hiddenDim, 0.0);
    for (int h = 0; h < m_hiddenDim; ++h) {
        double sum = m_b1[h];
        for (int i = 0; i < qMin(input.size(), m_w1[h].size()); ++i)
            sum += input[i] * m_w1[h][i];
        hidden[h] = qMax(0.0, sum); // ReLU
    }
    // Output layer (linear)
    double output = m_b2;
    for (int h = 0; h < m_hiddenDim; ++h)
        output += hidden[h] * m_w2[h];
    return {output};
}

/* ---- Train neural step ---- */

void SpectralGate5::trainNeuralStep(const QVector<double>& mag, double targetFloor)
{
    // Simplified: adjust weights toward target noise floor
    auto pred = neuralForward(mag);
    double error = pred[0] - targetFloor;
    double lr = 0.001;

    // Update output weights
    for (int h = 0; h < m_hiddenDim; ++h)
        m_w2[h] -= lr * error * m_w2[h];
    m_b2 -= lr * error;
}

/* ---- Estimate noise ---- */

void SpectralGate5::estimateNoise(const QVector<double>& noiseFrames)
{
    if (noiseFrames.isEmpty()) return;
    int halfFft = m_fftSize / 2 + 1;

    // Average magnitude spectrum over noise frames
    m_noiseMag.resize(halfFft, 0.0);
    int numFrames = noiseFrames.size() / m_frameSize;
    m_stats.noiseEstimationFrames = numFrames;

    for (int f = 0; f < numFrames; ++f) {
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize && f * m_frameSize + i < noiseFrames.size(); ++i)
            frame[i] = noiseFrames[f * m_frameSize + i];

        auto window = hannWindow(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] *= window[i];

        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);
        for (int i = 0; i < m_frameSize; ++i) re[i] = frame[i];
        fft(re, im);

        for (int k = 0; k < halfFft; ++k)
            m_noiseMag[k] += qSqrt(re[k] * re[k] + im[k] * im[k]);
    }

    for (int k = 0; k < halfFft; ++k) {
        m_noiseMag[k] /= qMax(1, numFrames);
        m_noiseMag[k] = qMax(m_noiseMag[k], 1e-10);
    }

    // Neural refinement of noise profile
    trainNeuralStep(m_noiseMag, m_thresholdDb);

    m_prevGain.resize(halfFft, 1.0);
}

/* ---- FFT (radix-2) ---- */

void SpectralGate5::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    if (n <= 1) return;
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]); std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = curRe * re[i+j+len/2] - curIm * im[i+j+len/2];
                double tIm = curRe * im[i+j+len/2] + curIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

/* ---- IFFT ---- */

void SpectralGate5::ifft(QVector<double>& re, QVector<double>& im) const
{
    for (auto& v : im) v = -v;
    fft(re, im);
    double n = re.size();
    for (int i = 0; i < (int)n; ++i) { re[i] /= n; im[i] = -im[i] / n; }
}

/* ---- Hann window ---- */

QVector<double> SpectralGate5::hannWindow(int size) const
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
    return w;
}

/* ---- Wiener suppression gain ---- */

QVector<double> SpectralGate5::wienerGain(const QVector<double>& signalMag,
                                             const QVector<double>& noiseMag) const
{
    int n = qMin(signalMag.size(), noiseMag.size());
    QVector<double> gain(n, 0.0);
    double reductionLin = qPow(10.0, m_reductionDb / 20.0);

    for (int k = 0; k < n; ++k) {
        double sigPow = signalMag[k] * signalMag[k];
        double noisePow = noiseMag[k] * noiseMag[k];
        // Wiener gain: max(SNR / (SNR + 1), floor)
        double snr = sigPow / qMax(noisePow, 1e-20);
        double wGain = snr / (snr + 1.0);
        // Spectral gate: if below threshold, apply reduction
        double sigDb = 10.0 * qLn10 ? qLn(signalMag[k] + 1e-20) / qLn(10.0) * 10.0 : -100.0;
        if (sigDb < m_thresholdDb)
            wGain = qMax(wGain, reductionLin);
        gain[k] = qBound(reductionLin, wGain, 1.0);
    }
    return gain;
}

/* ---- Process single frame ---- */

QVector<double> SpectralGate5::process(const QVector<double>& frame)
{
    if (m_noiseMag.isEmpty()) return frame;
    int halfFft = m_fftSize / 2 + 1;

    // Window
    auto window = hannWindow(qMin(frame.size(), m_frameSize));
    QVector<double> windowed(m_fftSize, 0.0);
    for (int i = 0; i < qMin(frame.size(), m_frameSize); ++i)
        windowed[i] = frame[i] * window[i];

    // FFT
    QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);
    for (int i = 0; i < m_fftSize; ++i) re[i] = windowed[i];
    fft(re, im);

    // Magnitude spectrum
    QVector<double> mag(halfFft);
    for (int k = 0; k < halfFft; ++k)
        mag[k] = qSqrt(re[k] * re[k] + im[k] * im[k]);

    // Neural noise refinement
    auto neuralOut = neuralForward(mag);

    // Compute Wiener gain
    auto gain = wienerGain(mag, m_noiseMag);

    // Smooth gain with previous frame
    for (int k = 0; k < qMin(gain.size(), m_prevGain.size()); ++k) {
        gain[k] = 0.7 * m_prevGain[k] + 0.3 * gain[k];
        m_prevGain[k] = gain[k];
    }

    // Apply gain
    for (int k = 0; k < halfFft; ++k) {
        re[k] *= gain[k]; im[k] *= gain[k];
        if (k > 0 && k < m_fftSize / 2) {
            re[m_fftSize - k] *= gain[k]; im[m_fftSize - k] *= gain[k];
        }
    }

    // IFFT
    ifft(re, im);
    QVector<double> output(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        output[i] = re[i] * window[i]; // Overlap-add window
    return output;
}

/* ---- Process full signal ---- */

QVector<double> SpectralGate5::processSignal(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(signal.size(), 0.0);
    int hop = m_frameSize / 2;
    int numFrames = signal.size() / hop;

    for (int f = 0; f < numFrames; ++f) {
        QVector<double> frame(m_frameSize, 0.0);
        for (int i = 0; i < m_frameSize && f * hop + i < signal.size(); ++i)
            frame[i] = signal[f * hop + i];

        auto gated = process(frame);
        for (int i = 0; i < m_frameSize && f * hop + i < output.size(); ++i)
            output[f * hop + i] += gated[i];
    }

    m_stats.numFrames += numFrames;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(numFrames, 0.0, timer.elapsed());
    return output;
}

/* ---- Noise profile ---- */

QVector<double> SpectralGate5::noiseProfile() const { return m_noiseMag; }

/* ---- Reset ---- */

void SpectralGate5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_noiseMag.clear();
    m_prevGain.clear();
    m_w1.clear();
    m_b1.clear();
    m_w2.clear();
    initNeuralWeights();
}

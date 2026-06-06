/**
 * @file HarmonicAnalyzer.cpp
 * @brief HarmonicAnalyzer 实现
 *
 * 实现谐波分析：加窗FFT、峰值提取、基频检测、泛音跟踪、THD计算。
 */

#include "utils/signal168/HarmonicAnalyzer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

HarmonicAnalyzer::HarmonicAnalyzer(QObject* parent)
    : QObject(parent)
{
}

HarmonicAnalyzer::~HarmonicAnalyzer() = default;

void HarmonicAnalyzer::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

void HarmonicAnalyzer::setFFTSize(int size)
{
    m_fftSize = qMax(64, size);
    /* Ensure power of 2 */
    int p = 1;
    while (p < m_fftSize) p <<= 1;
    m_fftSize = p;
}

void HarmonicAnalyzer::setMaxHarmonics(int maxH)
{
    m_maxHarmonics = qMax(1, maxH);
}

int HarmonicAnalyzer::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

QVector<double> HarmonicAnalyzer::hanningWindow(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
    return w;
}

void HarmonicAnalyzer::fft(QVector<double>& real, QVector<double>& imag, bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;

    /* Bit-reversal */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nr;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

QVector<int> HarmonicAnalyzer::findPeaks(const QVector<double>& magnitude,
                                          double threshold) const
{
    QVector<int> peaks;
    int n = magnitude.size();

    for (int i = 2; i < n - 2; ++i) {
        if (magnitude[i] > threshold &&
            magnitude[i] > magnitude[i - 1] &&
            magnitude[i] > magnitude[i + 1] &&
            magnitude[i] > magnitude[i - 2] &&
            magnitude[i] > magnitude[i + 2]) {
            peaks.append(i);
        }
    }

    /* Sort by magnitude descending */
    std::sort(peaks.begin(), peaks.end(), [&](int a, int b) {
        return magnitude[a] > magnitude[b];
    });

    return peaks;
}

double HarmonicAnalyzer::interpolatePeak(const QVector<double>& magnitude, int bin) const
{
    if (bin <= 0 || bin >= magnitude.size() - 1) return bin;
    double alpha = magnitude[bin - 1];
    double beta = magnitude[bin];
    double gamma = magnitude[bin + 1];
    double denom = alpha - 2.0 * beta + gamma;
    if (qFuzzyIsNull(denom)) return bin;
    return bin + 0.5 * (alpha - gamma) / denom;
}

HarmonicAnalyzer::AnalysisResult HarmonicAnalyzer::analyze(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    AnalysisResult result;
    result.fundamentalFreq = 0.0;
    result.fundamentalAmp = 0.0;
    result.thd = 0.0;
    result.thdPlusNoise = 0.0;

    if (signal.isEmpty()) return result;

    int fftLen = nextPowerOf2(qMax(m_fftSize, signal.size()));

    /* Apply Hanning window */
    QVector<double> window = hanningWindow(signal.size());
    QVector<double> real(fftLen, 0.0), imag(fftLen, 0.0);
    for (int i = 0; i < signal.size(); ++i)
        real[i] = signal[i] * window[i];

    /* Forward FFT */
    fft(real, imag, false);

    /* Compute magnitude spectrum */
    int halfLen = fftLen / 2;
    QVector<double> magnitude(halfLen);
    for (int i = 0; i < halfLen; ++i)
        magnitude[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);

    /* Find peaks */
    double maxMag = *std::max_element(magnitude.begin(), magnitude.end());
    double threshold = maxMag * 0.01; /* -40dB threshold */
    QVector<int> peaks = findPeaks(magnitude, threshold);

    if (peaks.isEmpty()) {
        m_timeSum += timer.elapsed();
        emit analysisCompleted(0.0);
        return result;
    }

    /* Detect fundamental (strongest peak) */
    int fundBin = peaks[0];
    double fundFreq = interpolatePeak(magnitude, fundBin) * m_sampleRate / fftLen;

    result.fundamentalFreq = fundFreq;
    result.fundamentalAmp = magnitude[fundBin];

    /* Track harmonics: find peaks near integer multiples of fundamental */
    double binResolution = m_sampleRate / fftLen;
    double fundBinExact = fundFreq / binResolution;

    for (int h = 1; h <= m_maxHarmonics; ++h) {
        double targetBin = fundBinExact * h;
        if (targetBin >= halfLen) break;

        /* Search within +/-3 bins of expected position */
        int searchLo = qMax(1, static_cast<int>(targetBin) - 3);
        int searchHi = qMin(halfLen - 1, static_cast<int>(targetBin) + 3);

        int bestBin = searchLo;
        double bestMag = 0.0;
        for (int b = searchLo; b <= searchHi; ++b) {
            if (magnitude[b] > bestMag) {
                bestMag = magnitude[b];
                bestBin = b;
            }
        }

        Harmonic harm;
        harm.binIndex = bestBin;
        harm.frequency = interpolatePeak(magnitude, bestBin) * binResolution;
        harm.amplitude = bestMag;
        /* Phase from complex spectrum */
        harm.phase = qAtan2(imag[bestBin], real[bestBin]);
        result.harmonics.append(harm);
    }

    /* Compute THD: sqrt(sum of harmonic powers) / fundamental power */
    double harmonicPower = 0.0;
    for (int i = 1; i < result.harmonics.size(); ++i) {
        double a = result.harmonics[i].amplitude;
        harmonicPower += a * a;
    }
    double fundPower = result.fundamentalAmp * result.fundamentalAmp;
    result.thd = (fundPower > 0) ? qSqrt(harmonicPower / fundPower) : 0.0;

    /* THD+N: total non-fundamental power / fundamental */
    double totalPower = 0.0;
    for (int i = 1; i < halfLen; ++i)
        totalPower += magnitude[i] * magnitude[i];
    result.thdPlusNoise = (fundPower > 0)
        ? qSqrt((totalPower - fundPower) / fundPower) : 0.0;

    m_stats.totalAnalyses++;
    m_stats.lastFundamental = fundFreq;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalAnalyses > 0)
        ? m_timeSum / m_stats.totalAnalyses : 0.0;

    emit analysisCompleted(fundFreq);
    return result;
}

void HarmonicAnalyzer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

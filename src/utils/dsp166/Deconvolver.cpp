/**
 * @file Deconvolver.cpp
 * @brief Deconvolver 实现
 *
 * 实现维纳反卷积：频域正则化逆滤波，FFT/IFFT处理。
 */

#include "utils/dsp166/Deconvolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

Deconvolver::Deconvolver(QObject* parent)
    : QObject(parent)
{
}

Deconvolver::~Deconvolver() = default;

void Deconvolver::setRegularization(double lambda)
{
    m_lambda = qMax(1e-12, lambda);
}

void Deconvolver::setNoisePower(double power)
{
    m_noisePower = qMax(1e-20, power);
}

int Deconvolver::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

void Deconvolver::fft(QVector<double>& real, QVector<double>& imag, bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;

    /* Bit-reversal permutation */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* Cooley-Tukey iterative FFT */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        double wReal = qCos(angle);
        double wImag = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newReal = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
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

double Deconvolver::estimateNoise(const QVector<double>& signal) const
{
    if (signal.size() < 2) return 0.0;
    /* Estimate noise variance from high-frequency differences */
    double sum = 0.0;
    for (int i = 1; i < signal.size(); ++i) {
        double diff = signal[i] - signal[i - 1];
        sum += diff * diff;
    }
    return sum / (2.0 * (signal.size() - 1));
}

QVector<double> Deconvolver::deconvolve(const QVector<double>& signal,
                                         const QVector<double>& psf)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || psf.isEmpty()) return QVector<double>();

    int sigLen = signal.size();
    int psfLen = psf.size();
    int fftLen = nextPowerOf2(sigLen + psfLen - 1);

    /* Zero-pad signal */
    QVector<double> sigReal(fftLen, 0.0), sigImag(fftLen, 0.0);
    for (int i = 0; i < sigLen; ++i) sigReal[i] = signal[i];

    /* Zero-pad PSF */
    QVector<double> psfReal(fftLen, 0.0), psfImag(fftLen, 0.0);
    for (int i = 0; i < psfLen; ++i) psfReal[i] = psf[i];

    /* Forward FFT both */
    fft(sigReal, sigImag, false);
    fft(psfReal, psfImag, false);

    /* Estimate noise power spectrum */
    double noiseEst = m_noisePower;
    if (noiseEst <= 0) noiseEst = estimateNoise(signal);
    double signalPower = 0.0;
    for (int i = 0; i < sigLen; ++i) signalPower += signal[i] * signal[i];
    signalPower /= sigLen;
    double snr = (noiseEst > 0) ? signalPower / noiseEst : 1e10;
    m_stats.lastSNR = snr;

    /* Wiener filter: H_w = H* / (|H|^2 + 1/SNR + lambda)
       where H is PSF spectrum, H* is conjugate */
    QVector<double> outReal(fftLen, 0.0), outImag(fftLen, 0.0);
    double regFactor = m_lambda + 1.0 / qMax(1e-20, snr);

    for (int i = 0; i < fftLen; ++i) {
        double hReal = psfReal[i];
        double hImag = psfImag[i];
        double hMag2 = hReal * hReal + hImag * hImag;
        double denom = hMag2 + regFactor;
        if (denom < 1e-20) denom = 1e-20;

        /* H* = conj(H) */
        double conjReal = hReal;
        double conjImag = -hImag;

        /* Wiener filter output = S * H* / (|H|^2 + reg) */
        double sReal = sigReal[i];
        double sImag = sigImag[i];

        /* Complex multiply S * conj(H) */
        double mulReal = sReal * conjReal - sImag * conjImag;
        double mulImag = sReal * conjImag + sImag * conjReal;

        outReal[i] = mulReal / denom;
        outImag[i] = mulImag / denom;
    }

    /* Inverse FFT */
    fft(outReal, outImag, true);

    /* Extract result (truncate to original signal length) */
    QVector<double> result(sigLen);
    for (int i = 0; i < sigLen; ++i)
        result[i] = outReal[i];

    m_stats.totalRuns++;
    m_stats.lastSignalLength = sigLen;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit deconvolutionCompleted(sigLen);
    return result;
}

void Deconvolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

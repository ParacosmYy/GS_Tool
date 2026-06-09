/**
 * @file Periodogram6.cpp
 * @brief Periodogram6 实现
 *
 * 实现周期图：Slepian锥形多窗估计与自适应加权。
 */

#include "utils/signal249/Periodogram6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Periodogram6::Periodogram6(QObject *parent) : QObject(parent) {}
Periodogram6::~Periodogram6() = default;

/* ---- Configuration ---- */

void Periodogram6::setFftSize(int size) { m_fftSize = qMax(0, size); }
void Periodogram6::setNumTapers(int num) { m_numTapers = qMax(1, num); }
void Periodogram6::setBandwidth(double nw) { m_nw = qMax(0.5, nw); }

/* ---- DFT for tapered signal ---- */

QVector<Periodogram6::Complex> Periodogram6::dft(
    const QVector<double>& input, int fftSize) const
{
    int N = fftSize;
    QVector<Complex> spectrum(N / 2 + 1, {0.0, 0.0});
    for (int k = 0; k <= N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < input.size(); ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += input[n] * qCos(angle);
            im += input[n] * qSin(angle);
        }
        spectrum[k] = {re, im};
    }
    return spectrum;
}

/* ---- Power spectrum magnitude squared ---- */

QVector<double> Periodogram6::powerSpectrum(const QVector<Complex>& spectrum) const
{
    QVector<double> psd(spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i) {
        double re = spectrum[i].first, im = spectrum[i].second;
        psd[i] = re * re + im * im;
    }
    return psd;
}

/* ---- Tridiagonal eigenvector (inverse iteration) ---- */

QVector<double> Periodogram6::tridiagEigenvector(
    const QVector<double>& diag, const QVector<double>& offDiag,
    int index) const
{
    int N = diag.size();
    QVector<double> v(N, 0.0);
    v[index % N] = 1.0;

    // Approximate eigenvalue using Gershgorin bounds
    double lambda = diag[index];
    for (int iter = 0; iter < 30; ++iter) {
        // Solve (T - lambda*I) * x = v via Thomas algorithm
        QVector<double> x(N);
        QVector<double> cp(N, 0.0), dp(N, 0.0);

        // Forward sweep
        cp[0] = offDiag[0] / (diag[0] - lambda);
        dp[0] = v[0] / (diag[0] - lambda);
        for (int i = 1; i < N; ++i) {
            double denom = (diag[i] - lambda) - offDiag[qMin(i - 1, offDiag.size() - 1)] * cp[i - 1];
            if (qAbs(denom) < 1e-30) denom = 1e-30;
            if (i < N - 1)
                cp[i] = (i < offDiag.size() ? offDiag[i] : 0.0) / denom;
            dp[i] = (v[i] - (i > 0 && (i - 1) < offDiag.size() ?
                     offDiag[i - 1] * dp[i - 1] : 0.0)) / denom;
        }

        // Back substitution
        x[N - 1] = dp[N - 1];
        for (int i = N - 2; i >= 0; --i)
            x[i] = dp[i] - cp[i] * x[i + 1];

        // Normalize
        double nrm = 0.0;
        for (int i = 0; i < N; ++i) nrm += x[i] * x[i];
        nrm = qSqrt(nrm);
        if (nrm < 1e-30) break;
        for (int i = 0; i < N; ++i) v[i] = x[i] / nrm;

        // Update eigenvalue estimate (Rayleigh quotient)
        double rq = 0.0;
        for (int i = 0; i < N; ++i)
            rq += v[i] * diag[i] * v[i];
        for (int i = 0; i < offDiag.size() && i < N - 1; ++i)
            rq += 2.0 * v[i] * offDiag[i] * v[i + 1];
        lambda = rq;
    }
    return v;
}

/* ---- Compute Slepian (DPSS) tapers ---- */

QVector<QVector<double>> Periodogram6::computeSlepianTapers(
    int N, int K, double NW) const
{
    // Build tridiagonal concentration matrix
    QVector<double> diag(N), offDiag(N - 1);
    for (int i = 0; i < N; ++i) {
        double n = i - (N - 1) / 2.0;
        diag[i] = n * n * 1.0 / N;
    }
    for (int i = 0; i < N - 1; ++i) {
        double n = (i + 1 - (N - 1) / 2.0);
        offDiag[i] = n * NW / N;
    }

    // Extract K eigenvectors (the ones with largest eigenvalues)
    QVector<QVector<double>> tapers;
    for (int k = 0; k < qMin(K, N); ++k) {
        auto taper = tridiagEigenvector(diag, offDiag, k);
        // Normalize
        double nrm = 0.0;
        for (double v : taper) nrm += v * v;
        nrm = qSqrt(nrm);
        if (nrm > 1e-30) {
            for (int i = 0; i < N; ++i) taper[i] /= nrm;
        }
        tapers.append(taper);
    }
    return tapers;
}

/* ---- Frequency axis ---- */

QVector<double> Periodogram6::frequencies(int signalLength) const
{
    int N = m_fftSize > 0 ? m_fftSize : signalLength;
    // Next power of 2
    int N2 = 1;
    while (N2 < N) N2 <<= 1;

    int halfN = N2 / 2 + 1;
    QVector<double> freqs(halfN);
    for (int k = 0; k < halfN; ++k)
        freqs[k] = static_cast<double>(k) / N2;
    return freqs;
}

/* ---- Multitaper periodogram estimation ---- */

QVector<double> Periodogram6::estimate(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    if (N == 0) return {};

    // Determine FFT size
    int fftSz = m_fftSize;
    if (fftSz <= 0) {
        fftSz = 1;
        while (fftSz < N) fftSz <<= 1;
    }

    // Compute Slepian tapers
    int K = qMin(m_numTapers, N);
    auto tapers = computeSlepianTapers(N, K, m_nw);

    // Compute periodogram for each taper
    int halfN = fftSz / 2 + 1;
    QVector<double> avgPsd(halfN, 0.0);
    QVector<double> weights(K, 1.0);  // Adaptive weights start as 1

    for (int k = 0; k < tapers.size(); ++k) {
        // Apply taper
        QVector<double> tapered(N);
        for (int i = 0; i < N; ++i)
            tapered[i] = signal[i] * tapers[k][i];

        // Compute DFT
        auto spectrum = dft(tapered, fftSz);
        auto psd = powerSpectrum(spectrum);

        // Accumulate weighted
        for (int i = 0; i < halfN && i < psd.size(); ++i)
            avgPsd[i] += weights[k] * psd[i];
    }

    // Adaptive weighting: re-weight by inverse variance estimate
    if (K > 1) {
        // Compute variance of multi-taper estimates
        QVector<double> variance(halfN, 0.0);
        QVector<double> mean(halfN, 0.0);
        for (int k = 0; k < tapers.size(); ++k) {
            QVector<double> tapered(N);
            for (int i = 0; i < N; ++i)
                tapered[i] = signal[i] * tapers[k][i];
            auto spectrum = dft(tapered, fftSz);
            auto psd = powerSpectrum(spectrum);
            for (int i = 0; i < halfN && i < psd.size(); ++i)
                mean[i] += psd[i];
        }
        for (int i = 0; i < halfN; ++i) mean[i] /= K;

        // Recompute with adaptive weights
        avgPsd.fill(0.0);
        double totalWeight = 0.0;
        for (int k = 0; k < tapers.size(); ++k) {
            QVector<double> tapered(N);
            for (int i = 0; i < N; ++i)
                tapered[i] = signal[i] * tapers[k][i];
            auto spectrum = dft(tapered, fftSz);
            auto psd = powerSpectrum(spectrum);

            double wk = 0.0;
            for (int i = 0; i < halfN && i < psd.size(); ++i)
                wk += psd[i];
            wk = (wk > 1e-30) ? 1.0 / wk : 1.0;

            for (int i = 0; i < halfN && i < psd.size(); ++i)
                avgPsd[i] += wk * psd[i];
            totalWeight += wk;
        }

        if (totalWeight > 1e-30) {
            for (int i = 0; i < halfN; ++i)
                avgPsd[i] /= totalWeight;
        }
    } else {
        for (int i = 0; i < halfN; ++i)
            avgPsd[i] /= K;
    }

    m_stats.inputLength = N;
    m_stats.fftSize = fftSz;
    m_stats.numTapers = K;
    m_stats.bandwidth = m_nw;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit estimationCompleted(fftSz, K, timer.elapsed());
    return avgPsd;
}

/* ---- Reset ---- */

void Periodogram6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

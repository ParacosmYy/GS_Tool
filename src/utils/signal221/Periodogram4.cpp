/**
 * @file Periodogram4.cpp
 * @brief Periodogram4 实现
 *
 * 实现多窗Thomson谱估计：DPSS窗生成、自适应加权、谱泄漏抑制。
 */

#include "utils/signal221/Periodogram4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Periodogram4::Periodogram4(QObject *parent) : QObject(parent)
{
    computeTapers();
}

Periodogram4::~Periodogram4() = default;

/* ---- Configuration ---- */

void Periodogram4::setParameters(int fftSize, int numTapers, double nw)
{
    m_fftSize = qMax(16, fftSize);
    m_numTapers = qMax(1, numTapers);
    m_nw = qMax(0.5, nw);
    m_stats.fftSize = m_fftSize;
    m_stats.numTapers = m_numTapers;
    m_stats.bandwidth = m_nw / m_fftSize;
    computeTapers();
}

/* ---- Tridiagonal eigensolver for DPSS ---- */

QVector<QVector<double>> Periodogram4::tridiagEigen(int n, double nw) const
{
    // Build tridiagonal matrix for DPSS eigenproblem
    double w = nw / n;
    QVector<double> mainDiag(n), offDiag(n - 1);

    for (int i = 0; i < n; ++i)
        mainDiag[i] = 0.5 * qCos(2.0 * M_PI * w) * (n - 1.0);
    for (int i = 0; i < n - 1; ++i)
        offDiag[i] = 0.5 * qSqrt(double((n - 1 - i) * (i + 1)));

    // QR iteration to find eigenvectors of tridiagonal matrix
    // Initialize eigenvectors as identity
    QVector<QVector<double>> vecs(m_numTapers, QVector<double>(n, 0.0));

    // Use power iteration with deflation for top eigenvectors
    QVector<bool> found(n, false);
    for (int k = 0; k < m_numTapers; ++k) {
        // Initialize with sinusoidal guess
        QVector<double> v(n);
        for (int i = 0; i < n; ++i)
            v[i] = qSin(M_PI * (k + 1) * (i + 0.5) / n);

        // Power iteration on tridiagonal matrix
        for (int iter = 0; iter < 200; ++iter) {
            // Multiply by tridiagonal matrix
            QVector<double> w(n, 0.0);
            for (int i = 0; i < n; ++i) {
                w[i] = mainDiag[i] * v[i];
                if (i > 0) w[i] += offDiag[i - 1] * v[i - 1];
                if (i < n - 1) w[i] += offDiag[i] * v[i + 1];
            }

            // Orthogonalize against previous eigenvectors
            for (int p = 0; p < k; ++p) {
                double d = 0.0;
                for (int i = 0; i < n; ++i) d += w[i] * vecs[p][i];
                for (int i = 0; i < n; ++i) w[i] -= d * vecs[p][i];
            }

            // Normalize
            double nrm = 0.0;
            for (int i = 0; i < n; ++i) nrm += w[i] * w[i];
            nrm = qSqrt(nrm);
            if (nrm < 1e-30) break;
            for (int i = 0; i < n; ++i) v[i] = w[i] / nrm;
        }
        vecs[k] = v;
    }
    return vecs;
}

/* ---- Compute DPSS tapers ---- */

void Periodogram4::computeTapers()
{
    m_tapers = tridiagEigen(m_fftSize, m_nw);
    m_taperEigenvalues.resize(m_tapers.size());

    // Compute eigenvalues (concentration ratios)
    for (int k = 0; k < m_tapers.size(); ++k) {
        double sumSq = 0.0;
        for (int i = 0; i < m_tapers[k].size(); ++i)
            sumSq += m_tapers[k][i] * m_tapers[k][i];
        m_taperEigenvalues[k] = sumSq > 0.0 ? 1.0 - 1.0 / sumSq : 0.0;
    }
}

/* ---- DFT at all frequency bins ---- */

QVector<double> Periodogram4::dftBin(const QVector<double>& signal,
                                       int fftSize) const
{
    int n = qMin(signal.size(), fftSize);
    int halfN = fftSize / 2 + 1;
    QVector<double> psd(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / fftSize;
            re += signal[i] * qCos(angle);
            im += signal[i] * qSin(angle);
        }
        psd[k] = (re * re + im * im) / fftSize;
    }
    return psd;
}

/* ---- Adaptive weights ---- */

QVector<QVector<double>> Periodogram4::adaptiveWeights(
    const QVector<QVector<double>>& eigCoeffs,
    const QVector<double>& eigenvalues) const
{
    int numTapers = eigCoeffs.size();
    int halfN = eigCoeffs.isEmpty() ? 0 : eigCoeffs[0].size();

    QVector<QVector<double>> weights(numTapers, QVector<double>(halfN, 0.0));

    for (int k = 0; k < halfN; ++k) {
        // Estimate signal power at this frequency
        double S = 0.0;
        for (int t = 0; t < numTapers; ++t)
            S += eigCoeffs[t][k];
        S /= numTapers;

        // Adaptive weight: wk = (lambda_k * S) / (lambda_k * S + (1 - lambda_k) * noise)
        double noiseFloor = 1e-10;
        for (int t = 0; t < numTapers; ++t) {
            double lam = (t < eigenvalues.size()) ? eigenvalues[t] : 0.99;
            double denom = lam * S + (1.0 - lam) * qMax(S * 0.01, noiseFloor);
            weights[t][k] = (denom > 0.0) ? lam * S / denom : 0.0;
        }

        // Normalize weights
        double wSum = 0.0;
        for (int t = 0; t < numTapers; ++t) wSum += weights[t][k] * weights[t][k];
        if (wSum > 0.0)
            for (int t = 0; t < numTapers; ++t) weights[t][k] /= qSqrt(wSum);
    }
    return weights;
}

/* ---- Estimate ---- */

QVector<double> Periodogram4::estimate(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int halfN = m_fftSize / 2 + 1;
    m_stats.signalLength = n;

    // Compute eigencoefficients: DFT of tapered signal for each taper
    QVector<QVector<double>> eigCoeffs(m_numTapers);
    for (int t = 0; t < m_numTapers; ++t) {
        QVector<double> tapered(n);
        for (int i = 0; i < n; ++i)
            tapered[i] = signal[i] * m_tapers[t][i % m_tapers[t].size()];
        eigCoeffs[t] = dftBin(tapered, m_fftSize);
    }

    // Compute adaptive weights
    QVector<QVector<double>> weights = adaptiveWeights(eigCoeffs, m_taperEigenvalues);

    // Combine weighted eigencoefficients
    QVector<double> psd(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        double weightedSum = 0.0;
        double weightSqSum = 0.0;
        for (int t = 0; t < m_numTapers; ++t) {
            weightedSum += weights[t][k] * eigCoeffs[t][k];
            weightSqSum += weights[t][k] * weights[t][k];
        }
        psd[k] = (weightSqSum > 0.0) ? weightedSum / weightSqSum : 0.0;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit estimationCompleted(m_fftSize, m_numTapers, timer.elapsed());
    return psd;
}

/* ---- Tapers accessor ---- */

QVector<QVector<double>> Periodogram4::tapers() const { return m_tapers; }

/* ---- Reset ---- */

void Periodogram4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

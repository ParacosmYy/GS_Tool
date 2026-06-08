/**
 * @file SignalSynchronizer5.cpp
 * @brief SignalSynchronizer5 实现
 *
 * 实现GCC-PHAT互相关TDOA估计、MUSIC伪谱多源定位、信号同步对齐。
 */

#include "utils/signal227/SignalSynchronizer5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalSynchronizer5::SignalSynchronizer5(QObject *parent) : QObject(parent) {}
SignalSynchronizer5::~SignalSynchronizer5() = default;

/* ---- Configuration ---- */

void SignalSynchronizer5::setParameters(double sampleRate, int fftSize, int maxSources)
{
    m_sampleRate = qMax(8000.0, sampleRate);
    m_fftSize = qMax(64, fftSize);
    m_maxSources = qMax(1, maxSources);
    m_stats.sampleRate = m_sampleRate;
    m_stats.fftSize = m_fftSize;
    m_stats.numSources = m_maxSources;
}

/* ---- FFT (radix-2 DIT) ---- */

void SignalSynchronizer5::computeFFT(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    // Bit reversal
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < len / 2; ++j) {
                double wRe = qCos(angle * j);
                double wIm = qSin(angle * j);
                double uRe = re[i + j];
                double uIm = im[i + j];
                double vRe = re[i + j + len / 2] * wRe - im[i + j + len / 2] * wIm;
                double vIm = re[i + j + len / 2] * wIm + im[i + j + len / 2] * wRe;
                re[i + j] = uRe + vRe;
                im[i + j] = uIm + vIm;
                re[i + j + len / 2] = uRe - vRe;
                im[i + j + len / 2] = uIm - vIm;
            }
        }
    }
}

/* ---- IFFT ---- */

void SignalSynchronizer5::computeIFFT(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    computeFFT(re, im);
    for (int i = 0; i < n; ++i) {
        re[i] /= n;
        im[i] = -im[i] / n;
    }
}

/* ---- Cross-power with PHAT weighting ---- */

void SignalSynchronizer5::crossPowerPHAT(const QVector<double>& refRe,
    const QVector<double>& refIm, const QVector<double>& testRe,
    const QVector<double>& testIm, QVector<double>& outRe,
    QVector<double>& outIm) const
{
    int n = refRe.size();
    outRe.resize(n);
    outIm.resize(n);
    for (int k = 0; k < n; ++k) {
        double crossRe = refRe[k] * testRe[k] + refIm[k] * testIm[k];
        double crossIm = refIm[k] * testRe[k] - refRe[k] * testIm[k];
        double mag = qSqrt(crossRe * crossRe + crossIm * crossIm);
        if (mag > 1e-10) {
            outRe[k] = crossRe / mag;
            outIm[k] = crossIm / mag;
        } else {
            outRe[k] = 0.0;
            outIm[k] = 0.0;
        }
    }
}

/* ---- Parabolic interpolation ---- */

double SignalSynchronizer5::parabolicInterpolation(const QVector<double>& corr,
                                                     int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= corr.size() - 1) return peakIdx;
    double a = corr[peakIdx - 1];
    double b = corr[peakIdx];
    double c = corr[peakIdx + 1];
    double denom = 2.0 * (2.0 * b - a - c);
    if (qAbs(denom) < 1e-15) return peakIdx;
    return peakIdx + (a - c) / denom;
}

/* ---- GCC-PHAT TDOA estimation ---- */

SignalSynchronizer5::TDOAResult SignalSynchronizer5::estimateGCC(
    const QVector<double>& ref, const QVector<double>& test) const
{
    TDOAResult result;
    int n = qMax(m_fftSize, qMax(ref.size(), test.size()));

    // Zero-pad to FFT size
    QVector<double> refRe(n, 0.0), refIm(n, 0.0);
    QVector<double> testRe(n, 0.0), testIm(n, 0.0);
    for (int i = 0; i < qMin(ref.size(), n); ++i) refRe[i] = ref[i];
    for (int i = 0; i < qMin(test.size(), n); ++i) testRe[i] = test[i];

    // FFT both signals
    const_cast<SignalSynchronizer5*>(this)->computeFFT(refRe, refIm);
    const_cast<SignalSynchronizer5*>(this)->computeFFT(testRe, testIm);

    // Cross-power with PHAT weighting
    QVector<double> gccRe, gccIm;
    crossPowerPHAT(refRe, refIm, testRe, testIm, gccRe, gccIm);

    // IFFT to get cross-correlation
    const_cast<SignalSynchronizer5*>(this)->computeIFFT(gccRe, gccIm);

    // Find peak
    int peakIdx = 0;
    double peakVal = -1e30;
    for (int i = 0; i < n; ++i) {
        if (gccRe[i] > peakVal) {
            peakVal = gccRe[i];
            peakIdx = i;
        }
    }

    // Handle wrap-around (circular correlation)
    if (peakIdx > n / 2) peakIdx -= n;

    double refined = parabolicInterpolation(gccRe, peakIdx + (peakIdx < 0 ? n / 2 : 0));

    result.peakIndex = peakIdx;
    result.delaySamples = (peakIdx < 0) ? peakIdx + n : peakIdx;
    result.delaySamples = peakIdx; // signed delay
    result.delaySeconds = peakIdx / m_sampleRate;
    result.confidence = peakVal / n;
    return result;
}

/* ---- Build spatial correlation matrix ---- */

QVector<QVector<double>> SignalSynchronizer5::buildCorrelationMatrix(
    const QVector<QVector<double>>& freqData) const
{
    int m = freqData.size(); // num sensors
    QVector<QVector<double>> R(m, QVector<double>(m, 0.0));
    int n = freqData[0].size();

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += freqData[i][k] * freqData[j][k];
            R[i][j] = sum / n;
        }
    }
    return R;
}

/* ---- Jacobi eigenvalue decomposition ---- */

void SignalSynchronizer5::jacobiEigen(QVector<QVector<double>>& A,
    QVector<double>& eigenvalues, QVector<QVector<double>>& eigenvectors,
    int maxIter) const
{
    int n = A.size();
    eigenvalues.resize(n);
    eigenvectors.resize(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) eigenvectors[i][i] = 1.0;

    for (int iter = 0; iter < maxIter; ++iter) {
        // Find largest off-diagonal element
        double maxOff = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(A[i][j]) > maxOff) {
                    maxOff = qAbs(A[i][j]);
                    p = i; q = j;
                }
            }
        }
        if (maxOff < 1e-12) break;

        // Compute rotation angle
        double app = A[p][p], aqq = A[q][q], apq = A[p][q];
        double theta = (aqq - app) / (2.0 * apq);
        double t = (theta >= 0 ? 1.0 : -1.0) / (qAbs(theta) + qSqrt(1.0 + theta * theta));
        double c = 1.0 / qSqrt(1.0 + t * t);
        double s = t * c;

        // Apply rotation
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = A[i][p], aiq = A[i][q];
            A[i][p] = A[p][i] = c * aip - s * aiq;
            A[i][q] = A[q][i] = s * aip + c * aiq;
        }
        A[p][p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
        A[q][q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
        A[p][q] = A[q][p] = 0.0;

        // Update eigenvectors
        for (int i = 0; i < n; ++i) {
            double vp = eigenvectors[i][p];
            double vq = eigenvectors[i][q];
            eigenvectors[i][p] = c * vp - s * vq;
            eigenvectors[i][q] = s * vp + c * vq;
        }
    }

    for (int i = 0; i < n; ++i) eigenvalues[i] = A[i][i];
}

/* ---- MUSIC pseudospectrum ---- */

QVector<double> SignalSynchronizer5::musicPseudospectrum(
    const QVector<QVector<double>>& signals, int numSources) const
{
    int m = signals.size();
    int n = signals[0].size();

    // Compute correlation matrix
    QVector<QVector<double>> R = buildCorrelationMatrix(signals);

    // Eigendecomposition
    QVector<double> eigenvalues;
    QVector<QVector<double>> eigenvectors;
    jacobiEigen(R, eigenvalues, eigenvectors);

    // Sort eigenvalues descending
    QVector<int> idx(m);
    for (int i = 0; i < m; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return eigenvalues[a] > eigenvalues[b];
    });

    // Noise subspace: eigenvectors corresponding to smallest eigenvalues
    int numNoise = m - qMin(numSources, m - 1);
    QVector<QVector<double>> noiseSubspace(numNoise);
    for (int i = 0; i < numNoise; ++i)
        noiseSubspace[i] = eigenvectors[idx[m - 1 - i]];

    // Compute pseudospectrum over delay grid
    int gridLen = 256;
    QVector<double> pseudo(gridLen);
    for (int d = 0; d < gridLen; ++d) {
        double tau = (d - gridLen / 2) / m_sampleRate;
        double denom = 0.0;
        for (int ns = 0; ns < numNoise; ++ns) {
            double proj = 0.0;
            for (int i = 0; i < m; ++i)
                proj += noiseSubspace[ns][i] * qCos(2.0 * M_PI * tau * i);
            denom += proj * proj;
        }
        pseudo[d] = (denom > 1e-15) ? 1.0 / denom : 0.0;
    }
    return pseudo;
}

/* ---- Multi-source TDOA ---- */

QVector<SignalSynchronizer5::TDOAResult> SignalSynchronizer5::estimateMultiTDOA(
    const QVector<QVector<double>>& sensorSignals) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<TDOAResult> results;
    int numSensors = sensorSignals.size();
    if (numSensors < 2) return results;

    m_stats.numSensors = numSensors;

    // Use first sensor as reference
    for (int s = 1; s < numSensors; ++s) {
        results.append(estimateGCC(sensorSignals[0], sensorSignals[s]));
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit synchronizationCompleted(results.size(), timer.elapsed());
    return results;
}

/* ---- Synchronize signals ---- */

QVector<QVector<double>> SignalSynchronizer5::synchronize(
    const QVector<QVector<double>>& signals) const
{
    int numSensors = signals.size();
    if (numSensors < 2) return signals;

    QVector<QVector<double>> aligned(numSensors);
    aligned[0] = signals[0];

    for (int s = 1; s < numSensors; ++s) {
        TDOAResult tdoa = estimateGCC(signals[0], signals[s]);
        int delay = qRound(tdoa.delaySamples);

        // Shift signal to align with reference
        if (delay > 0) {
            aligned[s].resize(signals[s].size() + delay);
            for (int i = 0; i < delay; ++i) aligned[s][i] = 0.0;
            for (int i = 0; i < signals[s].size(); ++i)
                aligned[s][i + delay] = signals[s][i];
        } else if (delay < 0) {
            int shift = -delay;
            aligned[s].resize(qMax(0, signals[s].size() - shift));
            for (int i = shift; i < signals[s].size(); ++i)
                aligned[s][i - shift] = signals[s][i];
        } else {
            aligned[s] = signals[s];
        }
    }
    return aligned;
}

/* ---- Reset ---- */

void SignalSynchronizer5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

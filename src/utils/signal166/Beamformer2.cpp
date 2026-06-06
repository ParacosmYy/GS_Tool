/**
 * @file Beamformer2.cpp
 * @brief Beamformer2 实现
 *
 * 实现MVDR波束形成器：采样协方差矩阵估计、
 * Gauss-Jordan矩阵求逆(含正则化对角加载)和空间功率谱扫描。
 */

#include "utils/signal166/Beamformer2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

Beamformer2::Beamformer2(QObject* parent)
    : QObject(parent)
{
}

Beamformer2::~Beamformer2() = default;

void Beamformer2::setNumSensors(int n)
{
    m_numSensors = qMax(1, n);
}

void Beamformer2::setDiagonalLoading(double loading)
{
    m_diagonalLoading = qMax(0.0, loading);
}

void Beamformer2::setSnapshotCount(int count)
{
    m_snapshotCount = qMax(1, count);
}

void Beamformer2::sampleCovariance(const QVector<QVector<double>>& snapshots,
                                    QVector<QVector<double>>& R) const
{
    int M = m_numSensors;
    int K = snapshots.size();
    R.resize(M);
    for (int i = 0; i < M; ++i) R[i].resize(M, 0.0);

    /* R = (1/K) * sum(x * x^T) */
    for (int k = 0; k < K; ++k) {
        const auto& x = snapshots[k];
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < M; ++j) {
                R[i][j] += x[i] * x[j];
            }
        }
    }

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            R[i][j] /= K;
        }
    }

    /* Diagonal loading for regularization */
    for (int i = 0; i < M; ++i) {
        R[i][i] += m_diagonalLoading;
    }
}

bool Beamformer2::invertMatrix(QVector<QVector<double>>& mat) const
{
    int n = mat.size();

    /* Augment with identity */
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }

    /* Gauss-Jordan elimination */
    for (int col = 0; col < n; ++col) {
        /* Find pivot */
        int pivot = col;
        double maxVal = qAbs(aug[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > maxVal) {
                maxVal = qAbs(aug[row][col]);
                pivot = row;
            }
        }
        if (maxVal < 1e-15) return false;

        /* Swap rows */
        if (pivot != col) {
            for (int j = 0; j < 2 * n; ++j) {
                std::swap(aug[col][j], aug[pivot][j]);
            }
        }

        /* Scale pivot row */
        double pivotVal = aug[col][col];
        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivotVal;

        /* Eliminate column */
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* Extract inverse */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            mat[i][j] = aug[i][n + j];
        }
    }
    return true;
}

double Beamformer2::innerProduct(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

QVector<double> Beamformer2::matVecMultiply(const QVector<QVector<double>>& M,
                                            const QVector<double>& v)
{
    int n = M.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(M[i].size(), v.size());
        for (int j = 0; j < cols; ++j) {
            result[i] += M[i][j] * v[j];
        }
    }
    return result;
}

QVector<double> Beamformer2::computeWeights(
    const QVector<QVector<double>>& snapshots,
    const QVector<double>& steeringVector)
{
    QElapsedTimer timer;
    timer.start();

    int M = m_numSensors;
    if (M == 0 || snapshots.isEmpty() || steeringVector.size() != M) {
        return QVector<double>();
    }

    /* Compute sample covariance matrix */
    QVector<QVector<double>> R;
    sampleCovariance(snapshots, R);

    /* Invert R */
    if (!invertMatrix(R)) {
        return QVector<double>(M, 0.0);
    }

    /* w = R^{-1} * a / (a^H * R^{-1} * a) */
    QVector<double> RinvA = matVecMultiply(R, steeringVector);
    double aRinvA = innerProduct(steeringVector, RinvA);

    if (qAbs(aRinvA) < 1e-15) {
        return QVector<double>(M, 0.0);
    }

    QVector<double> weights(M);
    for (int i = 0; i < M; ++i) {
        weights[i] = RinvA[i] / aRinvA;
    }

    /* Estimate SNR improvement */
    double signalGain = qAbs(innerProduct(weights, steeringVector));
    double noisePower = 0.0;
    for (int i = 0; i < M; ++i) noisePower += weights[i] * weights[i];
    m_stats.lastSnrImprovementDb = (noisePower > 1e-15)
        ? 20.0 * qLn(signalGain / qSqrt(noisePower)) / qLn(10.0) : 0.0;

    m_stats.totalBeamforms++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalBeamforms + m_stats.totalSpectrumCalcs;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit beamformCompleted(M);
    return weights;
}

double Beamformer2::applyBeamform(const QVector<double>& weights,
                                  const QVector<double>& snapshot) const
{
    return innerProduct(weights, snapshot);
}

QVector<double> Beamformer2::spatialSpectrum(
    const QVector<QVector<double>>& snapshots,
    const QVector<QVector<double>>& steeringVectors)
{
    QElapsedTimer timer;
    timer.start();

    int M = m_numSensors;
    if (M == 0 || snapshots.isEmpty() || steeringVectors.isEmpty()) {
        return QVector<double>();
    }

    /* Compute sample covariance matrix */
    QVector<QVector<double>> R;
    sampleCovariance(snapshots, R);

    /* Invert R */
    if (!invertMatrix(R)) {
        return QVector<double>(steeringVectors.size(), 0.0);
    }

    /* For each steering vector, compute P = 1 / (a^H * R^{-1} * a) */
    QVector<double> spectrum(steeringVectors.size());
    for (int d = 0; d < steeringVectors.size(); ++d) {
        const auto& a = steeringVectors[d];
        if (a.size() != M) { spectrum[d] = 0.0; continue; }

        QVector<double> RinvA = matVecMultiply(R, a);
        double denom = innerProduct(a, RinvA);
        spectrum[d] = (qAbs(denom) > 1e-15) ? 1.0 / denom : 0.0;
    }

    m_stats.totalSpectrumCalcs++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalBeamforms + m_stats.totalSpectrumCalcs;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit spectrumCompleted(steeringVectors.size());
    return spectrum;
}

void Beamformer2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

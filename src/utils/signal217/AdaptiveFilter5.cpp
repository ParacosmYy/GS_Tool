/**
 * @file AdaptiveFilter5.cpp
 * @brief AdaptiveFilter5 实现
 *
 * 实现自适应滤波：分块频域分区卷积、重叠保留、频域LMS更新。
 */

#include "utils/signal217/AdaptiveFilter5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter5::AdaptiveFilter5(QObject *parent) : QObject(parent) {}
AdaptiveFilter5::~AdaptiveFilter5() = default;

/* ---- Configuration ---- */

void AdaptiveFilter5::setParameters(int filterLength, int blockSize,
                                     double stepSize, double regularization)
{
    m_filterLen = qMax(1, filterLength);
    m_blockSize = qMax(1, blockSize);
    m_stepSize = qBound(1e-6, stepSize, 1.0);
    m_reg = qMax(1e-12, regularization);

    // FFT size = 2 * block size for overlap-save
    m_fftSize = 2 * m_blockSize;
    // Number of partitions to cover full filter length
    m_numPartitions = (m_filterLen + m_blockSize - 1) / m_blockSize;

    // Initialize frequency-domain weight partitions
    m_wReal.resize(m_numPartitions);
    m_wImag.resize(m_numPartitions);
    for (int p = 0; p < m_numPartitions; ++p) {
        m_wReal[p].resize(m_fftSize, 0.0);
        m_wImag[p].resize(m_fftSize, 0.0);
    }

    m_overlapBuf.resize(m_numPartitions);
    for (int p = 0; p < m_numPartitions; ++p)
        m_overlapBuf[p].resize(m_fftSize, 0.0);

    m_inputHistory.resize(m_numPartitions);
    for (int p = 0; p < m_numPartitions; ++p)
        m_inputHistory[p].resize(m_fftSize, 0.0);

    m_stats.filterLength = m_filterLen;
    m_stats.blockSize = m_blockSize;
    m_stats.numPartitions = m_numPartitions;
    m_stats.stepSize = m_stepSize;
}

/* ---- Bit-reversal permutation ---- */

void AdaptiveFilter5::bitReverse(QVector<double>& real,
                                  QVector<double>& imag) const
{
    int n = real.size();
    int bits = 0;
    while ((1 << bits) < n) ++bits;
    for (int i = 0; i < n; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < bits; ++b) { rev = (rev << 1) | (val & 1); val >>= 1; }
        if (rev > i) {
            std::swap(real[i], real[rev]);
            std::swap(imag[i], imag[rev]);
        }
    }
}

/* ---- FFT ---- */

void AdaptiveFilter5::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    bitReverse(real, imag);
    for (int len = 2; len <= n; len *= 2) {
        int half = len / 2;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                double angle = -2.0 * M_PI * j / len;
                double wr = qCos(angle), wi = qSin(angle);
                double tr = real[i+j+half]*wr - imag[i+j+half]*wi;
                double ti = real[i+j+half]*wi + imag[i+j+half]*wr;
                real[i+j+half] = real[i+j] - tr;
                imag[i+j+half] = imag[i+j] - ti;
                real[i+j] += tr;
                imag[i+j] += ti;
            }
        }
    }
}

/* ---- IFFT ---- */

void AdaptiveFilter5::ifft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    for (auto& v : imag) v = -v;
    fft(real, imag);
    for (int i = 0; i < n; ++i) { real[i] /= n; imag[i] = -imag[i] / n; }
}

/* ---- Complex multiply ---- */

void AdaptiveFilter5::cmul(double& ar, double& ai, double br, double bi)
{
    double r = ar * br - ai * bi;
    double i = ar * bi + ai * br;
    ar = r; ai = i;
}

/* ---- Process block ---- */

QVector<double> AdaptiveFilter5::process(const QVector<double>& input,
                                           const QVector<double>& desired)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_fftSize;
    int B = m_blockSize;

    // Shift input history: newest block at partition 0
    for (int p = m_numPartitions - 1; p > 0; --p)
        m_inputHistory[p] = m_inputHistory[p - 1];

    // Fill partition 0 with current input block (zero-padded)
    m_inputHistory[0].resize(N, 0.0);
    for (int i = 0; i < B && i < input.size(); ++i)
        m_inputHistory[0][i] = input[i];
    for (int i = B; i < N; ++i)
        m_inputHistory[0][i] = 0.0;

    // Transform input blocks to frequency domain
    QVector<QVector<double>> Xr(m_numPartitions), Xi(m_numPartitions);
    for (int p = 0; p < m_numPartitions; ++p) {
        Xr[p] = m_inputHistory[p];
        Xi[p].resize(N, 0.0);
        fft(Xr[p], Xi[p]);
    }

    // Partitioned frequency-domain convolution
    QVector<double> Yr(N, 0.0), Yi(N, 0.0);
    for (int p = 0; p < m_numPartitions; ++p) {
        for (int k = 0; k < N; ++k) {
            double yr = Xr[p][k] * m_wReal[p][k] - Xi[p][k] * m_wImag[p][k];
            double yi = Xr[p][k] * m_wImag[p][k] + Xi[p][k] * m_wReal[p][k];
            Yr[k] += yr;
            Yi[k] += yi;
        }
    }

    // IFFT to get output
    ifft(Yr, Yi);

    // Extract valid samples (overlap-save: last B samples)
    QVector<double> output(B, 0.0);
    for (int i = 0; i < B; ++i)
        output[i] = Yr[B + i];

    // Compute error signal
    QVector<double> error(B, 0.0);
    double errPow = 0.0;
    for (int i = 0; i < B && i < desired.size(); ++i) {
        error[i] = desired[i] - output[i];
        errPow += error[i] * error[i];
    }
    m_errorPower = (errPow > 0) ? errPow / B : 0.0;

    // Transform error to frequency domain for adaptation
    QVector<double> Er(N, 0.0), Ei(N, 0.0);
    for (int i = 0; i < B; ++i) Er[i] = error[i];
    fft(Er, Ei);

    // Frequency-domain LMS update for each partition
    for (int p = 0; p < m_numPartitions; ++p) {
        for (int k = 0; k < N; ++k) {
            // Conjugate of X_p[k]
            double xconj_r = Xr[p][k], xconj_i = -Xi[p][k];
            // Gradient: E[k] * conj(X_p[k])
            double gr = Er[k] * xconj_r - Ei[k] * xconj_i;
            double gi = Er[k] * xconj_i + Ei[k] * xconj_r;
            // Normalized update with regularization
            double px = Xr[p][k]*Xr[p][k] + Xi[p][k]*Xi[p][k] + m_reg;
            m_wReal[p][k] += m_stepSize * gr / px;
            m_wImag[p][k] += m_stepSize * gi / px;
        }
    }

    m_stats.errorPower = m_errorPower;
    m_stats.totalBlocks++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;
    emit blockProcessed(m_stats.totalBlocks, m_errorPower, timer.elapsed());

    return error;
}

/* ---- Filter only (no adaptation) ---- */

QVector<double> AdaptiveFilter5::filter(const QVector<double>& input) const
{
    int N = m_fftSize;
    int B = m_blockSize;

    QVector<double> Xr(N, 0.0), Xi(N, 0.0);
    for (int i = 0; i < B && i < input.size(); ++i) Xr[i] = input[i];

    const_cast<AdaptiveFilter5*>(this)->fft(Xr, Xi);

    QVector<double> Yr(N, 0.0), Yi(N, 0.0);
    for (int p = 0; p < m_numPartitions; ++p) {
        for (int k = 0; k < N; ++k) {
            Yr[k] += Xr[k]*m_wReal[p][k] - Xi[k]*m_wImag[p][k];
            Yi[k] += Xr[k]*m_wImag[p][k] + Xi[k]*m_wReal[p][k];
        }
    }
    const_cast<AdaptiveFilter5*>(this)->ifft(Yr, Yi);

    QVector<double> out(B, 0.0);
    for (int i = 0; i < B; ++i) out[i] = Yr[B + i];
    return out;
}

/* ---- Get coefficients in time domain ---- */

QVector<double> AdaptiveFilter5::coefficients() const
{
    QVector<double> h(m_filterLen, 0.0);
    for (int p = 0; p < m_numPartitions; ++p) {
        QVector<double> wr = m_wReal[p], wi = m_wImag[p];
        const_cast<AdaptiveFilter5*>(this)->ifft(wr, wi);
        for (int i = 0; i < m_blockSize && (p * m_blockSize + i) < m_filterLen; ++i)
            h[p * m_blockSize + i] = wr[i];
    }
    return h;
}

/* ---- Clear state ---- */

void AdaptiveFilter5::clear()
{
    for (int p = 0; p < m_numPartitions; ++p) {
        std::fill(m_wReal[p].begin(), m_wReal[p].end(), 0.0);
        std::fill(m_wImag[p].begin(), m_wImag[p].end(), 0.0);
        std::fill(m_inputHistory[p].begin(), m_inputHistory[p].end(), 0.0);
        std::fill(m_overlapBuf[p].begin(), m_overlapBuf[p].end(), 0.0);
    }
    m_errorPower = 0.0;
}

/* ---- Reset ---- */

void AdaptiveFilter5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

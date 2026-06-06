/**
 * @file ChirpZ4.cpp
 * @brief ChirpZ4 实现
 *
 * 实现Chirp-z变换：Bluestein算法、基2 FFT、任意长度DFT。
 */

#include "utils/fft177/ChirpZ4.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

ChirpZ4::ChirpZ4(QObject *parent)
    : QObject(parent)
{
}

ChirpZ4::~ChirpZ4() = default;

/* ---- Configuration ---- */

void ChirpZ4::setOutputSize(int M) { m_outputSize = qMax(0, M); }

void ChirpZ4::setSpiralParameters(double wReal, double wImag,
                                   double aReal, double aImag)
{
    m_wReal = wReal;
    m_wImag = wImag;
    m_aReal = aReal;
    m_aImag = aImag;
}

/* ---- Next power of 2 ---- */

int ChirpZ4::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- In-place radix-2 Cooley-Tukey FFT ---- */

void ChirpZ4::fft(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;

    /* Bit-reversal permutation */
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* Butterfly stages */
    for (int len = 2; len <= N; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < N; ++i) {
            re[i] /= N;
            im[i] /= N;
        }
    }
}

/* ---- Generate chirp coefficients ---- */

void ChirpZ4::generateChirp(int N, int M)
{
    int L = nextPow2(N + M - 1);
    if (L == m_lastFFTSize && !m_chirpRe.isEmpty()) return;
    m_lastFFTSize = L;

    /* y[k] = W^(k^2/2) * A^(-k) for chirp filter */
    m_chirpRe.resize(L);
    m_chirpIm.resize(L);
    m_chirpRe.fill(0.0);
    m_chirpIm.fill(0.0);

    for (int k = 0; k < qMax(N, M); ++k) {
        double kSq = k * k / 2.0;
        double wAngle = kSq; /* W^(k^2/2) = exp(-j*pi*k^2) for default spiral */
        double wRe = qCos(wAngle);
        double wIm = -qSin(wAngle);
        if (k < N) {
            m_chirpRe[k] = wRe;
            m_chirpIm[k] = wIm;
        }
        /* Negative wrap for convolution */
        if (k > 0 && L - k < L) {
            m_chirpRe[L - k] = wRe;
            m_chirpIm[L - k] = wIm;
        }
    }

    /* FFT of chirp filter */
    m_chirpFFTRe = m_chirpRe;
    m_chirpFFTIm = m_chirpIm;
    fft(m_chirpFFTRe, m_chirpFFTIm, false);
}

/* ---- Bluestein DFT (arbitrary size) ---- */

QPair<QVector<double>, QVector<double>> ChirpZ4::bluesteinDFT(
    const QVector<double>& input)
{
    int N = input.size();
    if (N == 0) return {{}, {}};

    /* If power of 2, just use standard FFT */
    if ((N & (N - 1)) == 0) {
        QVector<double> re = input;
        QVector<double> im(N, 0.0);
        fft(re, im, false);
        return {re, im};
    }

    /* Bluestein's algorithm */
    int L = nextPow2(2 * N - 1);

    /* Step 1: Multiply input by chirp */
    QVector<double> aRe(L, 0.0), aIm(L, 0.0);
    for (int n = 0; n < N; ++n) {
        double angle = -M_PI * n * n / N;
        aRe[n] = input[n] * qCos(angle);
        aIm[n] = input[n] * qSin(angle);
    }

    /* Step 2: Create chirp filter */
    QVector<double> bRe(L, 0.0), bIm(L, 0.0);
    for (int n = 0; n < N; ++n) {
        double angle = M_PI * n * n / N;
        bRe[n] = qCos(angle);
        bIm[n] = qSin(angle);
    }
    for (int n = L - N + 1; n < L; ++n) {
        int k = L - n;
        double angle = M_PI * k * k / N;
        bRe[n] = qCos(angle);
        bIm[n] = qSin(angle);
    }

    /* Step 3: Circular convolution via FFT */
    fft(aRe, aIm, false);
    fft(bRe, bIm, false);

    for (int i = 0; i < L; ++i) {
        double re = aRe[i] * bRe[i] - aIm[i] * bIm[i];
        double im = aRe[i] * bIm[i] + aIm[i] * bRe[i];
        aRe[i] = re;
        aIm[i] = im;
    }

    fft(aRe, aIm, true);

    /* Step 4: Multiply by conjugate chirp */
    QVector<double> outRe(N), outIm(N);
    for (int n = 0; n < N; ++n) {
        double angle = -M_PI * n * n / N;
        double cRe = qCos(angle);
        double cIm = qSin(angle);
        outRe[n] = aRe[n] * cRe - aIm[n] * cIm;
        outIm[n] = aRe[n] * cIm + aIm[n] * cRe;
    }

    return {outRe, outIm};
}

/* ---- Chirp-z transform ---- */

QPair<QVector<double>, QVector<double>> ChirpZ4::transform(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {{}, {}};

    int M = (m_outputSize > 0) ? m_outputSize : N;

    /* Default spiral: evaluate DFT on arbitrary contour */
    bool useDefaultSpiral = (m_wReal == 0.0 && m_wImag == 0.0);

    QVector<double> re, im;
    if (useDefaultSpiral) {
        /* Standard DFT via Bluestein */
        auto result = bluesteinDFT(input);
        re = result.first.mid(0, M);
        im = result.second.mid(0, M);
    } else {
        /* General chirp-z on specified spiral */
        int L = nextPow2(N + M - 1);
        re.resize(M);
        im.resize(M);

        for (int k = 0; k < M; ++k) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = m_wReal * n * k + m_wImag * n * k;
                double aAngle = m_aReal * n + m_aImag * n;
                double totalAngle = angle + aAngle;
                double wRe = qCos(totalAngle);
                double wIm = -qSin(totalAngle);
                sumRe += input[n] * wRe;
                sumIm += input[n] * wIm;
            }
            re[k] = sumRe;
            im[k] = sumIm;
        }
    }

    /* Compute magnitude and phase */
    QVector<double> magnitude(re.size());
    QVector<double> phase(re.size());
    for (int i = 0; i < re.size(); ++i) {
        magnitude[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
        phase[i] = qAtan2(im[i], re[i]);
    }

    m_stats.totalTransforms++;
    m_stats.inputSize = N;
    m_stats.outputSize = M;
    m_stats.fftSize = nextPow2(N + M - 1);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, M);
    return {magnitude, phase};
}

/* ---- Reset ---- */

void ChirpZ4::reset()
{
    m_chirpRe.clear();
    m_chirpIm.clear();
    m_chirpFFTRe.clear();
    m_chirpFFTIm.clear();
    m_lastFFTSize = 0;
}

void ChirpZ4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

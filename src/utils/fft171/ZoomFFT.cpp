/**
 * @file ZoomFFT.cpp
 * @brief ZoomFFT 实现
 *
 * 实现Chirp-z变换缩放FFT：chirp因子生成、FFT卷积加速、频带选择。
 */

#include "utils/fft171/ZoomFFT.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction ---- */

ZoomFFT::ZoomFFT(QObject *parent) : QObject(parent) {}
ZoomFFT::~ZoomFFT() = default;

/* ---- Helpers ---- */

void ZoomFFT::cmul(double ar, double ai, double br, double bi,
                    double& cr, double& ci)
{
    cr = ar * br - ai * bi;
    ci = ar * bi + ai * br;
}

int ZoomFFT::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Radix-2 FFT (Cooley-Tukey) ---- */

void ZoomFFT::fft(QVector<double>& real, QVector<double>& imag, bool inverse) const
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

    /* Butterfly stages */
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

/* ---- Chirp-z Transform ---- */

QVector<double> ZoomFFT::chirpZTransform(const QVector<double>& input,
                                           double startBin, double endBin,
                                           int numBins)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    int M = qMax(1, numBins);
    if (N == 0) return QVector<double>();

    /* Chirp-z parameters:
     * z_k = A * W^{-k}, A = e^{j*2*pi*startBin/N}, W = e^{j*2*pi*(endBin-startBin)/(M*N)}
     * Use Bluestein's algorithm: CZT via convolution with chirp */

    double aAngle = 2.0 * M_PI * startBin / N;
    double aReal = qCos(aAngle), aImag = qSin(aAngle);

    double wAngle = 2.0 * M_PI * (endBin - startBin) / (static_cast<qint64>(M) * N);
    double wReal = qCos(wAngle), wImag = qSin(wAngle);

    /* Build chirp sequence: y[n] = x[n] * A^{-n} * W^{n^2/2} */
    int convLen = nextPow2(N + M - 1);

    QVector<double> yReal(convLen, 0.0), yImag(convLen, 0.0);
    QVector<double> chirpReal(convLen, 0.0), chirpImag(convLen, 0.0);

    /* Precompute W^{n^2/2} */
    QVector<double> wPowReal(N + M), wPowImag(N + M);
    wPowReal[0] = 1.0; wPowImag[0] = 0.0;
    for (int i = 1; i < N + M; ++i)
        cmul(wPowReal[i - 1], wPowImag[i - 1], wReal, wImag,
             wPowReal[i], wPowImag[i]);

    /* y[n] = x[n] * A^{-n} * W^{n^2/2} */
    double curAReal = 1.0, curAImag = 0.0;
    for (int n = 0; n < N; ++n) {
        double wr = wPowReal[n * n / 2 + (n % 2 == 0 ? 0 : n / 2)];
        double wi = wPowImag[n * n / 2 + (n % 2 == 0 ? 0 : n / 2)];

        /* Simplified: W^{n^2/2} approximation */
        int idx = n * (n + 1) / 2;
        if (idx < wPowReal.size()) { wr = wPowReal[idx]; wi = wPowImag[idx]; }

        double invAR = curAReal, invAI = -curAImag;
        double tmpR, tmpI;
        cmul(input[n], 0.0, invAR, invAI, tmpR, tmpI);
        cmul(tmpR, tmpI, wr, wi, yReal[n], yImag[n]);

        /* Advance A^{-n} */
        double newAR = curAReal * aReal + curAImag * aImag;
        double newAI = curAImag * aReal - curAReal * aImag;
        curAReal = newAR; curAImag = newAI;
    }

    /* Chirp filter: c[n] = W^{-n^2/2} for -N+1 <= n <= M-1 */
    for (int n = 0; n < N + M - 1; ++n) {
        int k = n - (N - 1);
        int absK = qAbs(k);
        if (absK < wPowReal.size()) {
            chirpReal[n] = wPowReal[absK];
            chirpImag[n] = (k < 0) ? -wPowImag[absK] : wPowImag[absK];
        }
    }

    /* Convolution via FFT */
    fft(yReal, yImag, false);
    fft(chirpReal, chirpImag, false);

    for (int i = 0; i < convLen; ++i) {
        double r, im;
        cmul(yReal[i], yImag[i], chirpReal[i], chirpImag[i], r, im);
        yReal[i] = r; yImag[i] = im;
    }

    fft(yReal, yImag, true);

    /* Extract first M outputs and multiply by W^{k^2/2} */
    QVector<double> result(2 * M);
    for (int k = 0; k < M; ++k) {
        int idx = k * (k + 1) / 2;
        double wr = 1.0, wi = 0.0;
        if (idx < wPowReal.size()) { wr = wPowReal[idx]; wi = wPowImag[idx]; }

        double r, im;
        int pos = N - 1 + k;
        if (pos < convLen)
            cmul(yReal[pos], yImag[pos], wr, wi, r, im);
        else
            r = im = 0.0;

        result[2 * k] = r;      /* real part */
        result[2 * k + 1] = im;  /* imag part */
    }

    m_stats.totalTransforms++;
    m_stats.lastFFTSize = N;
    m_stats.lastZoomBins = M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(M);
    return result;
}

/* ---- Convenience zoom ---- */

QVector<double> ZoomFFT::zoom(const QVector<double>& input,
                                double centerFreqHz, double bandwidthHz,
                                double sampleRateHz, int numBins)
{
    int N = input.size();
    double binRes = sampleRateHz / qMax(1, N);
    double startBin = (centerFreqHz - bandwidthHz / 2.0) / binRes;
    double endBin = (centerFreqHz + bandwidthHz / 2.0) / binRes;

    QVector<double> czt = chirpZTransform(input, startBin, endBin, numBins);

    /* Convert to magnitude */
    int M = czt.size() / 2;
    QVector<double> mag(M);
    for (int k = 0; k < M; ++k)
        mag[k] = qSqrt(czt[2 * k] * czt[2 * k] + czt[2 * k + 1] * czt[2 * k + 1]);

    return mag;
}

/* ---- Statistics ---- */

void ZoomFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

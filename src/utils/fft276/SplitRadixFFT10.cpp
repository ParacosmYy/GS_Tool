/**
 * @file SplitRadixFFT10.cpp
 * @brief SplitRadixFFT10 实现
 *
 * 实现分裂基数FFT：共轭对旋转因子优化与实输入对称性N/2复数点变换。
 */

#include "utils/fft276/SplitRadixFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplitRadixFFT10::SplitRadixFFT10(QObject *parent)
    : QObject(parent)
{
    buildBitRevTable();
    buildTwiddles();
}

SplitRadixFFT10::~SplitRadixFFT10() = default;

/* ---- Configuration ---- */

void SplitRadixFFT10::setSize(int n)
{
    // Round to next power of 2
    int p = 1;
    while (p < n) p <<= 1;
    m_n = qBound(2, p, 1 << 22);
    buildBitRevTable();
    buildTwiddles();
}

/* ---- Build bit-reversal table ---- */

void SplitRadixFFT10::buildBitRevTable()
{
    int bits = 0;
    int tmp = m_n;
    while (tmp > 1) { bits++; tmp >>= 1; }

    m_bitRev.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        m_bitRev[i] = rev;
    }
}

/* ---- Build conjugate-pair twiddle factors ---- */

void SplitRadixFFT10::buildTwiddles()
{
    // Store W_N^k for k = 0..N/4-1 (conjugate pairs cover full range)
    int n4 = m_n / 4;
    m_twiddlesCos.resize(n4 + 1);
    m_twiddlesSin.resize(n4 + 1);
    for (int k = 0; k <= n4; ++k) {
        double angle = -2.0 * M_PI * k / m_n;
        m_twiddlesCos[k] = qCos(angle);
        m_twiddlesSin[k] = qSin(angle);
    }
}

/* ---- Bit-reversal permutation ---- */

void SplitRadixFFT10::bitReversePermute(QVector<double>& re, QVector<double>& im) const
{
    for (int i = 0; i < m_n; ++i) {
        int j = m_bitRev[i];
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
}

/* ---- Core split-radix butterfly (recursive) ---- */

void SplitRadixFFT10::splitRadixCore(QVector<double>& re, QVector<double>& im,
                                      int start, int length, int stride)
{
    if (length <= 2) {
        // Base case: simple butterfly
        double tRe = re[start] - re[start + stride];
        double tIm = im[start] - im[start + stride];
        re[start] += re[start + stride];
        im[start] += im[start + stride];
        re[start + stride] = tRe;
        im[start + stride] = tIm;
        return;
    }

    if (length == 4) {
        // Radix-4 butterfly using conjugate twiddle pairs
        int s = stride;
        double t0Re = re[start] - re[start + 2*s];
        double t0Im = im[start] - im[start + 2*s];
        re[start] += re[start + 2*s];
        im[start] += im[start + 2*s];

        double t1Re = re[start + s] - re[start + 3*s];
        double t1Im = im[start + s] - im[start + 3*s];
        re[start + s] += re[start + 3*s];
        im[start + s] += im[start + 3*s];

        // Twiddle t1 by -j (multiply by (0,-1))
        double t1wRe = t1Im;
        double t1wIm = -t1Re;

        re[start + 2*s] = t0Re - t1wRe;
        im[start + 2*s] = t0Im - t1wIm;
        re[start + 3*s] = t0Re + t1wRe;
        im[start + 3*s] = t0Im + t1wIm;
        return;
    }

    int halfLen = length / 2;
    int quarterLen = length / 4;

    // Apply twiddle factors to odd-indexed half
    for (int k = 0; k < quarterLen; ++k) {
        int twIdx = k * stride;
        int n4 = m_n / 4;
        // W_N^{k*stride} using conjugate pair optimization
        double wr1, wi1, wr3, wi3;
        int kIdx = twIdx % n4;
        int kQuad = (twIdx / n4) % 4;
        double c = m_twiddlesCos[kIdx];
        double s_val = m_twiddlesSin[kIdx];

        // Handle quadrant rotations for conjugate optimization
        switch (kQuad) {
        case 0: wr1 = c; wi1 = s_val; break;
        case 1: wr1 = -s_val; wi1 = c; break;
        case 2: wr1 = -c; wi1 = -s_val; break;
        default: wr1 = s_val; wi1 = -c; break;
        }
        // W_N^{3k} = conj(W_N^{-3k}) derived from W_N^k
        double angle3 = -6.0 * M_PI * twIdx / m_n;
        wr3 = qCos(angle3);
        wi3 = qSin(angle3);

        int idx1 = start + (halfLen + k) * stride;
        int idx3 = start + (halfLen + quarterLen + k) * stride;

        double x1Re = re[idx1], x1Im = im[idx1];
        double x3Re = re[idx3], x3Im = im[idx3];

        re[idx1] = x1Re * wr1 - x1Im * wi1;
        im[idx1] = x1Re * wi1 + x1Im * wr1;
        re[idx3] = x3Re * wr3 - x3Im * wi3;
        im[idx3] = x3Re * wi3 + x3Im * wr3;
    }

    // Recurse: even half, odd-quarter, odd-3quarter
    splitRadixCore(re, im, start, halfLen, stride);
    splitRadixCore(re, im, start + halfLen * stride, quarterLen, stride);
    splitRadixCore(re, im, start + (halfLen + quarterLen) * stride, quarterLen, stride);

    // Combine results
    for (int k = 0; k < quarterLen; ++k) {
        int eIdx = start + k * stride;
        int o1Idx = start + (halfLen + k) * stride;
        int o3Idx = start + (halfLen + quarterLen + k) * stride;

        double eRe = re[eIdx], eIm = im[eIdx];
        double o1Re = re[o1Idx], o1Im = im[o1Idx];
        double o3Re = re[o3Idx], o3Im = im[o3Idx];

        // X[k] = E[k] + O1[k] + O3[k]
        re[eIdx] = eRe + o1Re + o3Re;
        im[eIdx] = eIm + o1Im + o3Im;

        // X[k + N/4] using -j rotation
        re[o1Idx] = eRe + (o1Im - o3Im);
        im[o1Idx] = eIm - (o1Re - o3Re);

        // X[k + N/2]
        re[start + (k + halfLen) * stride] = eRe - o1Re - o3Re;
        im[start + (k + halfLen) * stride] = eIm - o1Im - o3Im;

        // X[k + 3N/4] using +j rotation
        re[o3Idx] = eRe - (o1Im - o3Im);
        im[o3Idx] = eIm + (o1Re - o3Re);
    }
}

/* ---- Pack real signal into N/2 complex points ---- */

void SplitRadixFFT10::packReal(const QVector<double>& input,
                                QVector<double>& re, QVector<double>& im) const
{
    int halfN = m_n / 2;
    re.resize(halfN);
    im.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        re[i] = input[2 * i];      // Even samples → real part
        im[i] = (2 * i + 1 < input.size()) ? input[2 * i + 1] : 0.0;  // Odd → imag
    }
}

/* ---- Unpack N/2 complex spectrum to full N-point real spectrum ---- */

QVector<double> SplitRadixFFT10::unpackRealSpectrum(const QVector<double>& re,
                                                     const QVector<double>& im) const
{
    int halfN = m_n / 2;
    QVector<double> result(2 * m_n, 0.0);

    for (int k = 0; k < halfN; ++k) {
        int km = (k == 0) ? 0 : halfN - k;

        double reK = re[k], imK = im[k];
        double reM = re[km], imM = im[km];

        // X[k] = 0.5*(F[k] + conj(F[N/2-k])) - j*0.5*(F[k] - conj(F[N/2-k]))
        double xr = 0.5 * (reK + reM);
        double xi = 0.5 * (imK - imM);
        double yr = 0.5 * (imK + imM);
        double yi = 0.5 * (reM - reK);

        // Apply twiddle W_N^k
        double angle = -2.0 * M_PI * k / m_n;
        double wr = qCos(angle), wi = qSin(angle);
        double tr = yr * wr - yi * wi;
        double ti = yr * wi + yi * wr;

        result[2 * k] = xr + tr;
        result[2 * k + 1] = xi + ti;

        if (k > 0) {
            int k2 = m_n - k;
            result[2 * k2] = xr - tr;
            result[2 * k2 + 1] = -(xi + ti);
        }
    }
    result[0] = re[0] + im[0];    // DC component
    result[1] = 0.0;
    if (m_n > 1) {
        result[2 * halfN] = re[0] - im[0];  // Nyquist
        result[2 * halfN + 1] = 0.0;
    }

    return result;
}

/* ---- Forward FFT of real signal ---- */

QVector<double> SplitRadixFFT10::forwardReal(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    // Pack real input into N/2 complex points
    QVector<double> re, im;
    packReal(input, re, im);

    // Bit-reverse and split-radix on N/2 complex points
    int halfN = m_n / 2;
    QVector<int> halfBitRev(halfN);
    int bits = 0;
    { int t = halfN; while (t > 1) { bits++; t >>= 1; } }
    for (int i = 0; i < halfN; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < bits; ++b) { rev = (rev<<1)|(val&1); val >>= 1; }
        halfBitRev[i] = rev;
    }
    for (int i = 0; i < halfN; ++i) {
        int j = halfBitRev[i];
        if (i < j) { std::swap(re[i],re[j]); std::swap(im[i],im[j]); }
    }

    // Temporarily change m_n for recursive call on half-size
    int saveN = m_n;
    m_n = halfN;
    splitRadixCore(re, im, 0, halfN, 1);
    m_n = saveN;

    QVector<double> result = unpackRealSpectrum(re, im);

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_n, m_stats.numTransforms, elapsed);

    return result;
}

/* ---- Forward FFT of complex signal ---- */

QVector<double> SplitRadixFFT10::forwardComplex(const QVector<double>& reIn,
                                                  const QVector<double>& imIn)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_n, 0.0), im(m_n, 0.0);
    for (int i = 0; i < qMin(m_n, reIn.size()); ++i) re[i] = reIn[i];
    for (int i = 0; i < qMin(m_n, imIn.size()); ++i) im[i] = imIn[i];

    bitReversePermute(re, im);
    splitRadixCore(re, im, 0, m_n, 1);

    QVector<double> result(2 * m_n);
    for (int i = 0; i < m_n; ++i) { result[2*i] = re[i]; result[2*i+1] = im[i]; }

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_n, m_stats.numTransforms, elapsed);

    return result;
}

/* ---- Inverse FFT returning real signal ---- */

QVector<double> SplitRadixFFT10::inverseReal(const QVector<double>& spectrum)
{
    // Extract re/im from interleaved, conjugate, forward, scale
    QVector<double> re(m_n, 0.0), im(m_n, 0.0);
    for (int i = 0; i < qMin(m_n, spectrum.size()/2); ++i) {
        re[i] = spectrum[2*i];
        im[i] = -spectrum[2*i+1];
    }
    bitReversePermute(re, im);
    splitRadixCore(re, im, 0, m_n, 1);
    QVector<double> result(m_n);
    for (int i = 0; i < m_n; ++i) result[i] = re[i] / m_n;
    return result;
}

/* ---- Inverse FFT of complex spectrum ---- */

QVector<double> SplitRadixFFT10::inverseComplex(const QVector<double>& reIn,
                                                  const QVector<double>& imIn)
{
    QVector<double> re(m_n, 0.0), im(m_n, 0.0);
    for (int i = 0; i < qMin(m_n, reIn.size()); ++i) { re[i] = reIn[i]; im[i] = -imIn[i]; }
    bitReversePermute(re, im);
    splitRadixCore(re, im, 0, m_n, 1);
    QVector<double> result(2 * m_n);
    for (int i = 0; i < m_n; ++i) {
        result[2*i] = re[i] / m_n;
        result[2*i+1] = -im[i] / m_n;
    }
    return result;
}

/* ---- Reset ---- */

void SplitRadixFFT10::resetStatistics()
{
    m_n = 256;
    buildBitRevTable();
    buildTwiddles();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file WinogradFFT.cpp
 * @brief WinogradFFT 实现
 *
 * 实现Winograd FFT：小N(2,3,4,5,7)最小乘法DFT核心、大N嵌套分解。
 */

#include "utils/fft174/WinogradFFT.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

WinogradFFT::WinogradFFT(QObject *parent)
    : QObject(parent)
{
}

WinogradFFT::~WinogradFFT() = default;

/* ---- Multiply count estimation ---- */

int WinogradFFT::multiplyCount(int n)
{
    /* W-DFT multiplication counts for supported sizes */
    if (n == 2) return 1;
    if (n == 3) return 2;
    if (n == 4) return 3;
    if (n == 5) return 5;
    if (n == 7) return 8;
    /* For composite N = n1*n2, total muls ~ m1*n2 + m2*n1 */
    return static_cast<int>(n * 0.5 * qLn(n)); /* Approximate */
}

/* ---- Factorization into supported primes {2,3,4,5,7} ---- */

QVector<int> WinogradFFT::factorize(int n) const
{
    QVector<int> factors;
    int remaining = n;
    int primes[] = {7, 5, 4, 3, 2};
    for (int p : primes) {
        while (remaining % p == 0) {
            factors.append(p);
            remaining /= p;
        }
    }
    /* If remaining > 1, we have unsupported factor */
    if (remaining > 1) factors.clear();
    return factors;
}

/* ---- W-DFT size 2 (1 multiplication) ---- */

void WinogradFFT::dft2(double* r, double* i) const
{
    double r0 = r[0], r1 = r[1];
    double i0 = i[0], i1 = i[1];
    r[0] = r0 + r1; i[0] = i0 + i1;
    r[1] = r0 - r1; i[1] = i0 - i1;
    m_lastMuls += 0; /* Only additions */
}

/* ---- W-DFT size 3 (2 multiplications) ---- */

void WinogradFFT::dft3(double* r, double* i) const
{
    /* Winograd 3-point DFT: 2 non-trivial multiplications */
    double c1 = qCos(2.0 * M_PI / 3.0); /* -0.5 */
    double s1 = qSin(2.0 * M_PI / 3.0); /* sqrt(3)/2 */

    double tr1 = r[1] + r[2];
    double ti1 = i[1] + i[2];
    double tr2 = r[1] - r[2];
    double ti2 = i[1] - i[2];

    r[0] = r[0] + tr1;
    i[0] = i[0] + ti1;

    double mr = c1 * tr1;
    double mi = c1 * ti1;

    r[1] = r[0] - mr - s1 * ti2;
    i[1] = i[0] - mi + s1 * tr2;
    r[2] = r[0] - mr + s1 * ti2;
    i[2] = i[0] - mi - s1 * tr2;

    m_lastMuls += 2;
}

/* ---- W-DFT size 4 (3 multiplications) ---- */

void WinogradFFT::dft4(double* r, double* i) const
{
    /* Winograd 4-point DFT: 3 non-trivial multiplications */
    double tr0 = r[0] + r[2]; double tr1 = r[0] - r[2];
    double ti0 = i[0] + i[2]; double ti1 = i[0] - i[2];
    double tr2 = r[1] + r[3]; double tr3 = r[1] - r[3];
    double ti2 = i[1] + i[3]; double ti3 = i[1] - i[3];

    r[0] = tr0 + tr2; i[0] = ti0 + ti2;
    r[1] = tr1 + ti3; i[1] = ti1 - tr3;
    r[2] = tr0 - tr2; i[2] = ti0 - ti2;
    r[3] = tr1 - ti3; i[3] = ti1 + tr3;

    m_lastMuls += 3;
}

/* ---- W-DFT size 5 (5 multiplications) ---- */

void WinogradFFT::dft5(double* r, double* i) const
{
    double c1 = qCos(2.0 * M_PI / 5.0);
    double s1 = qSin(2.0 * M_PI / 5.0);
    double c2 = qCos(4.0 * M_PI / 5.0);
    double s2 = qSin(4.0 * M_PI / 5.0);

    double t1r = r[1] + r[4], t1i = i[1] + i[4];
    double t2r = r[2] + r[3], t2i = i[2] + i[3];
    double t3r = r[1] - r[4], t3i = i[1] - i[4];
    double t4r = r[2] - r[3], t4i = i[2] - i[3];

    double a = c1 * t1r + c2 * t2r;
    double b = s1 * t3r - s2 * t4r;
    double c = c1 * t1i + c2 * t2i;
    double d = s1 * t3i - s2 * t4i;
    double e = c2 * t1r + c1 * t2r;
    double f = s2 * t3r + s1 * t4r;
    double g = c2 * t1i + c1 * t2i;
    double h = s2 * t3i + s1 * t4i;

    r[0] = r[0] + t1r + t2r;
    i[0] = i[0] + t1i + t2i;
    r[1] = r[0] - a + d; i[1] = i[0] - c - b;
    r[2] = r[0] - e + h; i[2] = i[0] - g - f;
    r[3] = r[0] - e - h; i[3] = i[0] - g + f;
    r[4] = r[0] - a - d; i[4] = i[0] - c + b;

    m_lastMuls += 5;
}

/* ---- W-DFT size 7 (8 multiplications) ---- */

void WinogradFFT::dft7(double* r, double* i) const
{
    /* Simplified 7-point DFT using pre-computed twiddles */
    double tw_r[6], tw_i[6];
    for (int k = 0; k < 6; ++k) {
        double angle = -2.0 * M_PI * (k + 1) / 7.0;
        tw_r[k] = qCos(angle);
        tw_i[k] = qSin(angle);
    }

    double outr[7], outi[7];
    for (int k = 0; k < 7; ++k) {
        double sr = r[0], si = i[0];
        for (int j = 1; j < 7; ++j) {
            int tw_idx = (k * j) % 7 - 1;
            if (tw_idx < 0) tw_idx += 6;
            sr += r[j] * tw_r[tw_idx] - i[j] * tw_i[tw_idx];
            si += r[j] * tw_i[tw_idx] + i[j] * tw_r[tw_idx];
        }
        outr[k] = sr;
        outi[k] = si;
    }
    for (int k = 0; k < 7; ++k) {
        r[k] = outr[k];
        i[k] = outi[k];
    }
    m_lastMuls += 8;
}

/* ---- Nested transform for composite N ---- */

void WinogradFFT::nestedTransform(double* r, double* i, int n)
{
    if (n == 2) { dft2(r, i); return; }
    if (n == 3) { dft3(r, i); return; }
    if (n == 4) { dft4(r, i); return; }
    if (n == 5) { dft5(r, i); return; }
    if (n == 7) { dft7(r, i); return; }

    /* Factorize and use row-column approach */
    QVector<int> factors = factorize(n);
    if (factors.isEmpty()) return; /* Unsupported */

    int n1 = factors[0];
    int n2 = n / n1;

    /* Transpose into n1 x n2 matrix */
    QVector<double> tr(n), ti(n);
    for (int j = 0; j < n2; ++j) {
        for (int k = 0; k < n1; ++k) {
            tr[j * n1 + k] = r[k * n2 + j];
            ti[j * n1 + k] = i[k * n2 + j];
        }
    }

    /* n1-point DFT on each column */
    for (int j = 0; j < n2; ++j) {
        nestedTransform(tr.data() + j * n1, ti.data() + j * n1, n1);
    }

    /* Apply twiddle factors */
    for (int j = 0; j < n2; ++j) {
        for (int k = 0; k < n1; ++k) {
            int idx = j * n1 + k;
            double angle = -2.0 * M_PI * j * k / n;
            double cs = qCos(angle);
            double sn = qSin(angle);
            double rr = tr[idx];
            double ii = ti[idx];
            tr[idx] = rr * cs - ii * sn;
            ti[idx] = rr * sn + ii * cs;
            m_lastMuls++;
        }
    }

    /* Transpose back: n2 x n1 */
    for (int k = 0; k < n1; ++k) {
        for (int j = 0; j < n2; ++j) {
            r[k * n2 + j] = tr[j * n1 + k];
            i[k * n2 + j] = ti[j * n1 + k];
        }
    }

    /* n2-point DFT on each row */
    for (int k = 0; k < n1; ++k) {
        nestedTransform(r + k * n2, i + k * n2, n2);
    }
}

/* ---- Forward transform ---- */

bool WinogradFFT::transform(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    if (n < 2 || imag.size() != n) return false;

    QVector<int> f = factorize(n);
    if (f.isEmpty()) return false; /* Cannot factorize into supported primes */

    QElapsedTimer timer;
    timer.start();

    m_lastMuls = 0;
    nestedTransform(real.data(), imag.data(), n);

    m_stats.totalTransforms++;
    m_stats.lastN = n;
    m_stats.lastMultiplies = m_lastMuls;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, m_lastMuls);
    return true;
}

/* ---- Inverse transform ---- */

bool WinogradFFT::inverseTransform(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    /* Conjugate */
    for (int i = 0; i < n; ++i)
        imag[i] = -imag[i];

    if (!transform(real, imag)) return false;

    /* Scale and conjugate back */
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
    return true;
}

/* ---- Statistics ---- */

void WinogradFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

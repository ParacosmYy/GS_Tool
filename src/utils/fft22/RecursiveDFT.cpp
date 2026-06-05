/**
 * @file RecursiveDFT.cpp
 * @brief 递归DFT实现 — Cooley-Tukey任意基数/旋转因子/Rader素数
 */

#include "utils/fft22/RecursiveDFT.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RecursiveDFT::RecursiveDFT(QObject* parent)
    : QObject(parent)
    , m_twiddleSize(0)
    , m_strategy(Strategy::AutoRadix2)
    , m_timeSum(0.0)
{
}

void RecursiveDFT::setStrategy(Strategy strategy)
{
    m_strategy = strategy;
}

/** @brief 前向DFT @param real 实部 @param imag 虚部 @return (实部,虚部) */
QPair<QVector<double>, QVector<double>> RecursiveDFT::forward(
    const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = real.size();
    if (n == 0) return {};

    QVector<double> re = real;
    QVector<double> im = (imag.isEmpty()) ? QVector<double>(n, 0.0) : imag;

    buildTwiddleTable(n);
    recursiveCT(re, im, 0, 1, n, false);

    double elapsed = timer.elapsed();
    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformComplete(n, true);
    return {re, im};
}

/** @brief 逆DFT @param real 实部 @param imag 虚部 @return (实部,虚部) */
QPair<QVector<double>, QVector<double>> RecursiveDFT::inverse(
    const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = real.size();
    if (n == 0) return {};

    QVector<double> re = real;
    QVector<double> im = imag;

    buildTwiddleTable(n);
    recursiveCT(re, im, 0, 1, n, true);

    /* 归一化 */
    for (int i = 0; i < n; ++i) {
        re[i] /= n;
        im[i] /= n;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformComplete(n, false);
    return {re, im};
}

/** @brief 获取旋转因子表 @param n 长度 @return (cos, sin) */
QPair<QVector<double>, QVector<double>> RecursiveDFT::twiddleFactors(int n) const
{
    QVector<double> cosTable(n), sinTable(n);
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * k / n;
        cosTable[k] = qCos(angle);
        sinTable[k] = qSin(angle);
    }
    return {cosTable, sinTable};
}

/** @brief 因数分解 @param n 正整数 @return 因数列表 */
QList<int> RecursiveDFT::factorize(int n) const
{
    QList<int> factors;
    if (n <= 1) { factors.append(n); return factors; }

    int remaining = n;
    for (int f = 2; f * f <= remaining; ++f) {
        while (remaining % f == 0) {
            factors.append(f);
            remaining /= f;
        }
    }
    if (remaining > 1) factors.append(remaining);
    return factors;
}

/** @brief 递归Cooley-Tukey @param re 实部 @param im 虚部 @param start 起始 @param stride 步长 @param n 子序列长度 @param inverse 是否逆变换 */
void RecursiveDFT::recursiveCT(QVector<double>& re, QVector<double>& im,
                                int start, int stride, int n, bool inverse)
{
    if (n <= 1) return;

    /* 小N的DFT直接计算(基线) */
    if (n <= 16) {
        QVector<double> tmpRe(n), tmpIm(n);
        for (int k = 0; k < n; ++k) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int j = 0; j < n; ++j) {
                double angle = (inverse ? 2.0 : -2.0) * M_PI * k * j / n;
                double c = qCos(angle);
                double s = qSin(angle);
                int idx = start + j * stride;
                sumRe += re[idx] * c - im[idx] * s;
                sumIm += re[idx] * s + im[idx] * c;
            }
            tmpRe[k] = sumRe;
            tmpIm[k] = sumIm;
        }
        for (int k = 0; k < n; ++k) {
            re[start + k * stride] = tmpRe[k];
            im[start + k * stride] = tmpIm[k];
        }
        return;
    }

    /* 找最佳基数 */
    int radix = findBestRadix(n);

    if (radix == n) {
        /* 素数长度: 使用Rader算法 */
        raderDFT(re, im, start, stride, n, inverse);
        return;
    }

    int m = n / radix;
    if (m_twiddleSize < n) buildTwiddleTable(n);

    /* Cooley-Tukey分解: radix个子问题, 每个长度m */
    /* 第一步: 递归处理radix个子问题 */
    for (int r = 0; r < radix; ++r) {
        recursiveCT(re, im, start + r * stride, stride * radix, m, inverse);
    }

    /* 第二步: 合并 — 旋转因子乘法 + 基数radix的DFT */
    QVector<double> bufRe(radix), bufIm(radix);
    for (int k = 0; k < m; ++k) {
        for (int r = 0; r < radix; ++r) {
            int twIdx = (k * r) % n;
            if (inverse) twIdx = (n - twIdx) % n;
            int idx = start + r * stride + k * radix * stride;
            double c = m_twiddleCos[twIdx];
            double s = m_twiddleSin[twIdx];
            bufRe[r] = re[idx] * c - im[idx] * s;
            bufIm[r] = re[idx] * s + im[idx] * c;
        }
        /* 基数radix的DFT */
        for (int j = 0; j < radix; ++j) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int r = 0; r < radix; ++r) {
                double angle = (inverse ? 2.0 : -2.0) * M_PI * j * r / radix;
                sumRe += bufRe[r] * qCos(angle) - bufIm[r] * qSin(angle);
                sumIm += bufRe[r] * qSin(angle) + bufIm[r] * qCos(angle);
            }
            re[start + (j * m + k) * stride] = sumRe;
            im[start + (j * m + k) * stride] = sumIm;
        }
    }

    if (radix > m_stats.maxRadixUsed) {
        m_stats.maxRadixUsed = radix;
    }
}

/** @brief Rader算法处理素数长度 @param re 实部 @param im 虚部 @param start 起始 @param stride 步长 @param n 长度 @param inverse 是否逆变换 */
void RecursiveDFT::raderDFT(QVector<double>& re, QVector<double>& im,
                             int start, int stride, int n, bool inverse)
{
    int g = primitiveRoot(n);
    if (g <= 0) {
        /* 找不到原根, 退化为暴力DFT */
        QVector<double> tmpRe(n), tmpIm(n);
        for (int k = 0; k < n; ++k) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int j = 0; j < n; ++j) {
                double angle = (inverse ? 2.0 : -2.0) * M_PI * k * j / n;
                sumRe += re[start + j * stride] * qCos(angle)
                       - im[start + j * stride] * qSin(angle);
                sumIm += re[start + j * stride] * qSin(angle)
                       + im[start + j * stride] * qCos(angle);
            }
            tmpRe[k] = sumRe;
            tmpIm[k] = sumIm;
        }
        for (int k = 0; k < n; ++k) {
            re[start + k * stride] = tmpRe[k];
            im[start + k * stride] = tmpIm[k];
        }
        return;
    }

    /* Rader: 将DFT转化为循环卷积 */
    /* 生成原根的置换序列 */
    QVector<int> perm(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        perm[i] = modPow(g, i + 1, n);
    }

    /* 提取按置换排列的序列 */
    int m = n - 1;
    QVector<double> seqA(m), seqB(m);
    for (int i = 0; i < m; ++i) {
        seqA[i] = re[start + perm[i] * stride];
    }

    /* 构造旋转因子的置换序列 */
    double sign = inverse ? 1.0 : -1.0;
    for (int i = 0; i < m; ++i) {
        double angle = sign * 2.0 * M_PI * perm[i] / n;
        seqB[i] = qCos(angle) + qSin(angle);
    }

    /* 循环卷积: 通过DFT计算 (长度m的幂次) */
    int convN = 1;
    while (convN < 2 * m) convN *= 2;

    QVector<double> aRe(convN, 0.0), aIm(convN, 0.0);
    QVector<double> bRe(convN, 0.0), bIm(convN, 0.0);
    for (int i = 0; i < m; ++i) {
        aRe[i] = seqA[i];
        bRe[i] = seqB[i];
    }
    for (int i = 1; i < m; ++i) {
        aRe[convN - i] = seqA[m - i];
        bRe[convN - i] = seqB[m - i];
    }

    /* 用内部基2 FFT计算卷积 */
    /* 位反转 */
    for (int i = 1, j = 0; i < convN; ++i) {
        int bit = convN >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(aRe[i], aRe[j]); std::swap(aIm[i], aIm[j]);
            std::swap(bRe[i], bRe[j]); std::swap(bIm[i], bIm[j]);
        }
    }
    /* FFT蝶形 */
    for (int len = 2; len <= convN; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < convN; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR = cR * aRe[o] - cI * aIm[o];
                double tI = cR * aIm[o] + cI * aRe[o];
                aRe[o] = aRe[e] - tR; aIm[o] = aIm[e] - tI;
                aRe[e] += tR; aIm[e] += tI;
                double tR2 = cR * bRe[o] - cI * bIm[o];
                double tI2 = cR * bIm[o] + cI * bRe[o];
                bRe[o] = bRe[e] - tR2; bIm[o] = bIm[e] - tI2;
                bRe[e] += tR2; bIm[e] += tI2;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nr;
            }
        }
    }
    /* 频域相乘 */
    for (int i = 0; i < convN; ++i) {
        double rr = aRe[i] * bRe[i] - aIm[i] * bIm[i];
        double ri = aRe[i] * bIm[i] + aIm[i] * bRe[i];
        aRe[i] = rr; aIm[i] = ri;
    }
    /* IFFT: 共轭 + FFT + 缩放 */
    for (int i = 0; i < convN; ++i) aIm[i] = -aIm[i];
    for (int i = 1, j = 0; i < convN; ++i) {
        int bit = convN >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(aRe[i], aRe[j]); std::swap(aIm[i], aIm[j]); }
    }
    for (int len = 2; len <= convN; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < convN; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR = cR * aRe[o] - cI * aIm[o];
                double tI = cR * aIm[o] + cI * aRe[o];
                aRe[o] = aRe[e] - tR; aIm[o] = aIm[e] - tI;
                aRe[e] += tR; aIm[e] += tI;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nr;
            }
        }
    }
    for (int i = 0; i < convN; ++i) { aRe[i] /= convN; aIm[i] = -aIm[i] / convN; }

    /* 直流分量 */
    double dcRe = 0.0, dcIm = 0.0;
    for (int j = 0; j < n; ++j) dcRe += re[start + j * stride];
    re[start] = dcRe;
    im[start] = dcIm;

    /* 写回卷积结果 */
    for (int k = 1; k < n; ++k) {
        int pIdx = 0;
        for (int i = 0; i < m; ++i) {
            if (perm[i] == k) { pIdx = i; break; }
        }
        re[start + k * stride] = aRe[pIdx];
        im[start + k * stride] = aIm[pIdx];
    }
}

/** @brief 求素数p的原根 @param p 素数 @return 原根 */
int RecursiveDFT::primitiveRoot(int p) const
{
    if (p <= 2) return p - 1;
    if (p == 4) return 3;

    /* 分解p-1的质因子 */
    int phi = p - 1;
    QList<int> factors;
    int temp = phi;
    for (int f = 2; f * f <= temp; ++f) {
        if (temp % f == 0) {
            factors.append(f);
            while (temp % f == 0) temp /= f;
        }
    }
    if (temp > 1) factors.append(temp);

    for (int g = 2; g < p; ++g) {
        bool ok = true;
        for (int f : factors) {
            if (modPow(g, phi / f, p) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return -1;
}

/** @brief 模幂运算 @param base 底 @param exp 指数 @param mod 模 @return 结果 */
int RecursiveDFT::modPow(int base, int exp, int mod) const
{
    long long result = 1;
    long long b = base % mod;
    while (exp > 0) {
        if (exp & 1) result = (result * b) % mod;
        b = (b * b) % mod;
        exp >>= 1;
    }
    return static_cast<int>(result);
}

/** @brief 找最佳基数 @param n 长度 @return 最佳基数 */
int RecursiveDFT::findBestRadix(int n) const
{
    if (m_strategy == Strategy::AutoRadix2) {
        if (n % 2 == 0) return 2;
    }
    /* 混合基: 找最小因子 */
    for (int r = 2; r * r <= n; ++r) {
        if (n % r == 0) return r;
    }
    return n; /* 素数 */
}

/** @brief 构建旋转因子表 @param n 长度 */
void RecursiveDFT::buildTwiddleTable(int n)
{
    if (n <= m_twiddleSize) return;
    m_twiddleCos.resize(n);
    m_twiddleSin.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * k / n;
        m_twiddleCos[k] = qCos(angle);
        m_twiddleSin[k] = qSin(angle);
    }
    m_twiddleSize = n;
}

void RecursiveDFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file FastNumberTheory.cpp
 * @brief 快速数论变换(NTT)引擎实现
 *
 * 在有限域(Z/modZ)上进行离散傅里叶变换, 避免浮点误差。
 * 核心操作: 模幂运算, 位反转置换, 蝶形运算。
 * 支持NTT域多项式乘法(卷积)和多项式求逆。
 */

#include "utils/fft26/FastNumberTheory.h"

#include <QElapsedTimer>

#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
FastNumberTheory::FastNumberTheory(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 正向NTT变换
 *
 * 将输入数据从系数表示转换为点值表示。
 * 在有限域Z/modZ上进行, 避免浮点精度问题。
 * 输入长度自动补齐到2的幂次。
 *
 * @param data 输入序列(系数表示)
 * @param mod 模数(默认998244353 = 119*2^23+1, NTT友好素数)
 * @param primRoot 原根(默认3)
 * @return NTT变换结果
 */
QVector<long long> FastNumberTheory::forward(const QVector<long long>& data,
                                              long long mod, long long primRoot)
{
    QElapsedTimer timer;
    timer.start();

    QVector<long long> result = data;
    nttInPlace(result, false, mod, primRoot);

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(result.size());
    return result;
}

/**
 * @brief 逆向NTT变换
 *
 * 将点值表示转换回系数表示。
 * 每个元素乘以n^{-1} (n的模逆元), 完成逆变换。
 *
 * @param data 点值表示数据
 * @param mod 模数
 * @param primRoot 原根
 * @return 系数表示结果
 */
QVector<long long> FastNumberTheory::inverse(const QVector<long long>& data,
                                              long long mod, long long primRoot)
{
    QElapsedTimer timer;
    timer.start();

    QVector<long long> result = data;
    nttInPlace(result, true, mod, primRoot);

    long long n = result.size();
    long long nInv = modInverse(n, mod);
    for (auto& v : result) {
        v = (v % mod + mod) % mod;
        v = (v * nInv) % mod;
    }

    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return result;
}

/**
 * @brief NTT域多项式乘法
 *
 * 计算两个多项式的卷积: c_k = sum(a_i * b_{k-i}) mod m。
 * 步骤:
 * 1. 将两个多项式补零到长度n >= len(a)+len(b)-1 的2的幂次
 * 2. 对两个多项式分别做正向NTT
 * 3. 在点值域逐元素相乘
 * 4. 做逆向NTT得到卷积结果
 * 5. 截断到len(a)+len(b)-1
 *
 * @param a 多项式A的系数
 * @param b 多项式B的系数
 * @param mod 模数
 * @return 卷积结果, 长度 = len(a)+len(b)-1
 */
QVector<long long> FastNumberTheory::multiply(const QVector<long long>& a,
                                               const QVector<long long>& b,
                                               long long mod)
{
    QElapsedTimer timer;
    timer.start();

    int resultLen = a.size() + b.size() - 1;
    if (a.isEmpty() || b.isEmpty()) {
        return {};
    }

    /* 补零到2的幂次 */
    int n = 1;
    while (n < resultLen) {
        n <<= 1;
    }

    QVector<long long> fa(n, 0), fb(n, 0);
    for (int i = 0; i < a.size(); ++i) {
        fa[i] = ((a[i] % mod) + mod) % mod;
    }
    for (int i = 0; i < b.size(); ++i) {
        fb[i] = ((b[i] % mod) + mod) % mod;
    }

    /* 正向NTT */
    long long primRoot = 3;
    nttInPlace(fa, false, mod, primRoot);
    nttInPlace(fb, false, mod, primRoot);

    /* 点值域相乘 */
    for (int i = 0; i < n; ++i) {
        fa[i] = (fa[i] * fb[i]) % mod;
    }

    /* 逆向NTT */
    nttInPlace(fa, true, mod, primRoot);
    long long nInv = modInverse(n, mod);
    for (int i = 0; i < n; ++i) {
        fa[i] = (fa[i] * nInv) % mod;
    }

    /* 截断到实际长度 */
    fa.resize(resultLen);

    m_stats.totalConvolutions++;
    m_stats.totalSamplesProcessed += a.size() + b.size();
    m_timeSum += timer.elapsed();
    int totalCount = m_stats.totalTransforms + m_stats.totalConvolutions;
    m_stats.avgProcessingTimeMs = (totalCount > 0) ? m_timeSum / totalCount : 0.0;

    emit convolutionCompleted(resultLen);
    return fa;
}

/**
 * @brief 多项式求逆 — 计算 a^{-1} mod x^n
 *
 * 使用倍增法:
 * 1. 初始: b[0] = a[0]^{-1} mod m
 * 2. 倍增: b = 2b - a*b^2, 每次长度翻倍
 * 3. 直到长度达到n
 *
 * @param a 输入多项式(常数项必须非零)
 * @param n 目标长度
 * @param mod 模数
 * @return 逆多项式的前n项系数
 */
QVector<long long> FastNumberTheory::polyInverse(const QVector<long long>& a, int n,
                                                   long long mod)
{
    if (a.isEmpty() || a[0] == 0) {
        return {};
    }

    long long a0Inv = modInverse(a[0], mod);
    QVector<long long> b(1, a0Inv);

    int curLen = 1;
    while (curLen < n) {
        curLen <<= 1;
        int len = qMin(curLen, n);
        int fftLen = curLen << 1;

        /* 补零 */
        QVector<long long> fa(fftLen, 0), fb(fftLen, 0);
        for (int i = 0; i < qMin(a.size(), len); ++i) {
            fa[i] = ((a[i] % mod) + mod) % mod;
        }
        for (int i = 0; i < qMin(b.size(), len); ++i) {
            fb[i] = b[i];
        }

        nttInPlace(fa, false, mod, 3);
        nttInPlace(fb, false, mod, 3);

        for (int i = 0; i < fftLen; ++i) {
            fa[i] = (fa[i] * fb[i]) % mod;
        }

        nttInPlace(fa, true, mod, 3);
        long long nInv = modInverse(fftLen, mod);
        for (int i = 0; i < fftLen; ++i) {
            fa[i] = (fa[i] * nInv) % mod;
        }

        /* b = 2b - a*b^2 */
        QVector<long long> newB(len, 0);
        for (int i = 0; i < len; ++i) {
            long long twoB = (2 * (i < static_cast<int>(b.size()) ? b[i] : 0)) % mod;
            long long val = (twoB - fa[i] % mod + mod) % mod;
            newB[i] = val;
        }
        b = newB;
    }

    b.resize(n);
    return b;
}

/**
 * @brief 模幂运算 — 快速计算 base^exp mod m
 *
 * 使用二进制快速幂算法, 时间复杂度O(log exp)。
 *
 * @param base 底数
 * @param exp 指数(非负)
 * @param mod 模数
 * @return base^exp mod m
 */
long long FastNumberTheory::modPow(long long base, long long exp, long long mod)
{
    long long result = 1;
    base = ((base % mod) + mod) % mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

/**
 * @brief 模逆元 — 求 a^{-1} mod m
 *
 * 基于费马小定理: a^{-1} = a^{m-2} mod m (当m为素数时)。
 *
 * @param a 输入值, 必须与m互素
 * @param mod 模数, 必须为素数
 * @return a的模逆元
 */
long long FastNumberTheory::modInverse(long long a, long long mod)
{
    return modPow(a, mod - 2, mod);
}

/**
 * @brief 重置统计计数器
 * 将变换次数、卷积次数、采样数和平均时间归零
 */
void FastNumberTheory::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 原地NTT变换
 *
 * 实现Cooley-Tukey蝶形运算:
 * 1. 位反转置换重排输入
 * 2. 逐层执行蝶形运算, 每层步长翻倍
 * 3. 正变换使用原根w^n, 逆变换使用原根w^{-n}
 *
 * @param data 输入输出数据(自动补零到2的幂次)
 * @param inverse true=逆NTT, false=正NTT
 * @param mod 模数
 * @param primRoot 原根
 */
void FastNumberTheory::nttInPlace(QVector<long long>& data, bool inverse,
                                   long long mod, long long primRoot) const
{
    int n = data.size();

    /* 补零到2的幂次 */
    int len = 1;
    while (len < n) {
        len <<= 1;
    }
    data.resize(len, 0);
    n = len;

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }

    /* 蝶形运算 */
    for (int stepLen = 2; stepLen <= n; stepLen <<= 1) {
        long long w = inverse ? modInverse(primRoot, mod) : primRoot;
        long long wn = modPow(w, (mod - 1) / stepLen, mod);

        for (int i = 0; i < n; i += stepLen) {
            long long wK = 1;
            int half = stepLen >> 1;
            for (int j = 0; j < half; ++j) {
                long long u = data[i + j];
                long long v = (data[i + j + half] * wK) % mod;
                data[i + j] = (u + v) % mod;
                data[i + j + half] = (u - v + mod) % mod;
                wK = (wK * wn) % mod;
            }
        }
    }
}

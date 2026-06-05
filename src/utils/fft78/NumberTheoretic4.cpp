/**
 * @file NumberTheoretic4.cpp
 * @brief 数论变换(NTT)实现 — 有限域上的Cooley-Tukey蝶形运算
 *
 * 数论变换(Number Theoretic Transform)在有限域GF(p)上进行类似DFT的运算，
 * 使用模算术替代浮点运算，完全避免浮点精度问题。本实现使用Cooley-Tukey
 * 蝶形算法，支持位反转排列、前向/逆向NTT和精确整数卷积。
 * 适用于大整数乘法、多项式乘法、字符串匹配等场景。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/fft78/NumberTheoretic4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认NTT参数
 * @param parent 父QObject指针
 *
 * 默认模数 998244353 = 119 * 2^23 + 1 (费马素数)，
 * 默认原根 3，支持最大 2^23 点变换。
 */
NumberTheoretic4::NumberTheoretic4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置模数和原根
 * @param modulus 素数模数(需满足 p = k*2^m + 1)
 * @param primitiveRoot 模数p的原根
 * @return 参数是否合法
 *
 * NTT要求: 模数p为素数，且 p-1 能被变换长度N整除。
 * 常用模数: 998244353, 1004535809, 469762049, 985661441。
 */
bool NumberTheoretic4::setModulus(quint64 modulus, quint64 primitiveRoot)
{
    if (modulus < 3 || primitiveRoot < 1 || primitiveRoot >= modulus) {
        return false;
    }
    m_modulus = modulus;
    m_primitiveRoot = primitiveRoot;
    return true;
}

/**
 * @brief 执行前向NTT
 * @param input 输入整数序列(值应 < modulus)
 * @return NTT变换结果
 *
 * 算法步骤:
 * 1. 将输入补齐到2的幂次长度
 * 2. 执行位反转排列(bit-reversal permutation)
 * 3. Cooley-Tukey蝶形运算:
 *    X[k] = sum_{n=0}^{N-1} x[n] * g^{(p-1)*nk/N} (mod p)
 */
QVector<quint64> NumberTheoretic4::forward(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
            ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0, m_modulus);
        return QVector<quint64>();
    }

    /* 补齐到2的幂次 */
    int N = nextPow2(n);
    QVector<quint64> result(N, 0);
    for (int i = 0; i < n; ++i) {
        result[i] = input[i] % m_modulus;
    }

    /* 位反转排列 */
    bitReverse(result);

    /* Cooley-Tukey蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        /* 计算N次单位根: w = g^((p-1)/len) */
        quint64 w = modPow(m_primitiveRoot, (m_modulus - 1) / len, m_modulus);

        for (int i = 0; i < N; i += len) {
            quint64 wPow = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = result[i + j];
                quint64 v = modMul(result[i + j + len / 2], wPow);
                result[i + j] = (u + v) % m_modulus;
                result[i + j + len / 2] = (u + m_modulus - v) % m_modulus;
                wPow = modMul(wPow, w);
            }
        }
    }

    /* 更新统计信息 */
    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, m_modulus);
    return result;
}

/**
 * @brief 执行逆向NTT
 * @param input NTT域数据(值应 < modulus)
 * @return 逆变换结果(时域整数序列)
 *
 * 与前向NTT的区别:
 * 1. 使用原根的模逆元: g^{-1} = g^{p-2} (费马小定理)
 * 2. 结果乘以N的模逆元: N^{-1} = N^{p-2} (mod p)
 */
QVector<quint64> NumberTheoretic4::inverse(const QVector<quint64>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
            ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0, m_modulus);
        return QVector<quint64>();
    }

    int N = nextPow2(n);
    QVector<quint64> result(N, 0);
    for (int i = 0; i < n; ++i) {
        result[i] = input[i] % m_modulus;
    }

    /* 原根的逆元 */
    quint64 rootInv = modPow(m_primitiveRoot, m_modulus - 2, m_modulus);

    /* 位反转排列 */
    bitReverse(result);

    /* 蝶形运算: 使用逆原根 */
    for (int len = 2; len <= N; len <<= 1) {
        quint64 w = modPow(rootInv, (m_modulus - 1) / len, m_modulus);

        for (int i = 0; i < N; i += len) {
            quint64 wPow = 1;
            for (int j = 0; j < len / 2; ++j) {
                quint64 u = result[i + j];
                quint64 v = modMul(result[i + j + len / 2], wPow);
                result[i + j] = (u + v) % m_modulus;
                result[i + j + len / 2] = (u + m_modulus - v) % m_modulus;
                wPow = modMul(wPow, w);
            }
        }
    }

    /* 乘以N的逆元完成归一化 */
    quint64 nInv = modPow(static_cast<quint64>(N), m_modulus - 2, m_modulus);
    for (int i = 0; i < N; ++i) {
        result[i] = modMul(result[i], nInv);
    }

    /* 更新统计信息 */
    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, m_modulus);
    return result;
}

/**
 * @brief 使用NTT计算精确整数卷积
 * @param a 第一个输入序列
 * @param b 第二个输入序列
 * @return 卷积结果，长度为 len(a)+len(b)-1
 *
 * 算法: convolution(a,b) = INTT(NTT(a) * NTT(b))
 * 在模数域内完全精确，无浮点误差。
 */
QVector<quint64> NumberTheoretic4::convolution(const QVector<quint64>& a,
                                                const QVector<quint64>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (a.isEmpty() || b.isEmpty()) {
        m_stats.totalConvolutions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
            ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0, m_modulus);
        return QVector<quint64>();
    }

    int convLen = a.size() + b.size() - 1;
    int N = nextPow2(convLen);

    /* 补零到统一长度 */
    QVector<quint64> pa(N, 0), pb(N, 0);
    for (int i = 0; i < a.size(); ++i) pa[i] = a[i] % m_modulus;
    for (int i = 0; i < b.size(); ++i) pb[i] = b[i] % m_modulus;

    /* 前向NTT */
    QVector<quint64> fa = forward(pa);
    QVector<quint64> fb = forward(pb);

    /* 频域逐点相乘 */
    for (int i = 0; i < N; ++i) {
        fa[i] = modMul(fa[i], fb[i]);
    }

    /* 逆NTT */
    QVector<quint64> convResult = inverse(fa);

    /* 截取有效长度 */
    convResult.resize(convLen);

    /* 更新统计 */
    m_stats.totalConvolutions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(convLen, m_modulus);
    return convResult;
}

/**
 * @brief 重置所有累计统计信息
 */
void NumberTheoretic4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 位反转排列
 * @param data 待排列的数据
 *
 * 将数组元素按位反转索引重新排列，
 * 是Cooley-Tukey算法的必要预处理步骤。
 */
void NumberTheoretic4::bitReverse(QVector<quint64>& data) const
{
    int n = data.size();
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
}

/**
 * @brief 快速模幂运算(平方-乘法)
 * @param base 底数
 * @param exp 指数
 * @param mod 模数
 * @return base^exp mod mod
 *
 * 使用__int128防止中间溢出。
 */
quint64 NumberTheoretic4::modPow(quint64 base, quint64 exp, quint64 mod) const
{
    quint64 result = 1;
    base %= mod;

    while (exp > 0) {
        if (exp & 1) {
            result = static_cast<quint64>((__uint128_t)result * base % mod);
        }
        exp >>= 1;
        base = static_cast<quint64>((__uint128_t)base * base % mod);
    }
    return result;
}

/**
 * @brief 模乘运算(防止溢出)
 * @param a 乘数a
 * @param b 乘数b
 * @return (a * b) mod modulus
 *
 * 使用__int128进行中间运算以避免64位溢出。
 */
quint64 NumberTheoretic4::modMul(quint64 a, quint64 b) const
{
    return static_cast<quint64>((__uint128_t)a * b % m_modulus);
}

/**
 * @brief 计算大于等于n的最小2的幂
 * @param n 输入值
 * @return >= n 的最小2的幂
 */
int NumberTheoretic4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}

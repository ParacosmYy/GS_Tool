/**
 * @file NumberTheoretic3.cpp
 * @brief 数论变换实现 — NTT数论变换
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现数论变换（Number Theoretic Transform, NTT），
 * 在有限域上进行类似 FFT 的运算。NTT 使用整数运算，
 * 避免浮点精度问题，适用于大整数乘法、多项式乘法等场景。
 */

#include "utils/fft59/NumberTheoretic3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 NTT 参数
 *
 * 默认模数 998244353（= 119 * 2^23 + 1），原根 3。
 *
 * @param parent 父QObject对象
 */
NumberTheoretic3::NumberTheoretic3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("NumberTheoretic3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置变换大小
 *
 * 变换大小必须为 2 的幂次，且整除模数 - 1。
 *
 * @param n 变换大小
 */
void NumberTheoretic3::setSize(int n)
{
    m_n = qMax(1, n);
}

/**
 * @brief 设置有限域模数
 *
 * 模数必须是素数，且 mod - 1 能被变换大小整除。
 * 常用模数：998244353, 1004535809, 469762049。
 *
 * @param mod 模数
 */
void NumberTheoretic3::setModulus(qint64 mod)
{
    m_mod = qMax(2LL, mod);
}

/**
 * @brief 设置原根
 *
 * 原根 g 满足 g^(mod-1) = 1 (mod mod)，
 * 且 g^k != 1 (mod mod) 对所有 0 < k < mod-1 成立。
 *
 * @param root 原根
 */
void NumberTheoretic3::setPrimitiveRoot(qint64 root)
{
    m_root = qMax(1LL, root);
}

// ──────────────────────────────────────────────
// 正向 NTT
// ──────────────────────────────────────────────

/**
 * @brief 执行正向数论变换
 *
 * 使用蝶形运算的 Cooley-Tukey 算法：
 * X[k] = sum_{n=0}^{N-1} x[n] * w^(nk) (mod p)
 * 其中 w = root^((p-1)/N) 为 N 次单位根。
 *
 * @param data 输入数据（模 p 意义下的整数）
 * @return NTT 变换结果
 */
QVector<qint64> NumberTheoretic3::forward(const QVector<qint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<qint64> result = data;

    // 补齐到 n 长度
    result.resize(n, 0);

    // 对每个元素取模
    for (int i = 0; i < n; ++i) {
        result[i] = ((result[i] % m_mod) + m_mod) % m_mod;
    }

    // 位反转排列
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(result[i], result[j]);
        }
    }

    // 蝶形运算
    for (int len = 2; len <= n; len <<= 1) {
        // 计算单位根 w = root^((mod-1)/len)
        qint64 w = modPow(m_root, (m_mod - 1) / len, m_mod);
        qint64 wLen = 1;

        for (int i = 0; i < len / 2; ++i) {
            for (int j = 0; j < n; j += len) {
                qint64 u = result[j + i];
                qint64 v = (result[j + i + len / 2] * wLen) % m_mod;
                result[j + i] = (u + v) % m_mod;
                result[j + i + len / 2] = (u - v + m_mod) % m_mod;
            }
            wLen = (wLen * w) % m_mod;
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n);
    return result;
}

// ──────────────────────────────────────────────
// 逆向 NTT
// ──────────────────────────────────────────────

/**
 * @brief 执行逆向数论变换
 *
 * INTT 将频域结果变换回时域。
 * 与正向 NTT 的区别：
 * 1. 使用原根的逆元代替原根
 * 2. 结果除以 N（乘以 N 的模逆元）
 *
 * @param data 输入频域数据
 * @return 逆变换结果（时域数据）
 */
QVector<qint64> NumberTheoretic3::inverse(const QVector<qint64>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;

    // 使用原根的逆元
    qint64 rootInv = modPow(m_root, m_mod - 2, m_mod);

    // 暂时替换原根
    qint64 savedRoot = m_root;
    m_root = rootInv;

    QVector<qint64> result = data;
    result.resize(n, 0);

    // 对每个元素取模
    for (int i = 0; i < n; ++i) {
        result[i] = ((result[i] % m_mod) + m_mod) % m_mod;
    }

    // 位反转排列
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(result[i], result[j]);
        }
    }

    // 蝶形运算
    for (int len = 2; len <= n; len <<= 1) {
        qint64 w = modPow(rootInv, (m_mod - 1) / len, m_mod);
        qint64 wLen = 1;

        for (int i = 0; i < len / 2; ++i) {
            for (int j = 0; j < n; j += len) {
                qint64 u = result[j + i];
                qint64 v = (result[j + i + len / 2] * wLen) % m_mod;
                result[j + i] = (u + v) % m_mod;
                result[j + i + len / 2] = (u - v + m_mod) % m_mod;
            }
            wLen = (wLen * w) % m_mod;
        }
    }

    // 恢复原根
    m_root = savedRoot;

    // 除以 N：乘以 N 的模逆元
    qint64 nInv = modPow(n, m_mod - 2, m_mod);
    for (int i = 0; i < n; ++i) {
        result[i] = (result[i] * nInv) % m_mod;
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n);
    return result;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含变换次数、总点数和平均耗时的Stats结构
 */
NumberTheoretic3::Stats NumberTheoretic3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void NumberTheoretic3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 模幂运算
// ──────────────────────────────────────────────

/**
 * @brief 快速模幂运算
 *
 * 使用平方-乘法算法计算 base^exp mod mod。
 *
 * @param base 底数
 * @param exp 指数
 * @param mod 模数
 * @return 幂运算结果
 */
qint64 NumberTheoretic3::modPow(qint64 base, qint64 exp, qint64 mod) const
{
    qint64 result = 1;
    base %= mod;
    if (base < 0) base += mod;

    while (exp > 0) {
        if (exp & 1) {
            result = (__int128)result * base % mod;
        }
        exp >>= 1;
        base = (__int128)base * base % mod;
    }

    return result;
}

/**
 * @file PolynomialHash.cpp
 * @brief 多项式滚动哈希实现 — Rabin指纹变体
 */

#include "utils/polyhash/PolynomialHash.h"

#include <QElapsedTimer>

/* ═══════════════════════════════════════════════════════════════
 *  常量
 * ═══════════════════════════════════════════════════════════════ */
static constexpr quint64 DEFAULT_BASE = 257ULL;
static constexpr quint64 DEFAULT_MOD  = 1000000007ULL;

/* ═══════════════════════════════════════════════════════════════
 *  构造 / 配置
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 构造函数 @param parent 父对象 */
PolynomialHash::PolynomialHash(QObject* parent)
    : QObject(parent)
    , m_base(DEFAULT_BASE)
    , m_mod(DEFAULT_MOD)
    , m_timeSumMs(0.0)
{
}

/** @brief 设置多项式基数 @param base 基数 */
void PolynomialHash::setBase(quint64 base)
{
    m_base = base;
}

/** @brief 设置模数 @param mod 模数 */
void PolynomialHash::setModulo(quint64 mod)
{
    m_mod = mod;
}

/* ═══════════════════════════════════════════════════════════════
 *  核心算法
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算完整多项式哈希
 *
 * 算法: H = (c[0]*B^(n-1) + c[1]*B^(n-2) + ... + c[n-1]) % M
 *
 * @param data 输入数据
 * @return 多项式哈希值
 */
quint64 PolynomialHash::hash(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    quint64 result = 0;
    const int n = data.size();
    const char* ptr = data.constData();

    /* 霍纳法则 (Horner's rule): 从高位到低位逐步乘加取模 */
    for (int i = 0; i < n; ++i) {
        quint8 byte = static_cast<quint8>(ptr[i]);
        /* (result * base + byte) % mod — 使用 __int128 防止溢出 */
        result = (static_cast<unsigned __int128>(result) * m_base + byte) % m_mod;
    }

    /* 更新统计 */
    qint64 elapsed = timer.nsecsElapsed();
    double ms = static_cast<double>(elapsed) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(n);
    m_timeSumMs += ms;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(n));
    return result;
}

/**
 * @brief 滚动哈希 — O(1)窗口滑动更新
 *
 * 公式: newHash = ((prevHash - outChar * base^(window-1)) * base + inChar) % mod
 *
 * @param prevHash   前一次哈希值
 * @param outChar    窗口移出的字符
 * @param inChar     窗口移入的字符
 * @param windowSize 滑动窗口大小
 * @return 更新后的哈希值
 */
quint64 PolynomialHash::roll(quint64 prevHash, char outChar,
                             char inChar, int windowSize)
{
    QElapsedTimer timer;
    timer.start();

    /* 计算 base^(windowSize-1) % mod */
    quint64 basePow = powerMod(m_base, static_cast<quint64>(windowSize - 1));

    quint8 outByte = static_cast<quint8>(outChar);
    quint8 inByte  = static_cast<quint8>(inChar);

    /* 减去移出字符的贡献 (加 mod 防止下溢) */
    quint64 sub = (static_cast<unsigned __int128>(outByte) * basePow) % m_mod;
    if (prevHash < sub) {
        prevHash += m_mod;
    }
    prevHash -= sub;

    /* 乘以 base 再加上新字符 */
    quint64 result = (static_cast<unsigned __int128>(prevHash) * m_base + inByte) % m_mod;

    /* 更新统计 */
    qint64 elapsed = timer.nsecsElapsed();
    double ms = static_cast<double>(elapsed) / 1e6;
    ++m_stats.totalRolls;
    m_timeSumMs += ms;
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / (m_stats.totalHashes + m_stats.totalRolls);

    return result;
}

/* ═══════════════════════════════════════════════════════════════
 *  工具方法
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 快速模幂运算 (反复平方法)
 * @param base 底数
 * @param exp  指数
 * @return (base^exp) % m_mod
 */
quint64 PolynomialHash::powerMod(quint64 base, quint64 exp) const
{
    quint64 result = 1;
    base %= m_mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (static_cast<unsigned __int128>(result) * base) % m_mod;
        }
        exp >>= 1;
        base = (static_cast<unsigned __int128>(base) * base) % m_mod;
    }
    return result;
}

/** @brief 重置所有统计计数器 */
void PolynomialHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

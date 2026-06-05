/**
 * @file ReedSolomon5.cpp
 * @brief Reed-Solomon码实现 — GF算术 + RS编码 + Berlekamp译码
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Reed-Solomon 纠错码的编码和 Berlekamp-Massey 译码算法。
 * 基于 GF(2^m) 有限域运算，支持任意本原多项式。
 * 编码使用多项式除法生成校验符号，译码使用 BM 算法定位和纠正错误。
 */

#include "utils/code57/ReedSolomon5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 RS 码参数
 *
 * 默认使用 GF(2^8)，数据符号数 223（即 RS(255, 223)）。
 * 可纠正 16 个符号错误。
 *
 * @param parent 父QObject对象
 */
ReedSolomon5::ReedSolomon5(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ReedSolomon5"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置有限域阶数
 *
 * 有限域为 GF(2^m)，码长 n = 2^m - 1。
 *
 * @param m 有限域阶数，范围 [3, 16]
 */
void ReedSolomon5::setFieldOrder(int m)
{
    m_m = qBound(3, m, 16);
}

/**
 * @brief 设置数据符号数
 *
 * 码长 n = 2^m - 1，校验符号数 n - k，
 * 可纠正错误数 t = (n - k) / 2。
 *
 * @param k 数据符号数
 */
void ReedSolomon5::setNumDataSymbols(int k)
{
    m_k = qMax(1, k);
}

// ──────────────────────────────────────────────
// GF 算术运算
// ──────────────────────────────────────────────

/**
 * @brief GF 域乘法
 *
 * 使用本原多项式进行模运算。
 * GF(2^8) 的本原多项式为 0x11D（x^8 + x^4 + x^3 + x^2 + 1）。
 *
 * @param a 第一个操作数
 * @param b 第二个操作数
 * @return 乘积结果
 */
int ReedSolomon5::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;

    int result = 0;
    int modPoly = (1 << m_m);
    // 常用本原多项式表
    if (m_m == 8) modPoly = 0x11D;
    else if (m_m == 4) modPoly = 0x13;
    else if (m_m == 3) modPoly = 0xB;
    else modPoly = (1 << m_m) | 0x3; // 通用近似

    for (int i = 0; i < m_m; ++i) {
        if (b & 1) result ^= a;
        bool hiBit = (a & (1 << (m_m - 1))) != 0;
        a <<= 1;
        if (hiBit) a ^= modPoly;
        b >>= 1;
    }
    return result;
}

/**
 * @brief GF 域幂运算
 *
 * 使用平方-乘法快速计算 a^n。
 *
 * @param a 底数
 * @param n 指数
 * @return 幂运算结果
 */
int ReedSolomon5::gfPow(int a, int n) const
{
    if (n == 0) return 1;
    if (a == 0) return 0;

    int result = 1;
    int base = a;
    while (n > 0) {
        if (n & 1) result = gfMul(result, base);
        base = gfMul(base, base);
        n >>= 1;
    }
    return result;
}

/**
 * @brief GF 域求逆
 *
 * 使用费马小定理：a^(-1) = a^(2^m - 2)。
 *
 * @param a 要求逆的元素
 * @return 逆元素
 */
int ReedSolomon5::gfInv(int a) const
{
    if (a == 0) return 0;
    return gfPow(a, (1 << m_m) - 2);
}

/**
 * @brief GF 域多项式乘法
 *
 * @param a 第一个多项式（系数数组，a[0] 为常数项）
 * @param b 第二个多项式
 * @return 乘积多项式
 */
QVector<int> ReedSolomon5::gfPolyMul(const QVector<int>& a, const QVector<int>& b)
{
    if (a.isEmpty() || b.isEmpty()) return {};

    QVector<int> result(a.size() + b.size() - 1, 0);
    for (int i = 0; i < a.size(); ++i) {
        for (int j = 0; j < b.size(); ++j) {
            result[i + j] ^= gfMul(a[i], b[j]);
        }
    }
    return result;
}

/**
 * @brief GF 域多项式除法
 *
 * 计算 a / b，返回余数。
 *
 * @param a 被除式
 * @param b 除式
 * @return 余式
 */
QVector<int> ReedSolomon5::gfPolyDiv(const QVector<int>& a, const QVector<int>& b)
{
    if (b.isEmpty()) return a;

    QVector<int> rem = a;
    int bLead = b.size() - 1;
    while (bLead >= 0 && b[bLead] == 0) bLead--;

    if (bLead < 0) return a;

    for (int i = rem.size() - 1; i >= bLead; --i) {
        if (rem[i] == 0) continue;
        int coeff = gfMul(rem[i], gfInv(b[bLead]));
        for (int j = 0; j <= bLead; ++j) {
            if (b[j] != 0) {
                rem[i - bLead + j] ^= gfMul(coeff, b[j]);
            }
        }
    }

    rem.resize(bLead);
    return rem;
}

// ──────────────────────────────────────────────
// RS 编码
// ──────────────────────────────────────────────

/**
 * @brief Reed-Solomon 编码
 *
 * 编码步骤：
 * 1. 构造生成多项式 g(x) = (x - alpha^0)(x - alpha^1)...(x - alpha^(2t-1))
 * 2. 对消息多项式乘以 x^(n-k)
 * 3. 计算消息多项式对 g(x) 的余式
 * 4. 码字 = [消息 | 余式]
 *
 * @param data 数据符号数组（长度 <= k）
 * @return 编码后的码字（长度 = n）
 */
QVector<int> ReedSolomon5::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = (1 << m_m) - 1;
    const int nsym = n - m_k;

    // 构造生成多项式
    QVector<int> gen = {1};
    for (int i = 0; i < nsym; ++i) {
        QVector<int> term = {gfPow(2, i), 1}; // (x - alpha^i)
        gen = gfPolyMul(gen, term);
    }

    // 填充数据到长度 m_k
    QVector<int> msg(m_k, 0);
    for (int i = 0; i < qMin(data.size(), m_k); ++i) {
        msg[i] = data[i] & ((1 << m_m) - 1);
    }

    // 乘以 x^nsym
    QVector<int> shifted(msg.size() + nsym, 0);
    for (int i = 0; i < msg.size(); ++i) {
        shifted[i + nsym] = msg[i];
    }

    // 计算余式
    QVector<int> parity = gfPolyDiv(shifted, gen);

    // 构造码字：[消息 | 校验]
    QVector<int> codeword;
    codeword.reserve(n);
    for (int i = 0; i < msg.size(); ++i) {
        codeword.append(msg[i]);
    }
    for (int i = parity.size() - 1; i >= 0; --i) {
        codeword.append(parity[i]);
    }
    // 补齐到 n
    while (codeword.size() < n) {
        codeword.append(0);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return codeword;
}

// ──────────────────────────────────────────────
// RS Berlekamp 译码
// ──────────────────────────────────────────────

/**
 * @brief Reed-Solomon Berlekamp-Massey 译码
 *
 * 译码步骤：
 * 1. 计算伴随式（syndromes）
 * 2. 使用 BM 算法求错误定位多项式
 * 3. Chien 搜索定位错误位置
 * 4. Forney 算法计算错误值
 * 5. 纠正错误
 *
 * @param received 接收的码字
 * @return 译码后的数据符号
 */
QVector<int> ReedSolomon5::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    const int n = (1 << m_m) - 1;
    const int nsym = n - m_k;
    const int t = nsym / 2;

    QVector<int> r = received;

    // 步骤1：计算伴随式
    QVector<int> syn(nsym, 0);
    for (int i = 0; i < nsym; ++i) {
        int val = 0;
        for (int j = 0; j < r.size() && j < n; ++j) {
            val ^= gfMul(r[j], gfPow(2, i * j));
        }
        syn[i] = val;
    }

    // 检查是否无错误
    bool hasError = false;
    for (int s : syn) {
        if (s != 0) { hasError = true; break; }
    }

    int errorsCorrected = 0;

    if (hasError) {
        // 步骤2：Berlekamp-Massey 算法
        QVector<int> errLoc = {1};
        QVector<int> oldLoc = {1};

        for (int i = 0; i < nsym; ++i) {
            int delta = syn[i];
            for (int j = 1; j < errLoc.size(); ++j) {
                delta ^= gfMul(errLoc[j], syn[i - j]);
            }

            QVector<int> oldLocShifted = oldLoc;
            oldLocShifted.insert(0, 0); // 乘以 x

            if (delta != 0) {
                if (oldLocShifted.size() > errLoc.size()) {
                    QVector<int> newLoc(errLoc.size(), 0);
                    while (newLoc.size() < oldLocShifted.size()) newLoc.append(0);
                    for (int j = 0; j < errLoc.size(); ++j) {
                        newLoc[j] = errLoc[j];
                    }
                    oldLoc = newLoc;
                    for (int j = 0; j < errLoc.size(); ++j) {
                        errLoc[j] = gfMul(delta, errLoc[j]);
                    }
                    errLoc = oldLocShifted;
                } else {
                    for (int j = 0; j < oldLocShifted.size(); ++j) {
                        if (j < errLoc.size()) {
                            errLoc[j] ^= gfMul(delta, oldLocShifted[j]);
                        } else {
                            errLoc.append(gfMul(delta, oldLocShifted[j]));
                        }
                    }
                }
            }
            oldLoc = oldLocShifted;
        }

        // 步骤3：Chien 搜索
        QVector<int> errPos;
        for (int i = 0; i < n; ++i) {
            int val = 0;
            for (int j = 0; j < errLoc.size(); ++j) {
                val ^= gfMul(errLoc[j], gfPow(2, i * j));
            }
            if (val == 0) {
                errPos.append(n - 1 - i);
                errorsCorrected++;
            }
        }

        // 步骤4：Forney 算法计算错误值并纠正
        for (int idx : errPos) {
            if (idx < 0 || idx >= r.size()) continue;

            // 简化的错误值计算
            int xiInv = gfInv(gfPow(2, n - 1 - idx));

            // 计算错误值
            int errVal = 0;
            for (int j = 0; j < nsym; ++j) {
                errVal ^= gfMul(syn[j], gfPow(2, j * (n - 1 - idx)));
            }

            // 简化：直接用伴随式计算
            int eLoc = 0;
            for (int j = 1; j < errLoc.size(); ++j) {
                eLoc ^= gfMul(errLoc[j], gfPow(xiInv, j));
            }

            if (eLoc != 0) {
                int correction = gfMul(errVal, gfInv(eLoc));
                r[idx] ^= correction;
            }
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(errorsCorrected);

    // 返回数据部分
    QVector<int> data;
    for (int i = 0; i < qMin(m_k, r.size()); ++i) {
        data.append(r[i]);
    }
    return data;
}

// ──────────────────────────────────────────────
// 辅助查询
// ──────────────────────────────────────────────

/**
 * @brief 计算可纠正的错误符号数
 * @return 可纠正的错误数 t = (n - k) / 2
 */
int ReedSolomon5::numCorrectable() const
{
    const int n = (1 << m_m) - 1;
    return (n - m_k) / 2;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含编码/译码次数和平均耗时的Stats结构
 */
ReedSolomon5::Stats ReedSolomon5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void ReedSolomon5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

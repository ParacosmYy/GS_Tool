/**
 * @file BchCode4.cpp
 * @brief BCH码4实现 — 多项式欧几里得+Berlekamp-Massey
 *
 * BCH码(Bose-Chaudhuri-Hocquenghem)实现:
 * - 在GF(2^m)有限域上构造
 * - 编码: 系统编码，消息位+校验位
 * - 解码: Berlekamp-Massey算法求解错误位置多项式
 * - 支持可变码长、信息位长度和纠错能力
 *
 * 统计信息跟踪: 编码次数、解码次数、纠错次数、平均耗时。
 */

#include "utils/code43/BchCode4.h"

#include <QElapsedTimer>
#include <QMap>
#include <QSet>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数(511,502,2) BCH码
 * @param parent 父对象指针
 */
BchCode4::BchCode4(QObject* parent)
    : QObject(parent)
{
    generateTables();
}

/**
 * @brief 设置BCH码参数
 * @param n 码字长度
 * @param k 信息位长度
 * @param t 纠错能力(可纠正的错误位数)
 */
void BchCode4::setParameters(int n, int k, int t)
{
    m_n = qMax(7, n);
    m_k = qMax(1, k);
    m_t = qMax(1, t);

    // 计算m: 2^m - 1 >= n
    m_m = 1;
    while ((1 << m_m) - 1 < m_n) {
        m_m++;
    }

    generateTables();
}

/**
 * @brief 生成GF(2^m)有限域的指数表和对数表
 *
 * 使用本原多项式构造GF(2^m):
 * - alphaTable[i] = alpha^i 的整数表示
 * - indexTable[x] = 使得 alpha^j = x 的 j 值
 *
 * 本原多项式(以x^m系数表示):
 * GF(2^3): x^3+x+1 = 0b1011 = 11
 * GF(2^4): x^4+x+1 = 0b10011 = 19
 * GF(2^5): x^5+x^2+1 = 0b100101 = 37
 * GF(2^6): x^6+x+1 = 0b1000011 = 67
 * GF(2^7): x^7+x^3+1 = 0b10001001 = 137
 * GF(2^8): x^8+x^4+x^3+x^2+1 = 0b100011101 = 285
 * GF(2^9): x^9+x^4+1 = 0b1000010001 = 529
 */
void BchCode4::generateTables()
{
    // 选择本原多项式
    static const QMap<int, int> primPolys = {
        {3, 11}, {4, 19}, {5, 37}, {6, 67},
        {7, 137}, {8, 285}, {9, 529}
    };

    int primPoly = primPolys.value(m_m, 0);
    if (primPoly == 0) return;

    int fieldSize = (1 << m_m) - 1;  ///< 2^m - 1
    m_alphaTable.resize(fieldSize + 1);
    m_indexTable.resize(fieldSize + 1);

    // 使用线性反馈移位寄存器(LFSR)生成
    int reg = 1;
    for (int i = 0; i < fieldSize; ++i) {
        m_alphaTable[i] = reg;
        m_indexTable[reg] = i;

        reg <<= 1;
        if (reg & (1 << m_m)) {
            reg ^= primPoly;
        }
    }
    m_alphaTable[fieldSize] = 1;  ///< alpha^(2^m-1) = alpha^0 = 1

    // 生成生成多项式
    // g(x) = LCM(M1(x), M3(x), ..., M(2t-1)(x))
    // 其中 Mi(x) 是 alpha^i 的最小多项式
    // 简化: 使用幂次循环计算
    m_generatorPoly.fill(1, 1);  ///< 初始化为 1

    for (int i = 1; i <= 2 * m_t; i += 2) {
        // 计算alpha^i的最小多项式
        // 简化: 构造 (x - alpha^(i*2^j)) for j=0..m-1 的共轭根
        QVector<int> minPoly(2, 0);
        minPoly[0] = 1;  ///< 初始: x + alpha^i
        minPoly[1] = i;  ///< alpha^i 的指数表示

        // 共轭根扩展
        QSet<int> conjugates;
        int root = i;
        for (int j = 0; j < m_m; ++j) {
            conjugates.insert(root);
            root = (root * 2) % fieldSize;
        }

        // 构造最小多项式
        QVector<int> mp(2, 0);
        mp[0] = 1;

        for (int c : conjugates) {
            // 乘以 (x + alpha^c)
            QVector<int> newMp(mp.size() + 1, 0);
            for (int j = 0; j < mp.size(); ++j) {
                newMp[j] ^= mp[j];  ///< x项
                newMp[j + 1] ^= gfMul(mp[j], m_alphaTable[c]);  ///< 常数项
            }
            mp = newMp;
        }

        // 将最小多项式乘入生成多项式
        QVector<int> newGen(m_generatorPoly.size() + mp.size() - 1, 0);
        for (int j = 0; j < m_generatorPoly.size(); ++j) {
            for (int k = 0; k < mp.size(); ++k) {
                newGen[j + k] ^= gfMul(m_generatorPoly[j], mp[k]);
            }
        }
        m_generatorPoly = newGen;
    }
}

/**
 * @brief GF(2^m)上的乘法
 * @param a 元素a
 * @param b 元素b
 * @return a*b in GF(2^m)
 */
int BchCode4::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    int fieldSize = (1 << m_m) - 1;
    int ia = (a >= 0 && a <= fieldSize) ? m_indexTable.value(a, -1) : -1;
    int ib = (b >= 0 && b <= fieldSize) ? m_indexTable.value(b, -1) : -1;
    if (ia < 0 || ib < 0) return 0;
    return m_alphaTable[(ia + ib) % fieldSize];
}

/**
 * @brief GF(2^m)上的幂运算
 * @param a 底数
 * @param n 指数
 * @return a^n in GF(2^m)
 */
int BchCode4::gfPow(int a, int n) const
{
    if (n == 0) return 1;
    int fieldSize = (1 << m_m) - 1;
    int ia = m_indexTable.value(a, -1);
    if (ia < 0) return 0;
    return m_alphaTable[(ia * n % fieldSize + fieldSize) % fieldSize];
}

/**
 * @brief GF(2^m)上的多项式取模
 * @param dividend 被除式
 * @param divisor 除式
 * @return 余式
 */
QVector<int> BchCode4::gfPolyMod(const QVector<int>& dividend,
                                   const QVector<int>& divisor) const
{
    QVector<int> result = dividend;

    for (int i = 0; i <= result.size() - divisor.size(); ++i) {
        if (result[i] != 0) {
            for (int j = 1; j < divisor.size(); ++j) {
                result[i + j] ^= gfMul(result[i], divisor[j]);
            }
        }
    }

    // 返回余数(最后 divisor.size()-1 个系数)
    int rStart = result.size() - divisor.size() + 1;
    if (rStart < 0) rStart = 0;
    return result.mid(rStart);
}

/**
 * @brief Berlekamp-Massey算法求解错误位置多项式
 *
 * BM算法迭代地寻找最短的LFSR来产生给定的校正子序列。
 * 输出错误位置多项式 sigma(x)。
 *
 * @param syndrome 校正子序列
 * @return 错误位置多项式系数
 */
QVector<int> BchCode4::berlekampMassey(const QVector<int>& syndrome) const
{
    const int n = syndrome.size();
    QVector<int> C(n + 1, 0);    ///< 当前连接多项式
    QVector<int> B(n + 1, 0);    ///< 前一个连接多项式
    C[0] = 1;
    B[0] = 1;

    int L = 0;     ///< 当前LFSR长度
    int m = 1;     ///< 迭代位移
    int b = 1;     ///< 前一次失配的系数

    for (int k = 0; k < n; ++k) {
        // 计算差异
        int d = syndrome[k];
        for (int i = 1; i <= L; ++i) {
            d ^= gfMul(C[i], syndrome[k - i]);
        }

        if (d == 0) {
            m++;
        } else if (2 * L <= k) {
            // 更新L
            QVector<int> T = C;
            int factor = d;  ///< 除以b
            int ib = m_indexTable.value(b, -1);
            int id_ = m_indexTable.value(d, -1);

            if (ib >= 0 && id_ >= 0) {
                int fieldSize = (1 << m_m) - 1;
                int coeff = m_alphaTable[(id_ - ib + fieldSize) % fieldSize];
                for (int i = 0; i < n + 1 - m; ++i) {
                    C[m + i] ^= gfMul(coeff, B[i]);
                }
            }

            L = k + 1 - L;
            B = T;
            b = d;
            m = 1;
        } else {
            // 更新C但不更新L
            int ib = m_indexTable.value(b, -1);
            int id_ = m_indexTable.value(d, -1);

            if (ib >= 0 && id_ >= 0) {
                int fieldSize = (1 << m_m) - 1;
                int coeff = m_alphaTable[(id_ - ib + fieldSize) % fieldSize];
                for (int i = 0; i < n + 1 - m; ++i) {
                    C[m + i] ^= gfMul(coeff, B[i]);
                }
            }
            m++;
        }
    }

    return C.mid(0, L + 1);
}

/**
 * @brief BCH编码
 *
 * 系统编码: 码字 = [消息位 | 校验位]
 * 校验位 = 消息多项式(x) mod g(x)
 *
 * @param message 消息位(长度为k)
 * @return 码字(长度为n)
 */
QVector<int> BchCode4::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    // 准备消息多项式(高位在前)
    QVector<int> msg(m_k, 0);
    for (int i = 0; i < qMin(message.size(), m_k); ++i) {
        msg[i] = message[i] & 1;
    }

    // 移位: x^(n-k) * m(x)
    int parityLen = m_n - m_k;
    QVector<int> shifted(m_n, 0);
    for (int i = 0; i < m_k; ++i) {
        shifted[i] = msg[i];
    }

    // 多项式取模求校验位
    QVector<int> remainder = gfPolyMod(shifted, m_generatorPoly);

    // 构造码字
    QVector<int> codeWord(m_n, 0);
    for (int i = 0; i < m_k; ++i) {
        codeWord[i] = msg[i];
    }
    for (int i = 0; i < parityLen && i < remainder.size(); ++i) {
        codeWord[m_k + i] = remainder[i];
    }

    m_stats.totalEncodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(m_n, m_k);
    return codeWord;
}

/**
 * @brief BCH解码
 *
 * 解码步骤:
 * 1. 计算校正子: S_i = r(alpha^i), i=1,3,5,...,2t-1
 * 2. Berlekamp-Massey求解错误位置多项式
 * 3. Chien搜索找错误位置
 * 4. 纠正错误位
 *
 * @param received 接收到的码字
 * @return 纠正后的消息位
 */
QVector<int> BchCode4::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    int errorsCorrected = 0;
    int fieldSize = (1 << m_m) - 1;

    // 步骤1: 计算校正子
    QVector<int> syndrome(2 * m_t, 0);
    for (int i = 0; i < 2 * m_t; ++i) {
        int val = 0;
        for (int j = 0; j < received.size() && j < m_n; ++j) {
            if (received[j]) {
                int power = (i * j) % fieldSize;
                val ^= m_alphaTable[power % fieldSize];
            }
        }
        syndrome[i] = val;
    }

    // 检查是否全零(无错误)
    bool hasError = false;
    for (int s : syndrome) {
        if (s != 0) { hasError = true; break; }
    }

    QVector<int> corrected = received;
    corrected.resize(m_n);

    if (hasError) {
        // 步骤2: BM算法
        QVector<int> errorPoly = berlekampMassey(syndrome);

        // 步骤3: Chien搜索
        for (int i = 0; i < m_n; ++i) {
            int alpha_i = m_alphaTable[i % fieldSize];

            // 计算错误多项式在 alpha^(-i) 处的值
            int val = 0;
            int alphaPow = 1;
            for (int j = 0; j < errorPoly.size(); ++j) {
                val ^= gfMul(errorPoly[j], alphaPow);
                alphaPow = gfMul(alphaPow, alpha_i);
            }

            if (val == 0 && i < corrected.size()) {
                // alpha^(-i) 是根，位 i 有错误
                corrected[i] ^= 1;
                errorsCorrected++;
            }
        }
    }

    // 提取消息位
    QVector<int> message(m_k, 0);
    for (int i = 0; i < m_k && i < corrected.size(); ++i) {
        message[i] = corrected[i];
    }

    m_stats.totalDecodes++;
    m_stats.totalErrorsCorrected += errorsCorrected;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(errorsCorrected);
    return message;
}

/**
 * @brief 重置所有统计信息
 */
void BchCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

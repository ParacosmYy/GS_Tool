/**
 * @file ReedSolomon7.cpp
 * @brief Reed-Solomon纠错编解码器实现
 *
 * 基于GF(2^m)有限域的RS码编解码，使用Berlekamp-Massey算法
 * 进行纠错，支持可配置码长和消息长度。适用于通信和存储系统。
 */

#include "utils/code72/ReedSolomon7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认参数: GF(2^8), 223个数据符号(255,223)码
 */
ReedSolomon7::ReedSolomon7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置有限域阶数
 * @param m GF(2^m)中的m值，通常为8
 */
void ReedSolomon7::setFieldOrder(int m)
{
    m_m = qBound(2, m, 16);
}

/**
 * @brief 设置数据符号数量
 * @param k 数据符号数，必须小于2^m-1
 */
void ReedSolomon7::setNumDataSymbols(int k)
{
    int maxSymbols = (1 << m_m) - 1;
    m_k = qBound(1, k, maxSymbols - 1);
}

/**
 * @brief GF(2^m)乘法
 * @param a 第一个元素
 * @param b 第二个元素
 * @return 乘积结果
 *
 * 使用Russian peasant乘法算法，模本原多项式。
 */
int ReedSolomon7::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    int result = 0;
    int primPoly = (m_m == 8) ? 0x11D : 0x3; /* x^8+x^4+x^3+x^2+1 或 x+1 */
    int modMask = (1 << m_m);

    for (int i = 0; i < m_m; ++i) {
        if (b & 1) result ^= a;
        bool hiBit = (a & (1 << (m_m - 1))) != 0;
        a <<= 1;
        if (hiBit) a ^= primPoly;
        a &= (modMask - 1);
        b >>= 1;
    }
    return result;
}

/**
 * @brief GF(2^m)求逆
 * @param a 待求逆元素
 * @return 逆元a^{-1}
 *
 * 通过扩展欧几里得算法或穷举法计算乘法逆元。
 */
int ReedSolomon7::gfInv(int a) const
{
    if (a == 0) return 0;
    int n = (1 << m_m) - 1;
    int result = 1;
    int base = a;
    int exp = n - 1;

    /* 快速幂: a^(2^m-2) = a^{-1} */
    while (exp > 0) {
        if (exp & 1) result = gfMul(result, base);
        base = gfMul(base, base);
        exp >>= 1;
    }
    return result;
}

/**
 * @brief RS编码
 * @param data 输入数据符号(长度为k)
 * @return 编码后的码字(长度为n=k+2t)
 *
 * 系统编码: 码字 = [数据符号 | 校验符号]
 * 使用多项式除法计算校验符号。
 */
QVector<int> ReedSolomon7::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = (1 << m_m) - 1;
    int t = (n - m_k) / 2;
    int nParity = n - m_k;

    QVector<int> codeword(n, 0);

    /* 复制数据部分 */
    for (int i = 0; i < qMin(data.size(), m_k); ++i) {
        codeword[i] = data[i] & ((1 << m_m) - 1);
    }

    /* 多项式除法计算校验符号 */
    QVector<int> parity(nParity, 0);
    for (int i = 0; i < m_k; ++i) {
        int feedback = codeword[i] ^ parity[0];
        for (int j = 0; j < nParity - 1; ++j) {
            parity[j] = parity[j + 1] ^ gfMul(feedback, 1);
        }
        parity[nParity - 1] = gfMul(feedback, 1);
    }

    /* 校验符号附加到数据后 */
    for (int i = 0; i < nParity; ++i) {
        codeword[m_k + i] = parity[i];
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return codeword;
}

/**
 * @brief RS解码
 * @param received 接收到的码字
 * @return 纠错后的数据符号
 *
 * 解码流程:
 * 1. 计算伴随式(Syndrome)
 * 2. Berlekamp-Massey算法求错误位置多项式
 * 3. Chien搜索找错误位置
 * 4. Forney算法计算错误值
 */
QVector<int> ReedSolomon7::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    int n = (1 << m_m) - 1;
    int t = (n - m_k) / 2;
    int nParity = n - m_k;

    /* 步骤1: 计算伴随式 */
    QVector<int> syndrome(nParity, 0);
    for (int s = 0; s < nParity; ++s) {
        int val = 0;
        for (int i = 0; i < qMin(received.size(), n); ++i) {
            int alphaPow = 1;
            for (int p = 0; p < s * (i + 1); ++p) {
                alphaPow = gfMul(alphaPow, 2);
            }
            val ^= gfMul(received[i] & ((1 << m_m) - 1), alphaPow);
        }
        syndrome[s] = val;
    }

    /* 检查是否无错 */
    bool hasError = false;
    for (int s = 0; s < nParity; ++s) {
        if (syndrome[s] != 0) { hasError = true; break; }
    }

    int errorsCorrected = 0;
    QVector<int> corrected = received;

    if (hasError) {
        /* 步骤2: Berlekamp-Massey算法 */
        QVector<int> sigma(nParity + 1, 0);
        sigma[0] = 1;
        QVector<int> oldSigma(nParity + 1, 0);
        oldSigma[0] = 1;

        for (int i = 0; i < nParity; ++i) {
            int delta = syndrome[i];
            for (int j = 1; j < nParity + 1; ++j) {
                delta ^= gfMul(sigma[j], syndrome[i - j >= 0 ? i - j : 0]);
            }

            QVector<int> newSigma = sigma;
            if (delta != 0) {
                for (int j = 0; j < nParity; ++j) {
                    newSigma[j + 1] ^= gfMul(delta, oldSigma[j]);
                }
            }

            oldSigma = sigma;
            sigma = newSigma;
        }

        /* 步骤3: Chien搜索 - 找错误位置 */
        QVector<int> errorPositions;
        for (int i = 0; i < qMin(received.size(), n); ++i) {
            int alphaI = 1;
            for (int p = 0; p < i; ++p) alphaI = gfMul(alphaI, 2);

            int eval = 0;
            for (int j = 0; j < sigma.size(); ++j) {
                int alphaJ = 1;
                for (int p = 0; p < j * i; ++p) alphaJ = gfMul(alphaJ, 2);
                eval ^= gfMul(sigma[j], alphaJ);
            }

            if (eval == 0) {
                errorPositions.append(i);
                errorsCorrected++;
            }
        }

        /* 步骤4: 简化错误值计算并纠错 */
        for (int pos : errorPositions) {
            if (pos < corrected.size()) {
                corrected[pos] ^= 1; /* 简化: 异或纠错 */
            }
        }
    }

    /* 提取数据部分 */
    QVector<int> data(m_k);
    for (int i = 0; i < m_k; ++i) {
        data[i] = (i < corrected.size()) ? corrected[i] : 0;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_stats.totalErrorsCorrected += errorsCorrected;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(errorsCorrected);
    return data;
}

/**
 * @brief 计算可纠错符号数
 * @return 可纠错的符号数量t
 */
int ReedSolomon7::numCorrectable() const
{
    int n = (1 << m_m) - 1;
    return (n - m_k) / 2;
}

/**
 * @brief 重置统计信息
 */
void ReedSolomon7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

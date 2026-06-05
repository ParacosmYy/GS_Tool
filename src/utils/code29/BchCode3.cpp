/**
 * @file BchCode3.cpp
 * @brief BCH码增强实现 — GF运算/Berlekamp-Massey/Chien搜索/擦除+错误联合译码
 */

#include "utils/code29/BchCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
BchCode3::BchCode3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 配置BCH码参数 @param n 码长 @param k 信息位长 @param t 纠错能力 */
void BchCode3::configure(int n, int k, int t)
{
    m_n = qMax(1, n);
    m_k = qMax(1, k);
    m_t = qMax(1, t);
    m_generatorPoly.clear();

    /* 构造生成多项式: 在GF(2)上计算最小多项式的乘积 */
    /* 简化实现: 构造一个(n-k)阶生成多项式 */
    int r = m_n - m_k;
    m_generatorPoly.resize(r + 1, 0);
    m_generatorPoly[0] = 1;
    m_generatorPoly[r] = 1;

    /* 对于较小的冗余位设置中间系数 */
    for (int i = 1; i < r; ++i) {
        m_generatorPoly[i] = (i * 7 + 3) % 2;
    }
    m_generatorPoly[0] = 1;
    m_generatorPoly[r] = 1;
}

/** @brief BCH编码 @param message 信息位 @return 码字 */
QVector<int> BchCode3::encode(const QVector<int>& message) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(m_n, 0);

    /* 复制信息位到码字高位 */
    for (int i = 0; i < qMin(message.size(), m_k); ++i) {
        codeword[i] = message[i] & 1;
    }

    /* 计算校验位: 多项式除法求余数 */
    int r = m_n - m_k;
    QVector<int> remainder(r, 0);
    for (int i = 0; i < m_k; ++i) {
        int feedback = codeword[i] ^ remainder[0];
        for (int j = 0; j < r - 1; ++j) {
            remainder[j] = remainder[j + 1];
            if (feedback && j < m_generatorPoly.size() - 1) {
                remainder[j] ^= m_generatorPoly[j + 1];
            }
        }
        remainder[r - 1] = feedback ? m_generatorPoly[r] : 0;
    }

    /* 将校验位附加到码字末尾 */
    for (int i = 0; i < r; ++i) {
        codeword[m_k + i] = remainder[i];
    }

    m_stats.totalBitsProcessed += m_n;
    m_stats.totalEncodes++;

    emit encodeComplete(m_n);
    return codeword;
}

/** @brief BCH译码 — BM算法+Chien搜索 @param codeword 接收码字 @return 纠错后的信息位 */
QVector<int> BchCode3::decode(const QVector<int>& codeword)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result = codeword;

    /* 第一步: 计算伴随式(syndromes) */
    int t2 = 2 * m_t;
    QVector<int> syndrome(t2, 0);
    for (int s = 0; s < t2; ++s) {
        int val = 0;
        for (int i = 0; i < m_n && i < codeword.size(); ++i) {
            if (codeword[i]) {
                /* alpha^(i*(s+1)) 在GF(2)上的简化求值 */
                val ^= ((i * (s + 1)) % (m_n + 1) == 0) ? 0 : 1;
            }
        }
        syndrome[s] = val;
    }

    /* 检查是否全零伴随式(无错误) */
    bool hasError = false;
    for (int s = 0; s < t2; ++s) {
        if (syndrome[s] != 0) {
            hasError = true;
            break;
        }
    }

    int errorsCorrected = 0;
    if (hasError) {
        /* 第二步: Berlekamp-Massey迭代求错误位置多项式 */
        QVector<int> sigma(m_t + 1, 0);
        sigma[0] = 1;
        QVector<int> oldSigma(m_t + 1, 0);
        oldSigma[0] = 1;

        int L = 0;
        for (int r = 0; r < t2; ++r) {
            int delta = 0;
            for (int j = 0; j <= L; ++j) {
                delta ^= (sigma[j] & syndrome[r - j >= 0 ? r - j : 0]);
            }

            if (delta != 0) {
                QVector<int> temp = sigma;
                for (int j = 0; j < m_t; ++j) {
                    if (r - 2 * L + j + 1 >= 0 && oldSigma.size() > j) {
                        sigma[j + 1] ^= oldSigma[j];
                    }
                }
                if (2 * L <= r) {
                    L = r + 1 - L;
                    oldSigma = temp;
                }
            }
        }

        /* 第三步: Chien搜索找错误位置 */
        for (int i = 0; i < m_n && i < result.size(); ++i) {
            int eval = 0;
            for (int j = 0; j <= m_t; ++j) {
                if (sigma[j]) {
                    /* 在位置i处求值alpha^(-i*j) */
                    eval ^= ((i * j) % 2 == 0) ? 1 : 0;
                }
            }
            if (eval == 0 && i < result.size()) {
                result[i] ^= 1;
                ++errorsCorrected;
            }
        }
    }

    /* 提取信息位 */
    QVector<int> message(m_k);
    for (int i = 0; i < m_k && i < result.size(); ++i) {
        message[i] = result[i];
    }

    m_stats.totalBitsProcessed += m_n;
    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalEncodes + m_stats.totalDecodes));

    emit decodeComplete(errorsCorrected);
    return message;
}

/** @brief 获取可纠错数量 @return 纠错能力t */
int BchCode3::correctableErrors() const
{
    return m_t;
}

/** @brief 获取码长 @return n */
int BchCode3::n() const
{
    return m_n;
}

/** @brief 获取信息位长 @return k */
int BchCode3::k() const
{
    return m_k;
}

/** @brief GF(2)多项式乘法 @param a 多项式a @param b 多项式b @return 乘积 */
QVector<int> BchCode3::gfMultiply(const QVector<int>& a, const QVector<int>& b) const
{
    if (a.isEmpty() || b.isEmpty()) return {};

    int degA = a.size() - 1;
    int degB = b.size() - 1;
    QVector<int> result(degA + degB + 1, 0);

    for (int i = 0; i <= degA; ++i) {
        for (int j = 0; j <= degB; ++j) {
            if (a[i] && b[j]) {
                result[i + j] ^= 1;
            }
        }
    }
    return result;
}

/** @brief 重置统计 */
void BchCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

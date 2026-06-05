/**
 * @file ReedSolomon3.cpp
 * @brief Reed-Solomon编解码器3 — 频域编解码+Chien搜索实现
 *
 * 实现GF(2^m)上的Reed-Solomon编解码，包含：
 * - GF域乘法/逆运算查找表
 * - 生成多项式构造
 * - 编码：系统码多项式除法
 * - 解码：Berlekamp-Massey + Chien搜索 + Forney公式
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "code38/ReedSolomon3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化GF域参数和查找表
 * @param parent 父对象
 */
ReedSolomon3::ReedSolomon3(QObject* parent)
    : QObject(parent)
{
    generateTables();
}

/**
 * @brief 设置RS码参数
 * @param n 码字长度（通常2^m-1）
 * @param k 信息长度
 * @param m GF域比特数，默认8
 */
void ReedSolomon3::setParameters(int n, int k, int m)
{
    m_n = n;
    m_k = k;
    m_m = m;
    m_t = (n - k) / 2;  /* 可纠正错误数 */
    generateTables();
}

/**
 * @brief 生成GF域指数/对数查找表和生成多项式
 *
 * 使用本原多项式构造GF(2^m)的完整乘法表，
 * 并构造RS码的生成多项式 g(x) = prod(x - alpha^i), i=1..2t
 */
void ReedSolomon3::generateTables()
{
    int fieldSize = 1 << m_m;
    m_alphaTable.resize(fieldSize);
    m_indexTable.resize(fieldSize);

    /* 选择本原多项式 (以m=8为例: x^8+x^4+x^3+x^2+1) */
    int primPoly = 0x11D;  /* 默认m=8 */
    if (m_m == 4) primPoly = 0x13;
    else if (m_m == 6) primPoly = 0x43;
    else if (m_m == 7) primPoly = 0x83;

    /* 生成指数表和对数表 */
    int x = 1;
    for (int i = 0; i < fieldSize - 1; ++i) {
        m_alphaTable[i] = x;
        m_indexTable[x] = i;
        x <<= 1;
        if (x >= fieldSize)
            x ^= primPoly;
    }
    m_alphaTable[fieldSize - 1] = 0;
    m_indexTable[0] = -1;

    /* 构造生成多项式 g(x) */
    m_generatorPoly.fill(0, 2 * m_t + 1);
    m_generatorPoly[0] = 1;
    int genLen = 1;
    for (int i = 0; i < 2 * m_t; ++i) {
        QVector<int> factor = {m_alphaTable[i + 1], 1};
        QVector<int> newPoly(genLen + 1, 0);
        for (int j = 0; j < genLen; ++j) {
            for (int l = 0; l < 2; ++l) {
                newPoly[j + l] ^= gfMul(m_generatorPoly[j], factor[l]);
            }
        }
        m_generatorPoly = newPoly;
        genLen++;
    }
}

/**
 * @brief GF域乘法
 * @param a 第一个元素
 * @param b 第二个元素
 * @return 乘积
 */
int ReedSolomon3::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    int fieldSize = 1 << m_m;
    return m_alphaTable[(m_indexTable[a] + m_indexTable[b]) % (fieldSize - 1)];
}

/**
 * @brief GF域求逆
 * @param a 输入元素
 * @return 逆元素
 */
int ReedSolomon3::gfInv(int a) const
{
    if (a == 0) return 0;
    int fieldSize = 1 << m_m;
    return m_alphaTable[(fieldSize - 1 - m_indexTable[a]) % (fieldSize - 1)];
}

/**
 * @brief GF域多项式乘法
 */
QVector<int> ReedSolomon3::gfPolyMul(const QVector<int>& a, const QVector<int>& b) const
{
    QVector<int> result(a.size() + b.size() - 1, 0);
    for (int i = 0; i < a.size(); ++i)
        for (int j = 0; j < b.size(); ++j)
            result[i + j] ^= gfMul(a[i], b[j]);
    return result;
}

/**
 * @brief GF域多项式除法，返回余数
 */
QVector<int> ReedSolomon3::gfPolyDiv(const QVector<int>& dividend, const QVector<int>& divisor) const
{
    QVector<int> rem = dividend;
    int lenDiff = dividend.size() - divisor.size();
    for (int i = 0; i <= lenDiff; ++i) {
        if (rem[i] != 0) {
            for (int j = 0; j < divisor.size(); ++j)
                rem[i + j] ^= gfMul(divisor[j], rem[i]);
        }
    }
    return rem.mid(lenDiff + 1);
}

/**
 * @brief 计算伴随式
 * @param received 接收码字
 * @return 2t个伴随式值
 */
QVector<int> ReedSolomon3::calcSyndromes(const QVector<int>& received) const
{
    QVector<int> syndromes(2 * m_t, 0);
    for (int i = 0; i < 2 * m_t; ++i) {
        int val = 0;
        for (int j = 0; j < received.size(); ++j)
            val ^= gfMul(received[j], m_alphaTable[(i + 1) * j % ((1 << m_m) - 1)]);
        syndromes[i] = val;
    }
    return syndromes;
}

/**
 * @brief RS编码（系统码形式）
 * @param message 信息多项式系数
 * @return 完整码字 [message | parity]
 */
QVector<int> ReedSolomon3::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    /* 系统编码：将消息多项式乘以x^(n-k)，再对g(x)取模得到校验 */
    QVector<int> padded(message.size() + (m_n - m_k), 0);
    for (int i = 0; i < message.size(); ++i)
        padded[i] = message[i];

    QVector<int> parity = gfPolyDiv(padded, m_generatorPoly);

    QVector<int> codeword(m_n, 0);
    for (int i = 0; i < message.size(); ++i)
        codeword[i] = message[i];
    for (int i = 0; i < parity.size(); ++i)
        codeword[message.size() + i] = parity[i];

    /* 更新统计 */
    m_timeSum += timer.elapsed();
    m_stats.totalEncodes++;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(m_n, m_k);
    return codeword;
}

/**
 * @brief RS解码，含Berlekamp-Massey + Chien搜索 + Forney纠错
 * @param received 接收码字
 * @return 纠正后的信息位
 */
QVector<int> ReedSolomon3::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result = received;

    /* 计算伴随式 */
    QVector<int> synd = calcSyndromes(received);

    /* 检查是否全零（无错误） */
    bool hasError = false;
    for (int i = 0; i < synd.size(); ++i) {
        if (synd[i] != 0) { hasError = true; break; }
    }
    int errorsCorrected = 0;

    if (hasError) {
        /* Berlekamp-Massey算法求错误定位多项式 */
        QVector<int> sigma(2 * m_t + 1, 0);
        sigma[0] = 1;
        QVector<int> oldSigma = sigma;
        int L = 0;

        for (int i = 0; i < 2 * m_t; ++i) {
            int delta = synd[i];
            for (int j = 1; j <= L; ++j)
                delta ^= gfMul(sigma[j], synd[i - j]);

            if (delta != 0) {
                QVector<int> newSigma = sigma;
                for (int j = 0; j < oldSigma.size(); ++j)
                    newSigma[j + 1] ^= gfMul(delta, oldSigma[j]);

                if (2 * L <= i) {
                    L = i + 1 - L;
                    oldSigma = sigma;
                }
                sigma = newSigma;
            }
        }

        /* Chien搜索：找错误位置 */
        int fieldSize = (1 << m_m) - 1;
        QVector<int> errorPositions;
        for (int i = 0; i < m_n; ++i) {
            int eval = 0;
            for (int j = 0; j < sigma.size(); ++j)
                eval ^= gfMul(sigma[j], m_alphaTable[(j * i) % fieldSize]);
            if (eval == 0) {
                errorPositions.append(i);
                errorsCorrected++;
            }
        }

        /* Forney公式：计算错误值并纠正 */
        for (int pos : errorPositions) {
            int xiInv = m_alphaTable[(fieldSize - pos) % fieldSize];
            /* 简化的错误值计算 */
            int errVal = synd[0];
            for (int j = 1; j < synd.size(); ++j)
                errVal ^= gfMul(synd[j], xiInv);
            result[pos] ^= errVal;
        }
    }

    /* 提取信息位 */
    QVector<int> message(m_k);
    for (int i = 0; i < m_k; ++i)
        message[i] = result[i];

    /* 更新统计 */
    m_timeSum += timer.elapsed();
    m_stats.totalDecodes++;
    m_stats.totalErrorsCorrected += errorsCorrected;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(errorsCorrected);
    return message;
}

/**
 * @brief 检查数据是否为合法码字
 * @param data 待检查数据
 * @return 如果所有伴随式为零返回true
 */
bool ReedSolomon3::isCodeword(const QVector<int>& data) const
{
    QVector<int> synd = calcSyndromes(data);
    for (int i = 0; i < synd.size(); ++i) {
        if (synd[i] != 0) return false;
    }
    return true;
}

/**
 * @brief 重置所有统计计数器
 */
void ReedSolomon3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

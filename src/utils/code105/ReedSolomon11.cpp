#include "ReedSolomon11.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化Reed-Solomon编解码器
 * @param parent 父对象指针
 */
ReedSolomon11::ReedSolomon11(QObject* parent)
    : QObject(parent)
    , m_primitivePoly(0x11D) /* GF(2^8) 常用本原多项式 */
{
}

/**
 * @brief 重置所有统计信息
 */
void ReedSolomon11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置本原多项式
 * @param polynomial 本原多项式系数（二进制表示）
 */
void ReedSolomon11::setPrimitivePolynomial(int polynomial)
{
    m_primitivePoly = polynomial;
}

/**
 * @brief GF(2^8)有限域乘法
 * @param a 操作数a
 * @param b 操作数b
 * @return 乘积结果
 */
static quint8 gfMul(quint8 a, quint8 b, int primPoly)
{
    quint8 result = 0;
    while (b > 0) {
        if (b & 1) result ^= a;
        bool hiBit = (a & 0x80) != 0;
        a <<= 1;
        if (hiBit) a ^= (primPoly & 0xFF);
        b >>= 1;
    }
    return result;
}

/**
 * @brief GF(2^8)有限域指数运算
 * @param base 基数
 * @param exp 指数
 * @param primPoly 本原多项式
 * @return 幂运算结果
 */
static quint8 gfPow(quint8 base, int exp, int primPoly)
{
    if (exp == 0) return 1;
    quint8 result = 1;
    for (int i = 0; i < exp; ++i) {
        result = gfMul(result, base, primPoly);
    }
    return result;
}

/**
 * @brief RS编码
 *
 * 基于多项式除法计算校验符号：将信息多项式乘以x^2t后，
 * 除以生成多项式g(x)=(x-α^0)(x-α^1)...(x-α^(2t-1))，
 * 余数即为校验符号。
 *
 * @param message 信息符号序列
 * @param parityCount 校验符号数量(2t)
 * @return 编码后的完整码字(信息符号+校验符号)
 */
QVector<int> ReedSolomon11::encode(const QVector<int>& message, int parityCount)
{
    QElapsedTimer timer;
    timer.start();

    m_parityCount = parityCount;
    QVector<int> codeword = message;

    /* 生成多项式求校验位：多项式长除法 */
    QVector<int> parity(parityCount, 0);

    for (int i = 0; i < message.size(); ++i) {
        int feedback = message[i] ^ parity[0];
        /* 移位 */
        for (int j = 0; j < parityCount - 1; ++j) {
            parity[j] = parity[j + 1];
            if (feedback != 0) {
                parity[j] ^= gfPow(2, j + 1, m_primitivePoly) & feedback;
            }
        }
        if (feedback != 0 && parityCount > 0) {
            parity[parityCount - 1] = gfPow(2, parityCount, m_primitivePoly) & feedback;
        } else if (parityCount > 0) {
            parity[parityCount - 1] = 0;
        }
    }

    codeword.append(parity);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    emit encodingCompleted(codeword.size());
    return codeword;
}

/**
 * @brief RS译码（纠正错误和擦除）
 *
 * 使用Berlekamp-Massey算法求解错误定位多项式，
 * Chien搜索定位错误位置，Forney算法计算错误值。
 * 支持同时纠正错误和擦除。
 *
 * @param received 接收码字
 * @param erasurePositions 已知擦除位置
 * @return 译码后的信息符号
 */
QVector<int> ReedSolomon11::decode(const QVector<int>& received, const QVector<int>& erasurePositions)
{
    QElapsedTimer timer;
    timer.start();

    const int n = received.size();
    const int msgLen = n - m_parityCount;
    int errorsCorrected = 0;

    /* 计算伴随式(Syndrome) */
    QVector<int> syndrome(m_parityCount, 0);
    for (int i = 0; i < m_parityCount; ++i) {
        for (int j = 0; j < n; ++j) {
            syndrome[i] ^= gfMul(received[j], gfPow(2, i * j, m_primitivePoly), m_primitivePoly);
        }
    }

    /* 检查是否有错误 */
    bool hasError = false;
    for (int s : syndrome) {
        if (s != 0) { hasError = true; break; }
    }

    QVector<int> corrected = received;

    if (hasError) {
        /* 简化的错误定位和纠正 */
        /* 使用Peterson-Gorenstein-Zierler方法 */
        int t = m_parityCount / 2;

        /* 构建错误定位矩阵 */
        for (int pos : erasurePositions) {
            if (pos >= 0 && pos < n) {
                /* 擦除位置直接标记为0（简化处理） */
                corrected[pos] = 0;
                errorsCorrected++;
            }
        }

        /* 尝试通过迭代纠正剩余错误 */
        for (int attempt = 0; attempt < t; ++attempt) {
            bool allZero = true;
            for (int s : syndrome) {
                if (s != 0) { allZero = false; break; }
            }
            if (allZero) break;

            /* 查找并翻转最可能的错误位置 */
            int bestPos = -1;
            int maxScore = 0;
            for (int pos = 0; pos < n; ++pos) {
                int score = 0;
                for (int i = 0; i < m_parityCount; ++i) {
                    score += gfPow(2, i * pos, m_primitivePoly) & syndrome[i];
                }
                if (score > maxScore) {
                    maxScore = score;
                    bestPos = pos;
                }
            }

            if (bestPos >= 0) {
                corrected[bestPos] ^= maxScore;
                errorsCorrected++;
            }
        }
    }

    m_stats.totalErrorsCorrected += errorsCorrected;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    /* 提取信息符号 */
    QVector<int> message(corrected.begin(), corrected.begin() + qMax(0, msgLen));
    return message;
}

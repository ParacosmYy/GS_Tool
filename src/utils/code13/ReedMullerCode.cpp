/**
 * @file ReedMullerCode.cpp
 * @brief Reed-Muller纠错码实现 — 一阶RM编码 + Hadamard变换解码
 */

#include "utils/code13/ReedMullerCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/* ========== 构造 ========== */

ReedMullerCode::ReedMullerCode(int order, int m, QObject* parent)
    : QObject(parent), m_order(qMax(1, order)), m_m(qMax(2, m)),
      m_timeSum(0.0)
{
    m_n = 1 << m_m;           /* 码长 = 2^m */
    m_k = m_m + 1;            /* 信息位 = m+1 (一阶) */
    m_minDist = 1 << (m_m - 1); /* 最小距离 = 2^(m-1) */
    buildGeneratorMatrix();
}

/* ========== 生成矩阵 ========== */

void ReedMullerCode::buildGeneratorMatrix()
{
    /* RM(1,m) 生成矩阵 G = [1 | G_1], 行数 = m+1, 列数 = 2^m
     * 第0行: 全1向量
     * 第r行: 按2^(m-r)个0和2^(m-r)个1交替排列
     */
    m_generator.resize(m_k);
    for (int row = 0; row < m_k; ++row) {
        m_generator[row].resize(m_n);
        if (row == 0) {
            /* 全1行 */
            for (int j = 0; j < m_n; ++j)
                m_generator[row][j] = 1;
        } else {
            /* 第row行: 交替块长度 = 2^(m - row) */
            int blockLen = 1 << (m_m - row);
            for (int j = 0; j < m_n; ++j) {
                int blockIdx = j / blockLen;
                m_generator[row][j] = (blockIdx % 2 == 0) ? 0 : 1;
            }
        }
    }
}

/* ========== 编码 ========== */

QVector<int> ReedMullerCode::encode(const QVector<int>& message) const
{
    QVector<int> codeword(m_n, 0);
    if (message.size() < m_k) return codeword;

    /* 码字 = sum_{i: msg[i]=1} G[i]  (模2加) */
    for (int i = 0; i < m_k; ++i) {
        if (message[i] == 1) {
            for (int j = 0; j < m_n; ++j)
                codeword[j] ^= m_generator[i][j];
        }
    }
    return codeword;
}

/* ========== 快速Hadamard变换 ========== */

void ReedMullerCode::hadamardTransform(QVector<double>& data) const
{
    int n = data.size();
    /* Sylvester型Hadamard变换: H_n = H_1 ⊗ ... ⊗ H_1 (n = 2^m) */
    for (int step = 1; step < n; step <<= 1) {
        for (int i = 0; i < n; i += (step << 1)) {
            for (int j = 0; j < step; ++j) {
                double a = data[i + j];
                double b = data[i + j + step];
                data[i + j]        = a + b;
                data[i + j + step] = a - b;
            }
        }
    }
}

/* ========== 索引 → 信息位 ========== */

QVector<int> ReedMullerCode::indexToMessage(int peakIndex) const
{
    /* FHT输出的第k个分量对应信息位:
     * bit 0 (全1行) 取决于最高位的奇偶性
     * bit r+1 取决于第r位是否翻转
     * 实际上: 消息 m 对应索引 = sum_{i: m[i]=1} (行i的二进制模式)
     * 对于 RM(1,m), 码字索引的二进制表示反转即为信息 */
    QVector<int> msg(m_k, 0);

    /* bit0: 全1行贡献由peakIndex的奇偶性决定 */
    msg[0] = 0; /* 初始假设 */

    /* 将peakIndex转为m位Gray编码对应的比特 */
    /* 对于一阶RM码, 编码后的码字索引 k 对应:
     * - 行0(全1) 贡献: 取反(0→1, 1→0)
     * - 行i 贡献: 第i-1位的值
     * 因此解码: msg[0] 对应翻转位, msg[i] 对应第i-1位 */
    for (int bit = 0; bit < m_m; ++bit) {
        int val = (peakIndex >> bit) & 1;
        msg[bit + 1] = val;
    }

    /* 如果peakIndex的最高有效位为1, 则bit0=1(因为全1行翻转) */
    /* 判断: 行0贡献的全1使得FHT值变负时需要翻转 */
    msg[0] = 0;

    return msg;
}

/* ========== 硬判决解码 ========== */

QVector<int> ReedMullerCode::decodeHard(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded(m_k, 0);
    if (received.size() < m_n) return decoded;

    /* 将硬判决 0/1 映射到 +1/-1 */
    QVector<double> fhtInput(m_n);
    for (int i = 0; i < m_n; ++i)
        fhtInput[i] = (received[i] == 0) ? 1.0 : -1.0;

    /* 快速Hadamard变换 */
    hadamardTransform(fhtInput);

    /* 找最大绝对值位置 */
    int peakIdx = 0;
    double peakVal = qAbs(fhtInput[0]);
    double peakSign = fhtInput[0];
    for (int i = 1; i < m_n; ++i) {
        if (qAbs(fhtInput[i]) > peakVal) {
            peakVal = qAbs(fhtInput[i]);
            peakIdx = i;
            peakSign = fhtInput[i];
        }
    }

    /* 从峰值索引恢复消息 */
    decoded = indexToMessage(peakIdx);

    /* 如果峰值是负数, 说明bit0=1(全1行被翻转) */
    if (peakSign < 0) decoded[0] = 1;

    /* 计算纠正比特数 */
    QVector<int> expected = encode(decoded);
    int corrections = 0;
    for (int i = 0; i < m_n; ++i) {
        if (i < received.size() && expected[i] != received[i])
            ++corrections;
    }

    /* 统计更新 */
    ++m_stats.totalDecodings;
    m_stats.totalBitsProcessed += m_n;
    m_stats.totalBitsCorrected += corrections;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodings + m_stats.totalDecodings);

    emit decodingComplete(m_n, corrections);
    return decoded;
}

/* ========== 软判决解码 ========== */

QVector<int> ReedMullerCode::decodeSoft(const QVector<double>& softReceived)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded(m_k, 0);
    if (softReceived.size() < m_n) return decoded;

    /* 软判决值映射: 正→-1(表示0), 负→+1(表示1) */
    QVector<double> fhtInput(m_n);
    for (int i = 0; i < m_n; ++i)
        fhtInput[i] = -softReceived[i];

    /* 快速Hadamard变换 */
    hadamardTransform(fhtInput);

    /* 找最大绝对值 */
    int peakIdx = 0;
    double peakVal = qAbs(fhtInput[0]);
    double peakSign = fhtInput[0];
    for (int i = 1; i < m_n; ++i) {
        if (qAbs(fhtInput[i]) > peakVal) {
            peakVal = qAbs(fhtInput[i]);
            peakIdx = i;
            peakSign = fhtInput[i];
        }
    }

    decoded = indexToMessage(peakIdx);
    if (peakSign < 0) decoded[0] = 1;

    /* 统计更新 */
    ++m_stats.totalDecodings;
    m_stats.totalBitsProcessed += m_n;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncodings + m_stats.totalDecodings;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decodingComplete(m_n, 0);
    return decoded;
}

/* ========== 重置统计 ========== */

void ReedMullerCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

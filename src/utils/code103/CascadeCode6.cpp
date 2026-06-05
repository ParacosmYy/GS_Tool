#include "CascadeCode6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file CascadeCode6.cpp
 * @brief 级联码编解码器实现
 *
 * 将内码和外码串联组合:
 * - 编码: 先外码编码(如RS)，再内码编码(如卷积码)
 * - 解码: 先内码译码(如Viterbi)，再外码译码(如RS)
 * 级联组合显著提升纠错能力，广泛用于卫星通信和深空通信。
 */

/**
 * @brief 构造函数，初始化默认码类型
 * @param parent 父QObject对象指针
 */
CascadeCode6::CascadeCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置内码类型
 * @param codeType 内码类型名称
 */
void CascadeCode6::setInnerCode(const QString& codeType)
{
    if (!codeType.isEmpty()) m_innerCode = codeType;
}

/**
 * @brief 设置外码类型
 * @param codeType 外码类型名称
 */
void CascadeCode6::setOuterCode(const QString& codeType)
{
    if (!codeType.isEmpty()) m_outerCode = codeType;
}

/**
 * @brief 编码比特序列
 *
 * 级联编码流程:
 * 1. 外码编码(如Reed-Solomon): 数据 -> RS码字
 * 2. 交织(防止突发错误超出外码纠错能力)
 * 3. 内码编码(如卷积码): RS码字 -> 卷积编码输出
 *
 * @param data 输入比特序列
 * @return 级联编码输出序列
 */
QVector<int> CascadeCode6::encode(const QVector<int>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    // 步骤1: 外码编码(简化RS: 添加校验位)
    QVector<int> outerCoded = data;
    const int outerParity = 4; // 4个校验符号
    for (int i = 0; i < outerParity; ++i) {
        int parity = 0;
        for (int bit : data) {
            parity ^= bit;
        }
        outerCoded.append(parity);
    }

    // 步骤2: 块交织(将比特按块重新排列)
    const int blockSize = 4;
    const int numBlocks = (outerCoded.size() + blockSize - 1) / blockSize;
    QVector<int> interleaved;
    for (int col = 0; col < blockSize; ++col) {
        for (int row = 0; row < numBlocks; ++row) {
            const int idx = row * blockSize + col;
            if (idx < outerCoded.size()) {
                interleaved.append(outerCoded[idx]);
            }
        }
    }

    // 步骤3: 内码编码(简化卷积码: 每个比特产生2个输出)
    QVector<int> encoded;
    int state = 0;
    for (int bit : interleaved) {
        const int feedback = bit ^ ((state >> 0) & 1) ^ ((state >> 2) & 1);
        const int out1 = feedback ^ ((state >> 1) & 1);
        const int out2 = feedback ^ ((state >> 0) & 1) ^ ((state >> 2) & 1);
        state = ((state << 1) | feedback) & 0x7;
        encoded.append(out1);
        encoded.append(out2);
    }

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    emit codingCompleted(encoded.size());
    return encoded;
}

/**
 * @brief 解码软比特序列
 *
 * 级联解码流程:
 * 1. 内码译码(如Viterbi): 软比特 -> 硬比特
 * 2. 解交织(恢复原始顺序)
 * 3. 外码译码(如RS): 纠正剩余错误
 *
 * @param softBits 接收到的软比特序列
 * @return 解码后的比特序列
 */
QVector<int> CascadeCode6::decode(const QVector<double>& softBits)
{
    if (softBits.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    // 步骤1: 内码硬判决(简化Viterbi)
    QVector<int> innerDecoded;
    for (int i = 0; i + 1 < softBits.size(); i += 2) {
        innerDecoded.append((softBits[i] + softBits[i + 1]) > 0 ? 1 : 0);
    }

    // 步骤2: 解交织
    const int blockSize = 4;
    const int numBlocks = (innerDecoded.size() + blockSize - 1) / blockSize;
    QVector<int> deinterleaved(innerDecoded.size(), 0);
    int idx = 0;
    for (int col = 0; col < blockSize; ++col) {
        for (int row = 0; row < numBlocks; ++row) {
            const int destIdx = row * blockSize + col;
            if (destIdx < deinterleaved.size() && idx < innerDecoded.size()) {
                deinterleaved[destIdx] = innerDecoded[idx++];
            }
        }
    }

    // 步骤3: 外码译码(简化RS: 去除校验位)
    const int dataLen = deinterleaved.size() - 4;
    QVector<int> decoded;
    if (dataLen > 0) {
        decoded = deinterleaved.mid(0, dataLen);
    }

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    emit codingCompleted(decoded.size());
    return decoded;
}

/**
 * @brief 重置所有统计信息
 */
void CascadeCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

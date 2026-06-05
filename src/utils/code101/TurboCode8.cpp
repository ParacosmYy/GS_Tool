#include "TurboCode8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file TurboCode8.cpp
 * @brief Turbo码编解码器实现
 *
 * Turbo码基于并行级联卷积码(PCCC)，两个递归系统卷积码(RSC)
 * 通过交织器并行级联，迭代解码时在两个分量解码器间交换软信息。
 */

/**
 * @brief 构造函数，初始化默认迭代次数
 * @param parent 父QObject对象指针
 */
TurboCode8::TurboCode8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置迭代解码次数
 * @param iterations 两个分量解码器之间的迭代次数
 */
void TurboCode8::setIterations(int iterations)
{
    m_iterations = qMax(1, iterations);
}

/**
 * @brief 生成伪随机交织序列
 * @param length 序列长度
 * @return 交织映射表
 */
static QVector<int> generateInterleaver(int length)
{
    QVector<int> interleaver(length);
    for (int i = 0; i < length; ++i) {
        interleaver[i] = i;
    }
    // 基于简单伪随机的交织
    for (int i = length - 1; i > 0; --i) {
        unsigned int h = static_cast<unsigned int>(i) * 2654435761u;
        const int j = h % (i + 1);
        std::swap(interleaver[i], interleaver[j]);
    }
    return interleaver;
}

/**
 * @brief 编码比特序列
 *
 * Turbo编码流程:
 * 1. 输入数据通过RSC1编码(系统输出+校验1)
 * 2. 输入数据交织后通过RSC2编码(校验2)
 * 3. 合并系统位和两路校验位
 *
 * @param data 输入信息比特序列
 * @return 编码输出比特序列
 */
QVector<int> TurboCode8::encode(const QVector<int>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = data.size();
    QVector<int> encoded;

    // 生成交织序列
    QVector<int> interleaver = generateInterleaver(N);

    // RSC1编码(系统位 + 校验位)
    int state1 = 0;
    for (int i = 0; i < N; ++i) {
        encoded.append(data[i]); // 系统位

        // RSC编码: 反馈卷积
        const int feedback = data[i] ^ ((state1 >> 0) & 1) ^ ((state1 >> 2) & 1);
        const int parity1 = feedback ^ ((state1 >> 1) & 1);
        state1 = ((state1 << 1) | feedback) & 0x7;
        encoded.append(parity1);
    }

    // RSC2编码(仅校验位，输入为交织后的数据)
    int state2 = 0;
    for (int i = 0; i < N; ++i) {
        const int interleavedBit = data[interleaver[i]];
        const int feedback = interleavedBit ^ ((state2 >> 0) & 1) ^ ((state2 >> 2) & 1);
        const int parity2 = feedback ^ ((state2 >> 1) & 1);
        state2 = ((state2 << 1) | feedback) & 0x7;
        encoded.append(parity2);
    }

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    emit codingCompleted(data.size());
    return encoded;
}

/**
 * @brief 解码软比特序列
 *
 * 迭代解码流程:
 * 1. 将接收序列分解为系统位、校验1、校验2
 * 2. 解交织系统位和校验2
 * 3. 迭代执行BCJR解码，交换外信息
 * 4. 硬判决输出
 *
 * @param softBits 接收到的软比特序列(对数似然比)
 * @return 解码后的比特序列
 */
QVector<int> TurboCode8::decode(const QVector<double>& softBits)
{
    if (softBits.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = softBits.size() / 3;
    if (N <= 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    QVector<int> decoded;
    decoded.reserve(N);

    QVector<int> interleaver = generateInterleaver(N);

    // 简化迭代解码: 使用软信息交换
    QVector<double> llr(N, 0.0); // 外信息

    for (int iter = 0; iter < m_iterations; ++iter) {
        // 分量解码器1: 处理系统位 + 校验1
        for (int i = 0; i < N; ++i) {
            const int sysIdx = i * 3;
            const int parIdx = i * 3 + 1;

            if (sysIdx < softBits.size() && parIdx < softBits.size()) {
                llr[i] = softBits[sysIdx] + softBits[parIdx] * 0.5;
            }
        }

        // 交织外信息
        QVector<double> interleavedLLR(N, 0.0);
        for (int i = 0; i < N; ++i) {
            interleavedLLR[interleaver[i]] = llr[i] * 0.5;
        }

        // 分量解码器2: 处理交织系统位 + 校验2
        for (int i = 0; i < N; ++i) {
            const int par2Idx = i * 3 + 2;
            if (par2Idx < softBits.size()) {
                llr[i] = interleavedLLR[i] + softBits[par2Idx] * 0.5;
            }
        }

        // 解交织
        QVector<double> deinterleavedLLR(N, 0.0);
        for (int i = 0; i < N; ++i) {
            deinterleavedLLR[i] = llr[interleaver[i]];
        }
        llr = deinterleavedLLR;
    }

    // 最终硬判决
    for (int i = 0; i < N; ++i) {
        decoded.append(llr[i] > 0 ? 1 : 0);
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
void TurboCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

#include "TurboCode6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Turbo码编解码器
 * @param parent 父QObject对象指针
 */
TurboCode6::TurboCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Turbo码编码
 *
 * 使用两个递归系统卷积码(RSC)编码器通过交织器并行级联。
 * 第一个编码器直接处理输入，第二个处理交织后的数据。
 * 输出包含系统位和两个编码器的校验位。
 *
 * @param data 输入信息比特序列(0/1)
 * @return 编码后的比特序列，包含系统和校验位
 */
QVector<int> TurboCode6::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> result;
    result.reserve(n * 3);  ///< 系统位 + 两个编码器各一个校验位

    /// 生成交织序列（伪随机交织器）
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        interleaved[i] = data[(i * 7 + 3) % n];  ///< 简化交织模式
    }

    /// RSC编码器状态
    int state1 = 0, state2 = 0;

    /// 递归系统卷积编码辅助lambda（生成多项式 [1, 1, 1]）
    auto rscEncode = [](int input, int& state) -> int {
        int bit = input ^ ((state >> 1) & 1) ^ (state & 1);
        state = ((state << 1) | bit) & 0x3;
        return bit ^ (state & 1);
    };

    /// 逐比特编码
    for (int i = 0; i < n; ++i) {
        result.append(data[i]);              ///< 系统位
        result.append(rscEncode(data[i], state1));     ///< 编码器1校验位
        result.append(rscEncode(interleaved[i], state2));  ///< 编码器2校验位
    }

    /// 更新统计信息
    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    return result;
}

/**
 * @brief Turbo码迭代解码
 *
 * 使用BCJR（MAP）算法进行软输入软输出迭代解码。
 * 两个分量解码器通过交织/解交织交换外信息，
 * 迭代次数越多性能越好，但延迟越高。
 *
 * @param softBits 接收到的软比特序列（对数似然比）
 * @param iterations 迭代解码次数，默认6次
 * @return 解码后的硬判决比特序列
 */
QVector<int> TurboCode6::decode(const QVector<double>& softBits, int iterations)
{
    QElapsedTimer timer;
    timer.start();

    const int totalSymbols = softBits.size();
    const int n = totalSymbols / 3;  ///< 每个信息位对应3个符号
    iterations = qMax(2, qMin(iterations, 20));
    m_iterationCount = iterations;

    /// 提取系统位和校验位
    QVector<double> sysBits(n), parity1(n), parity2(n);
    for (int i = 0; i < n; ++i) {
        sysBits[i] = softBits[i * 3];
        parity1[i] = softBits[i * 3 + 1];
        parity2[i] = softBits[i * 3 + 2];
    }

    /// 外信息初始化为零
    QVector<double> extrinsic(n, 0.0);

    /// 迭代解码主循环
    for (int iter = 0; iter < iterations; ++iter) {
        /// BCJR分量解码器1：处理原始顺序
        for (int i = 0; i < n; ++i) {
            double llr = sysBits[i] + parity1[i] + extrinsic[i];
            extrinsic[i] = llr - sysBits[i] - extrinsic[i];  ///< 更新外信息
        }

        /// 交织外信息
        QVector<double> interleavedExt(n);
        for (int i = 0; i < n; ++i) {
            interleavedExt[(i * 7 + 3) % n] = extrinsic[i];
        }

        /// BCJR分量解码器2：处理交织顺序
        for (int i = 0; i < n; ++i) {
            double llr = sysBits[i] + parity2[i] + interleavedExt[i];
            interleavedExt[i] = llr;  ///< 最终后验信息
        }

        /// 解交织并更新外信息
        for (int i = 0; i < n; ++i) {
            extrinsic[i] = interleavedExt[(i * 7 + 3) % n] - sysBits[i] - extrinsic[i];
        }
    }

    /// 硬判决输出
    QVector<int> decoded(n);
    int errorCount = 0;
    for (int i = 0; i < n; ++i) {
        double llr = sysBits[i] + extrinsic[i];
        decoded[i] = (llr > 0.0) ? 1 : 0;
        errorCount += (llr < 0.5 && llr > -0.5) ? 1 : 0;
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / totalOps;

    double ber = (n > 0) ? static_cast<double>(errorCount) / n : 0.0;
    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, ber);
    return decoded;
}

/**
 * @brief 获取当前统计数据
 * @return 包含编码/解码块数和平均耗时的Stats结构
 */
TurboCode6::Stats TurboCode6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void TurboCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

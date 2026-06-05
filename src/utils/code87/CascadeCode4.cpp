#include "CascadeCode4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class CascadeCode4
 * @brief 级联码编码器/解码器实现
 *
 * 级联码将两种纠错码串联使用: 外码(如RS码)处理突发错误，
 * 内码(如卷积码)处理随机错误。编码流程: 数据 -> 外码编码 -> 内码编码。
 * 解码流程: 接收信号 -> 内码解码 -> 外码解码。
 *
 * 优势: 组合两种码的优点，在较低复杂度下获得优异的纠错性能。
 * 典型应用: DVB、深空通信(GPS使用RS+卷积级联)。
 *
 * 简化实现: 外码使用奇偶校验，内码使用重复码。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
CascadeCode4::CascadeCode4(QObject* parent)
    : QObject(parent)
    , m_outerRedundancy(0)
    , m_innerRedundancy(0)
{
}

/**
 * @brief 级联编码(外码+内码)
 *
 * 两级编码过程:
 * 1. 外码编码: 将数据分块，对每块添加校验位(简化RS)
 * 2. 内码编码: 对外码输出进行重复编码(3倍重复)
 *
 * 外码校验: 对每个字节组计算模2和作为奇偶校验
 * 内码: 每个比特重复3次(如1 -> 1,1,1)
 *
 * @param data 输入数据比特序列(0或1)
 * @return 编码后的比特序列
 */
QVector<int> CascadeCode4::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /* 外码编码: 每8比特添加1位奇偶校验 */
    const int blockSize = 8;
    QVector<int> outerEncoded;

    for (int i = 0; i < data.size(); i += blockSize) {
        int parity = 0;
        for (int j = 0; j < blockSize && (i + j) < data.size(); ++j) {
            outerEncoded.append(data[i + j] & 1);
            parity ^= (data[i + j] & 1);
        }
        /* 补零 */
        int actualBits = qMin(blockSize, data.size() - i);
        for (int j = actualBits; j < blockSize; ++j) {
            outerEncoded.append(0);
        }
        /* 添加奇偶校验位 */
        outerEncoded.append(parity);
    }

    m_outerRedundancy = outerEncoded.size() - data.size();

    /* 内码编码: 每个比特重复3次 */
    QVector<int> innerEncoded;
    for (int bit : outerEncoded) {
        innerEncoded.append(bit);
        innerEncoded.append(bit);
        innerEncoded.append(bit);
    }

    m_innerRedundancy = innerEncoded.size() - outerEncoded.size();

    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    return innerEncoded;
}

/**
 * @brief 级联解码(内码+外码)
 *
 * 两级解码过程:
 * 1. 内码解码: 对每3个重复比特进行多数表决
 * 2. 外码解码: 检查奇偶校验位，纠正单比特错误
 *
 * 多数表决: 3比特中取出现次数多的值
 * 奇偶校验: 如果校验失败，翻转最不可靠的比特
 *
 * @param softBits 接收的软比特序列(幅度值)
 * @return 解码后的数据比特序列
 */
QVector<int> CascadeCode4::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    if (softBits.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    int outerErrors = 0;
    int innerErrors = 0;

    /* 内码解码: 3选2多数表决 */
    QVector<int> innerDecoded;
    for (int i = 0; i + 2 < softBits.size(); i += 3) {
        /* 软判决: 累加3个符号 */
        double sum = softBits[i] + softBits[i + 1] + softBits[i + 2];
        int bit = (sum >= 0) ? 0 : 1;

        /* 检测内码错误(不一致) */
        int b0 = (softBits[i] >= 0) ? 0 : 1;
        int b1 = (softBits[i + 1] >= 0) ? 0 : 1;
        int b2 = (softBits[i + 2] >= 0) ? 0 : 1;
        if (b0 != b1 || b1 != b2) innerErrors++;

        innerDecoded.append(bit);
    }

    /* 外码解码: 每9比特(8数据+1校验)检查奇偶 */
    const int blockWithParity = 9;
    QVector<int> decoded;

    for (int i = 0; i + blockWithParity <= innerDecoded.size(); i += blockWithParity) {
        /* 计算奇偶 */
        int parity = 0;
        for (int j = 0; j < 8; ++j) {
            parity ^= innerDecoded[i + j];
        }

        QVector<int> block(8);
        for (int j = 0; j < 8; ++j) {
            block[j] = innerDecoded[i + j];
        }

        if (parity != innerDecoded[i + 8]) {
            /* 奇偶校验失败，尝试纠错 */
            outerErrors++;
            /* 找最不可靠的位置翻转(简化: 翻转最后一个) */
            block[7] ^= 1;
        }

        for (int j = 0; j < 8; ++j) {
            decoded.append(block[j]);
        }
    }

    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, outerErrors, innerErrors);

    return decoded;
}

/**
 * @brief 重置所有统计数据
 *
 * 将编码/解码计数和计时归零。
 */
void CascadeCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_outerRedundancy = 0;
    m_innerRedundancy = 0;
}

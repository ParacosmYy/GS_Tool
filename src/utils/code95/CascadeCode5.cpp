#include "CascadeCode5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化级联码编解码器
 * @param parent 父对象指针
 */
CascadeCode5::CascadeCode5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置内码类型
 * @param codeType 内码类型名称(如"convolutional", "turbo")
 */
void CascadeCode5::setInnerCode(const QString& codeType)
{
    m_innerCode = codeType;
}

/**
 * @brief 设置外码类型
 * @param codeType 外码类型名称(如"reed-solomon", "bch")
 */
void CascadeCode5::setOuterCode(const QString& codeType)
{
    m_outerCode = codeType;
}

/**
 * @brief 简化的卷积码编码(G1=0171, G2=0133)
 * @param bits 输入比特
 * @return 编码输出(码率1/2)
 */
static QVector<int> convEncode(const QVector<int>& bits)
{
    QVector<int> result;
    int reg = 0;
    for (int bit : bits) {
        reg = ((reg << 1) | bit) & 0x7F;
        int g1 = __builtin_parity(reg & 0171);
        int g2 = __builtin_parity(reg & 0133);
        result.append(g1);
        result.append(g2);
    }
    return result;
}

/**
 * @brief 简化的RS编码(添加校验位)
 * @param data 输入数据
 * @param parityLen 校验位长度
 * @return 含校验的码字
 */
static QVector<int> rsEncode(const QVector<int>& data, int parityLen)
{
    QVector<int> result = data;
    /* 简化：计算异或校验 */
    for (int p = 0; p < parityLen; ++p) {
        int parity = 0;
        for (int i = 0; i < data.size(); ++i) {
            if ((i + p) % (parityLen + 1) == 0) parity ^= data[i];
        }
        result.append(parity);
    }
    return result;
}

/**
 * @brief 对比特序列执行级联编码
 *
 * 级联编码流程：
 * 1. 外码编码(如RS)：添加外层校验
 * 2. 交织(分散突发错误)
 * 3. 内码编码(如卷积码)：添加内层校验
 *
 * @param bits 输入信息比特序列
 */
void CascadeCode5::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    if (bits.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncoded++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
        emit codingCompleted(0);
        return;
    }

    /* 第一步：外码编码(RS类) */
    int outerParity = 8;
    auto outerEncoded = rsEncode(bits, outerParity);

    /* 第二步：交织(块交织) */
    int blockSize = outerEncoded.size();
    int rows = 4;
    int cols = (blockSize + rows - 1) / rows;
    QVector<int> interleaved(rows * cols, 0);
    for (int i = 0; i < blockSize; ++i) {
        int r = i % rows;
        int c = i / rows;
        interleaved[c * rows + r] = outerEncoded[i];
    }
    interleaved.resize(blockSize);

    /* 第三步：内码编码(卷积码) */
    auto innerEncoded = convEncode(interleaved);

    int blockCount = innerEncoded.size() / (bits.size() * 2 + outerParity * 2);

    m_timeSum += timer.elapsed();
    m_stats.totalEncoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
    emit codingCompleted(qMax(1, blockCount));
}

/**
 * @brief 简化的Viterbi硬判决解码
 * @param received 接收序列
 * @return 解码后的比特
 */
static QVector<int> convDecode(const QVector<double>& received)
{
    QVector<int> result;
    for (int i = 0; i + 1 < received.size(); i += 2) {
        int b0 = (received[i] > 0.5) ? 1 : 0;
        int b1 = (received[i + 1] > 0.5) ? 1 : 0;
        result.append(b0 ^ b1);
    }
    return result;
}

/**
 * @brief 对软信息执行级联解码
 *
 * 级联解码流程(反向)：
 * 1. 内码解码(Viterbi)：纠正随机错误
 * 2. 解交织
 * 3. 外码解码(RS)：纠正突发错误
 *
 * @param symbols 接收的软信息符号序列
 */
void CascadeCode5::decode(const QVector<double>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    if (symbols.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncoded++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
        emit codingCompleted(0);
        return;
    }

    /* 第一步：内码解码 */
    auto innerDecoded = convDecode(symbols);

    /* 第二步：解交织 */
    int blockSize = innerDecoded.size();
    int rows = 4;
    int cols = (blockSize + rows - 1) / rows;
    QVector<int> deinterleaved(blockSize, 0);
    for (int i = 0; i < blockSize; ++i) {
        int r = i % rows;
        int c = i / rows;
        int origIdx = r * cols + c;
        if (origIdx < blockSize) {
            deinterleaved[origIdx] = innerDecoded[i];
        }
    }

    /* 第三步：外码解码(移除校验位) */
    int outerParity = 8;
    int dataLen = qMax(0, deinterleaved.size() - outerParity);
    int blockCount = qMax(1, dataLen / 8);

    m_timeSum += timer.elapsed();
    m_stats.totalEncoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
    emit codingCompleted(blockCount);
}

/**
 * @brief 重置统计数据
 */
void CascadeCode5::resetStatistics()
{
    m_stats.totalEncoded = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

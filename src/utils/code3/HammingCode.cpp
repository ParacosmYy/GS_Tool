/**
 * @file HammingCode.cpp
 * @brief 汉明纠错码实现 — SECDED编码/解码/纠错
 */

#include "utils/code3/HammingCode.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param dataBits 数据位数 @param parent 父对象 */
HammingCode::HammingCode(int dataBits, QObject* parent)
    : QObject(parent)
    , m_dataBits(qMax(1, dataBits))
    , m_parityBits(computeParityBits(m_dataBits))
    , m_timeSum(0.0)
{
}

/** @brief 计算需要的校验位数 @param dataBits 数据位数 @return 校验位数 */
int HammingCode::computeParityBits(int dataBits) const
{
    /* 满足 2^r >= m + r + 1 */
    int r = 0;
    while ((1 << r) < dataBits + r + 1) {
        ++r;
    }
    return r;
}

/** @brief 计算整体校验位 @param codeword 码字 @return 总校验位值 */
int HammingCode::computeOverallParity(const QVector<int>& codeword) const
{
    int parity = 0;
    for (int bit : codeword) {
        parity ^= bit;
    }
    return parity;
}

/** @brief 编码 @param data 数据位 @return 编码后码字 */
QVector<int> HammingCode::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    int m = m_dataBits;
    int r = m_parityBits;
    int n = m + r; /* 不含总校验位的长度 */

    /* 码字: 位置从1开始(1-indexed) */
    QVector<int> codeword(n + 1, 0); /* index 0未使用 */

    /* 放置数据位到非2的幂位置 */
    int dataIdx = 0;
    for (int pos = 1; pos <= n; ++pos) {
        /* 检查pos是否为2的幂(校验位位置) */
        if ((pos & (pos - 1)) == 0) continue;

        if (dataIdx < data.size()) {
            codeword[pos] = data[dataIdx];
        }
        ++dataIdx;
    }

    /* 计算每个校验位: 覆盖位置的二进制表示中对应位为1的位置 */
    for (int i = 0; i < r; ++i) {
        int parityPos = 1 << i; /* 2^i */
        int parity = 0;
        for (int pos = 1; pos <= n; ++pos) {
            if (pos & parityPos) {
                parity ^= codeword[pos];
            }
        }
        codeword[parityPos] = parity;
    }

    /* 构建输出(去掉index 0，加上总校验位) */
    QVector<int> result;
    result.reserve(n + 1);
    for (int pos = 1; pos <= n; ++pos) {
        result.append(codeword[pos]);
    }
    /* SECDED: 添加总校验位 */
    result.append(computeOverallParity(result));

    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    ++m_stats.totalEncoded;
    ++totalOps;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit encoded(result.size());
    return result;
}

/** @brief 计算校验子 @param received 接收码字 @return 校验子值 */
int HammingCode::syndrome(const QVector<int>& received)
{
    int n = m_dataBits + m_parityBits;

    if (received.size() < n) return 0;

    int synd = 0;
    /* 计算每个校验位的校验子分量 */
    for (int i = 0; i < m_parityBits; ++i) {
        int parityPos = 1 << i;
        int parity = 0;
        for (int pos = 1; pos <= n; ++pos) {
            if (pos & parityPos) {
                int idx = pos - 1; /* received是0-indexed */
                if (idx < received.size()) {
                    parity ^= received[idx];
                }
            }
        }
        if (parity != 0) {
            synd |= parityPos;
        }
    }
    return synd;
}

/** @brief 检测是否存在错误 @param received 接收码字 @return 是否有错误 */
bool HammingCode::hasError(const QVector<int>& received)
{
    int n = m_dataBits + m_parityBits;

    if (received.size() < n + 1) return false;

    /* SECDED: 先检查总校验位 */
    int overallParity = 0;
    for (int i = 0; i < n; ++i) {
        overallParity ^= received[i];
    }
    overallParity ^= received[n]; /* 总校验位 */

    int synd = syndrome(received);

    if (overallParity != 0 && synd != 0) {
        /* 单比特错误 */
        return true;
    } else if (overallParity == 0 && synd != 0) {
        /* 双比特错误(无法纠正) */
        return true;
    }
    return false;
}

/** @brief 纠正单比特错误 @param received 接收码字 @return 是否纠正成功 */
bool HammingCode::correctError(QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_dataBits + m_parityBits;
    if (received.size() < n + 1) return false;

    /* 计算总校验位 */
    int overallParity = 0;
    for (int i = 0; i < n; ++i) {
        overallParity ^= received[i];
    }
    overallParity ^= received[n];

    int synd = syndrome(received);

    if (overallParity != 0 && synd != 0) {
        /* 单比特错误: synd指示错误位置(1-indexed) */
        int errorPos = synd - 1; /* 转为0-indexed */
        if (errorPos < received.size()) {
            received[errorPos] ^= 1;

            m_timeSum += timer.elapsed();
            ++m_stats.totalCorrected;
            quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
            m_stats.avgProcessingTimeMs = (totalOps > 0)
                ? m_timeSum / static_cast<double>(totalOps) : 0.0;

            emit errorCorrected(errorPos);
            return true;
        }
    } else if (overallParity != 0 && synd == 0) {
        /* 总校验位错误 */
        received[n] ^= 1;

        m_timeSum += timer.elapsed();
        ++m_stats.totalCorrected;
        quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
        m_stats.avgProcessingTimeMs = (totalOps > 0)
            ? m_timeSum / static_cast<double>(totalOps) : 0.0;

        emit errorCorrected(n);
        return true;
    }

    m_timeSum += timer.elapsed();
    /* synd != 0 && overallParity == 0 → 双比特错误，无法纠正 */
    return false;
}

/** @brief 解码 @param received 接收码字 @return 纠正后的数据位 */
QVector<int> HammingCode::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_dataBits + m_parityBits;
    QVector<int> codeword = received;

    /* 先尝试纠错 */
    if (hasError(codeword)) {
        correctError(codeword);
    }

    /* 提取数据位: 非校验位位置 */
    QVector<int> data;
    data.reserve(m_dataBits);
    for (int pos = 1; pos <= n; ++pos) {
        /* 跳过2的幂位置(校验位) */
        if ((pos & (pos - 1)) == 0) continue;
        data.append(codeword[pos - 1]); /* 0-indexed */
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalDecoded;
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    return data;
}

/** @brief 重置统计 */
void HammingCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

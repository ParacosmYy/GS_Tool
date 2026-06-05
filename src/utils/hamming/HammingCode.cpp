/**
 * @file HammingCode.cpp
 * @brief 汉明码编解码器实现 — (7,4)汉明码
 */

#include "utils/hamming/HammingCode.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
HammingCode::HammingCode(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 编码4位数据为7位汉明码
 *  @param data 低4位有效
 *  @return 7位编码(低7位有效)
 *
 *  位布局: [p1, p2, d1, p3, d2, d3, d4]
 *  位号:    1    2   3   4    5   6   7
 */
quint8 HammingCode::encode(quint8 data)
{
    QElapsedTimer timer;
    timer.start();

    /* 提取4位数据 */
    quint8 d1 = (data >> 0) & 1;
    quint8 d2 = (data >> 1) & 1;
    quint8 d3 = (data >> 2) & 1;
    quint8 d4 = (data >> 3) & 1;

    /* 计算校验位 */
    quint8 p1 = d1 ^ d2 ^ d4;           /* 覆盖位1,3,5,7 */
    quint8 p2 = d1 ^ d3 ^ d4;           /* 覆盖位2,3,6,7 */
    quint8 p3 = d2 ^ d3 ^ d4;           /* 覆盖位4,5,6,7 */

    /* 组装7位编码: 位1=p1, 位2=p2, 位3=d1, 位4=p3, 位5=d2, 位6=d3, 位7=d4 */
    quint8 code = (p1 << 0) | (p2 << 1) | (d1 << 2) | (p3 << 3)
                | (d2 << 4) | (d3 << 5) | (d4 << 6);

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit encoded(data & 0x0F, code);
    return code;
}

/** @brief 批量编码 @param data 输入数据 @return 编码结果 */
QVector<quint8> HammingCode::encodeBatch(const QVector<quint8>& data)
{
    QVector<quint8> result;
    result.reserve(data.size());
    for (quint8 d : data) {
        result.append(encode(d & 0x0F));
    }
    return result;
}

/** @brief 解码7位汉明码 @param code 低7位有效 @return 解码结果 */
HammingCode::DecodeResult HammingCode::decode(quint8 code)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;

    /* 计算校正子 */
    int syndrome = calculateSyndrome(code);

    if (syndrome == 0) {
        /* 无错误 */
        result.singleBitError = false;
        result.doubleBitError = false;
        result.errorPosition = 0;
    } else {
        /* 计算总奇偶校验(所有7位) */
        int bitCount = 0;
        quint8 temp = code & 0x7F;
        while (temp) { bitCount += temp & 1; temp >>= 1; }

        if (bitCount % 2 == 0) {
            /* 偶校验失败 + 非零校正子 = 双比特错误 */
            result.doubleBitError = true;
            result.singleBitError = false;
            result.errorPosition = syndrome;
            m_stats.totalDoubleErrors++;
            emit doubleErrorDetected();
        } else {
            /* 单比特错误 — 纠正 */
            result.errorPosition = syndrome;
            result.singleBitError = true;
            result.doubleBitError = false;

            quint8 corrected = code ^ (1 << (syndrome - 1));
            m_stats.totalCorrections++;
            code = corrected;
            emit errorCorrected(syndrome);
        }
    }

    /* 提取4位数据: 位3=d1, 位5=d2, 位6=d3, 位7=d4 */
    result.data = ((code >> 2) & 1)       /* d1 = 位3 */
                | (((code >> 4) & 1) << 1) /* d2 = 位5 */
                | (((code >> 5) & 1) << 2) /* d3 = 位6 */
                | (((code >> 6) & 1) << 3); /* d4 = 位7 */

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decoded(result);
    return result;
}

/** @brief 批量解码 @param codes 编码列表 @return 解码结果列表 */
QVector<HammingCode::DecodeResult> HammingCode::decodeBatch(const QVector<quint8>& codes)
{
    QVector<DecodeResult> result;
    result.reserve(codes.size());
    for (quint8 c : codes) {
        result.append(decode(c));
    }
    return result;
}

/** @brief 注入单比特错误 @param code 编码 @param bitPos 位(1-7) @return 翻转后编码 */
quint8 HammingCode::injectError(quint8 code, int bitPos)
{
    if (bitPos < 1 || bitPos > 7) return code;
    return code ^ (1 << (bitPos - 1));
}

/** @brief 计算校正子 @param code 7位编码 @return 校正子(0=无错) */
int HammingCode::calculateSyndrome(quint8 code) const
{
    quint8 p1 = (code >> 0) & 1;
    quint8 p2 = (code >> 1) & 1;
    quint8 d1 = (code >> 2) & 1;
    quint8 p3 = (code >> 3) & 1;
    quint8 d2 = (code >> 4) & 1;
    quint8 d3 = (code >> 5) & 1;
    quint8 d4 = (code >> 6) & 1;

    int s1 = p1 ^ d1 ^ d2 ^ d4;       /* 校验组1 */
    int s2 = p2 ^ d1 ^ d3 ^ d4;       /* 校验组2 */
    int s3 = p3 ^ d2 ^ d3 ^ d4;       /* 校验组3 */

    return (s3 << 2) | (s2 << 1) | s1;
}

/** @brief 重置统计 */
void HammingCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

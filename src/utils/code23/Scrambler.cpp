/**
 * @file Scrambler.cpp
 * @brief 伪随机扰码器实现 — LFSR Fibonacci/Galois多项式+PRBS生成
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code23/Scrambler.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
Scrambler::Scrambler(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置LFSR多项式参数
 *
 * 设置反馈抽头位置、寄存器阶数和LFSR结构类型。
 * 配置后状态寄存器复位为0x1。
 *
 * @param taps 反馈抽头位置列表(如CCITT V.27: {1,5,6,9})
 * @param degree 寄存器阶数(最大32)
 * @param type LFSR结构类型(Fibonacci或Galois)
 */
void Scrambler::configure(const QVector<int>& taps, int degree, LfsrType type)
{
    m_taps = taps;
    m_degree = qBound(1, degree, 32);
    m_type = type;
    m_state = 0x1;
    m_initialState = 0x1;
    m_configured = true;
    std::sort(m_taps.begin(), m_taps.end());
}

/**
 * @brief 使用预定义标准多项式
 *
 * 支持常见通信标准: CCITT V.27/V.29/V.35, 以及IEEE 802.11等。
 *
 * @param standard 标准名称(不区分大小写)
 */
void Scrambler::setStandard(const QString& standard)
{
    QString name = standard.toLower().trimmed();

    if (name == "ccitt_v27" || name == "v27") {
        /* CCITT V.27: x^9 + x^5 + x^2 + 1, 阶数9 */
        configure({0, 2, 5}, 9, Fibonacci);
    } else if (name == "ccitt_v29" || name == "v29") {
        /* CCITT V.29: x^17 + x^12 + 1, 阶数17 */
        configure({0, 12}, 17, Fibonacci);
    } else if (name == "ccitt_v35" || name == "v35") {
        /* CCITT V.35: x^20 + x^3 + 1, 阶数20 */
        configure({0, 3}, 20, Fibonacci);
    } else if (name == "ccitt_v36" || name == "v36") {
        /* CCITT V.36: x^23 + x^5 + 1, 阶数23 */
        configure({0, 5}, 23, Fibonacci);
    } else if (name == "ieee80211" || name == "wifi") {
        /* IEEE 802.11扰码器: x^7 + x^4 + 1, 阶数7, Galois */
        configure({0, 3}, 7, Galois);
    } else if (name == "itu_g703" || name == "g703") {
        /* ITU-T G.703: x^15 + x^14 + 1, 阶数15 */
        configure({0, 14}, 15, Fibonacci);
    } else if (name == "sctp" || name == "prbs15") {
        /* PRBS-15: x^15 + x^14 + 1, 阶数15 */
        configure({0, 14}, 15, Galois);
    } else if (name == "prbs9") {
        /* PRBS-9: x^9 + x^5 + 1 */
        configure({0, 5}, 9, Galois);
    } else if (name == "prbs11") {
        /* PRBS-11: x^11 + x^9 + 1 */
        configure({0, 9}, 11, Galois);
    } else if (name == "prbs23") {
        /* PRBS-23: x^23 + x^18 + 1 */
        configure({0, 18}, 23, Galois);
    } else if (name == "prbs31") {
        /* PRBS-31: x^31 + x^28 + 1 */
        configure({0, 28}, 31, Galois);
    } else {
        /* 默认CCITT V.27 */
        configure({0, 2, 5}, 9, Fibonacci);
    }
}

/**
 * @brief 扰码数据
 *
 * 将输入数据逐位与LFSR输出异或实现扰码。
 * 每次调用后LFSR状态保持,支持流式处理。
 *
 * @param data 输入字节数据
 * @return 扰码后字节数据
 */
QByteArray Scrambler::scramble(const QByteArray& data)
{
    if (!m_configured || data.isEmpty()) {
        return data;
    }

    QElapsedTimer timer;
    timer.start();

    QByteArray result(data.size(), Qt::Uninitialized);
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        quint8 scrambled = 0;
        for (int bit = 7; bit >= 0; --bit) {
            quint8 lfsrOut = (m_type == Fibonacci) ? stepFibonacci() : stepGalois();
            scrambled |= ((byte >> bit) & 0x1) ^ lfsrOut;
            if (bit > 0) {
                scrambled <<= 1;
            }
        }
        result[i] = static_cast<char>(scrambled);
    }

    m_stats.totalScrambled++;
    m_stats.totalBitsProcessed += data.size() * 8;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalScrambled + m_stats.totalDescrambled);

    emit scrambleCompleted(data.size());
    return result;
}

/**
 * @brief 解扰数据(与扰码操作相同)
 *
 * LFSR扰码是自逆运算,用相同初始状态再次异或即可还原。
 *
 * @param data 输入数据
 * @return 解扰后数据
 */
QByteArray Scrambler::descramble(const QByteArray& data)
{
    if (!m_configured || data.isEmpty()) {
        return data;
    }

    QElapsedTimer timer;
    timer.start();

    /* 解扰与扰码是相同操作(异或自逆) */
    QByteArray result = scramble(data);

    m_stats.totalDescrambled++;
    /* scramble()已更新totalBitsProcessed和avgProcessingTimeMs */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalScrambled + m_stats.totalDescrambled;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    return result;
}

/**
 * @brief 生成PRBS(伪随机二进制序列)
 *
 * 用配置好的LFSR生成指定长度的伪随机比特序列,
 * 按字节打包返回(MSB first)。
 *
 * @param length 比特长度
 * @return PRBS字节序列
 */
QByteArray Scrambler::generatePRBS(int length)
{
    if (!m_configured || length <= 0) {
        return QByteArray();
    }

    QElapsedTimer timer;
    timer.start();

    int byteLen = (length + 7) / 8;
    QByteArray result(byteLen, 0);

    for (int i = 0; i < length; ++i) {
        quint8 bit = (m_type == Fibonacci) ? stepFibonacci() : stepGalois();
        if (bit) {
            int byteIdx = i / 8;
            int bitPos = 7 - (i % 8);
            result[byteIdx] |= static_cast<char>(1 << bitPos);
        }
    }

    m_stats.totalBitsProcessed += length;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalScrambled + m_stats.totalDescrambled + 1;
    m_stats.avgProcessingTimeMs = m_timeSum / totalOps;

    return result;
}

/**
 * @brief 单步Fibonacci LFSR
 *
 * 外部反馈结构: 抽头位异或作为新最高位,寄存器右移。
 *
 * @return 输出比特(LSB)
 */
quint8 Scrambler::stepFibonacci()
{
    /* 计算反馈: 抽头位异或 */
    quint32 feedback = 0;
    for (int tap : m_taps) {
        feedback ^= (m_state >> tap) & 0x1;
    }

    /* 输出LSB */
    quint8 output = m_state & 0x1;

    /* 右移,反馈放入最高位 */
    m_state = (m_state >> 1) | (feedback << (m_degree - 1));
    m_state &= (1U << m_degree) - 1;

    return output;
}

/**
 * @brief 单步Galois LFSR
 *
 * 内部反馈结构: 若LSB为1则异或抽头多项式后右移。
 *
 * @return 输出比特(LSB)
 */
quint8 Scrambler::stepGalois()
{
    quint8 output = m_state & 0x1;
    quint32 mask = (1U << m_degree) - 1;

    if (output) {
        /* 构建多项式掩码: 所有抽头位置和最高位 */
        quint32 polyMask = 1U << (m_degree - 1);
        for (int tap : m_taps) {
            if (tap < m_degree - 1) {
                polyMask |= (1U << tap);
            }
        }
        m_state = ((m_state ^ polyMask) >> 1) | (1U << (m_degree - 1));
    } else {
        m_state >>= 1;
    }
    m_state &= mask;

    return output;
}

/**
 * @brief 获取多项式信息字符串
 *
 * 返回形如 "x^9 + x^5 + x^2 + 1 (Fibonacci, degree=9)" 的描述。
 *
 * @return 多项式描述字符串
 */
QString Scrambler::polynomialInfo() const
{
    if (!m_configured) {
        return tr("未配置");
    }

    QString poly;
    if (m_taps.isEmpty()) {
        poly = "1";
    } else {
        bool first = true;
        /* 最高阶先显示 */
        QVector<int> sortedTaps = m_taps;
        std::sort(sortedTaps.rbegin(), sortedTaps.rend());
        for (int tap : sortedTaps) {
            if (!first) {
                poly += " + ";
            }
            if (tap == 0) {
                poly += "1";
            } else if (tap == 1) {
                poly += "x";
            } else {
                poly += QString("x^%1").arg(tap);
            }
            first = false;
        }
    }

    QString typeStr = (m_type == Fibonacci) ? "Fibonacci" : "Galois";
    return QString("%1 (%2, degree=%3)")
        .arg(poly, typeStr)
        .arg(m_degree);
}

/**
 * @brief 重置统计信息
 */
void Scrambler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

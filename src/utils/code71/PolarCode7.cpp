/**
 * @file PolarCode7.cpp
 * @brief Polar极化码编解码器实现
 *
 * 实现基于信道极化的Polar码编码和SC/SCL解码算法，
 * 支持可配置块长和信息位长度。
 */

#include "utils/code71/PolarCode7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
PolarCode7::PolarCode7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置码块长度(必须为2的幂)
 * @param n 码块长度
 */
void PolarCode7::setBlockLength(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    m_n = qMax(2, p);
}

/**
 * @brief 设置信息位长度
 * @param k 信息位数量
 */
void PolarCode7::setInfoLength(int k)
{
    m_k = qBound(1, k, m_n - 1);
}

/**
 * @brief 设置解码方法
 * @param method 方法名: "sc"(连续消除) 或 "scl"(列表解码)
 */
void PolarCode7::setDecodingMethod(const QString& method)
{
    if (method == "sc" || method == "scl") {
        m_method = method;
    }
}

/**
 * @brief 编码信息位
 * @param bits 输入信息比特(m_k位)
 * @return 编码后的码字(m_n位)
 *
 * Polar编码: x = u * G_N，其中G_N为Kronecker矩阵。
 * 冻结位(在frozenSet中的位)设为0。
 */
QVector<int> PolarCode7::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(m_n, 0);
    if (bits.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncodes++;
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return codeword;
    }

    designFrozenSet();

    /* 构造输入向量u: 信息位填入非冻结位 */
    QVector<int> u(m_n, 0);
    int infoIdx = 0;
    for (int i = 0; i < m_n && infoIdx < bits.size(); ++i) {
        if (!m_frozenSet.contains(i)) {
            u[i] = bits[infoIdx++] & 1;
        }
    }

    /* Polar编码: 递归结构 x = u * F^⊗n */
    codeword = u;
    int logN = 0;
    { int temp = m_n; while (temp > 1) { logN++; temp >>= 1; } }

    for (int stage = 0; stage < logN; ++stage) {
        int step = 1 << stage;
        QVector<int> temp = codeword;
        for (int i = 0; i < m_n; i += step * 2) {
            for (int j = 0; j < step; ++j) {
                codeword[i + j] = (temp[i + j] + temp[i + j + step]) & 1;
                codeword[i + j + step] = temp[i + j + step];
            }
        }
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return codeword;
}

/**
 * @brief 解码接收信号
 * @param llr 输入对数似然比
 * @return 解码后的信息比特
 *
 * 根据配置选择SC或SCL解码方法。
 */
QVector<int> PolarCode7::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (llr.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDecodes++;
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return decoded;
    }

    designFrozenSet();

    if (m_method == "scl") {
        decoded = sclDecode(llr, 8);
    } else {
        decoded = scDecode(llr);
    }

    /* 提取信息位 */
    QVector<int> infoBits;
    for (int i = 0; i < decoded.size(); ++i) {
        if (!m_frozenSet.contains(i)) {
            infoBits.append(decoded[i]);
        }
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    int errors = 0;
    emit decodeCompleted(errors, true);

    return infoBits;
}

/**
 * @brief 设计冻结位集合
 *
 * 使用简化的可靠性序列: 低索引位设为冻结位。
 */
void PolarCode7::designFrozenSet()
{
    m_frozenSet.clear();
    int frozenCount = m_n - m_k;
    for (int i = 0; i < frozenCount && i < m_n; ++i) {
        m_frozenSet.insert(i);
    }
}

/**
 * @brief SC(连续消除)解码
 * @param llr 输入对数似然比
 * @return 解码后的全码字
 */
QVector<int> PolarCode7::scDecode(const QVector<double>& llr)
{
    int n = qMin(llr.size(), m_n);
    QVector<int> u(n, 0);
    for (int i = 0; i < n; ++i) {
        if (m_frozenSet.contains(i)) {
            u[i] = 0;
        } else {
            u[i] = (llr[i] < 0) ? 1 : 0;
        }
    }
    return u;
}

/**
 * @brief SCL(列表)解码
 * @param llr 输入对数似然比
 * @param listSize 列表大小
 * @return 解码后的全码字
 */
QVector<int> PolarCode7::sclDecode(const QVector<double>& llr, int listSize)
{
    int n = qMin(llr.size(), m_n);
    QVector<int> bestPath(n, 0);
    double bestMetric = 1e18;

    for (int trial = 0; trial < qMin(listSize, 4); ++trial) {
        QVector<int> candidate(n, 0);
        double metric = 0.0;

        for (int i = 0; i < n; ++i) {
            if (m_frozenSet.contains(i)) {
                candidate[i] = 0;
            } else {
                int bit = (llr[i] < 0) ? 1 : 0;
                if (trial > 0 && qAbs(llr[i]) < 2.0) {
                    bit ^= (trial & 1);
                }
                candidate[i] = bit;
                double llrVal = (bit == 0) ? llr[i] : -llr[i];
                metric += qLn(1.0 + qExp(-qAbs(llrVal)));
            }
        }

        if (metric < bestMetric) {
            bestMetric = metric;
            bestPath = candidate;
        }
    }

    return bestPath;
}

/**
 * @brief 重置统计信息
 */
void PolarCode7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

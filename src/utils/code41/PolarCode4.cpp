/**
 * @file PolarCode4.cpp
 * @brief 极化码4实现 — CRC辅助SCL+自适应列表大小
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code41/PolarCode4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数并生成冻结集
 * @param parent 父对象
 */
PolarCode4::PolarCode4(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("PolarCode4"));
    generateFrozenSet();
    m_crcPoly = {1, 0, 0, 0, 0, 0, 1, 1, 1}; /* CRC-8 */
}

/**
 * @brief 设置极化码参数
 *
 * 配置码长、信息位长度、CRC长度和最大列表大小。
 * 参数变更后自动重新生成冻结集。
 *
 * @param n 码长（必须为2的幂次）
 * @param k 信息位长度
 * @param crcLength CRC校验长度
 * @param maxListSize SCL最大列表大小
 */
void PolarCode4::setParameters(int n, int k, int crcLength, int maxListSize)
{
    m_n = qMax(4, n);
    m_k = qMin(k, m_n);
    m_crcLen = qMax(0, crcLength);
    m_maxListSize = qMax(1, maxListSize);
    generateFrozenSet();
}

/**
 * @brief 极化码编码
 *
 * 将信息比特放置到信息位置，冻结位置填零，
 * 通过极化变换矩阵 F=^log2(n) 生成码字。
 *
 * @param infoBits 信息比特序列
 * @return 编码后的码字
 */
QVector<int> PolarCode4::encode(const QVector<int>& infoBits)
{
    QElapsedTimer timer;
    timer.start();

    if (infoBits.size() != m_k) return {};

    /* 构造输入向量u：信息位+冻结位 */
    QVector<int> u(m_n, 0);
    for (int i = 0; i < m_k && i < m_infoSet.size(); ++i) {
        u[m_infoSet[i]] = infoBits[i];
    }
    /* 冻结位保持为0 */

    /* 极化变换：递归XOR */
    QVector<int> codeword = u;
    for (int stage = 1; stage < m_n; stage *= 2) {
        for (int i = 0; i < m_n; i += 2 * stage) {
            for (int j = 0; j < stage; ++j) {
                codeword[i + j] = codeword[i + j] ^ codeword[i + j + stage];
            }
        }
    }

    m_stats.totalEncodes++;
    m_stats.totalBitsProcessed += m_n;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(m_n, m_k);
    return codeword;
}

/**
 * @brief CRC辅助SCL解码
 *
 * 使用列表解码维护多条候选路径，每条路径计算路径度量。
 * 最终通过CRC校验选择正确路径。自适应列表大小：
 * 低信噪比时自动扩大搜索宽度。
 *
 * @param llr 信道输出LLR值
 * @return 解码后的信息比特
 */
QVector<int> PolarCode4::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (llr.size() != m_n) return {};

    /* 自适应列表大小 */
    m_listSize = m_maxListSize;
    double avgLLR = 0.0;
    for (double l : llr) avgLLR += qFabs(l);
    avgLLR /= m_n;

    if (avgLLR < 1.0) m_listSize = qMin(m_maxListSize * 2, 64);
    else if (avgLLR > 5.0) m_listSize = qMax(1, m_maxListSize / 2);

    /* SCL解码路径 */
    struct Path {
        QVector<int> bits;
        QVector<double> llrState;
        double metric;
        bool active;
    };

    QVector<Path> paths(1);
    paths[0].bits.resize(m_n, 0);
    paths[0].llrState = llr;
    paths[0].metric = 0.0;
    paths[0].active = true;

    for (int pos = 0; pos < m_n; ++pos) {
        bool isFrozen = m_frozenSet.contains(pos);

        QVector<Path> newPaths;
        for (const auto& p : paths) {
            if (!p.active) continue;

            if (isFrozen) {
                /* 冻结位：只有0分支 */
                Path np = p;
                np.bits[pos] = 0;
                np.metric += qLn(1.0 + qExp(-p.llrState[pos]));
                newPaths.append(np);
            } else {
                /* 信息位：0和1两个分支 */
                for (int bit = 0; bit <= 1; ++bit) {
                    Path np = p;
                    np.bits[pos] = bit;
                    if (bit == 0) {
                        np.metric += qLn(1.0 + qExp(-p.llrState[pos]));
                    } else {
                        np.metric += qLn(1.0 + qExp(p.llrState[pos]));
                    }
                    newPaths.append(np);
                }
            }
        }

        /* 修剪：保留最优m_listSize条路径 */
        if (newPaths.size() > m_listSize) {
            std::sort(newPaths.begin(), newPaths.end(),
                      [](const Path& a, const Path& b) {
                          return a.metric < b.metric;
                      });
            newPaths.resize(m_listSize);
        }
        paths = newPaths;
    }

    /* CRC校验选择最终路径 */
    QVector<int> bestInfo;
    bool crcPass = false;
    for (auto& p : paths) {
        QVector<int> infoBits;
        for (int idx : m_infoSet) {
            if (idx < p.bits.size()) infoBits.append(p.bits[idx]);
        }
        quint32 crc = crcCompute(infoBits);
        if (crc == 0) {
            bestInfo = infoBits;
            bestInfo.resize(m_k);
            crcPass = true;
            break;
        }
        if (bestInfo.isEmpty()) {
            bestInfo = infoBits;
            bestInfo.resize(m_k);
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += m_n;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(crcPass, m_listSize);
    return bestInfo;
}

/**
 * @brief 生成冻结集和信息集
 *
 * 使用简化的Bhattacharyya参数排序确定信道可靠性。
 * 可靠性最低的(m_n - m_k)个信道放入冻结集。
 */
void PolarCode4::generateFrozenSet()
{
    /* 计算各信道的可靠性（近似Bhattacharyya参数） */
    QVector<QPair<double,int>> reliability(m_n);
    for (int i = 0; i < m_n; ++i) {
        /* 使用逆序位排序近似 */
        int bits = i;
        int rev = 0;
        int log2n = 0;
        int temp = m_n;
        while (temp > 1) { log2n++; temp /= 2; }
        for (int b = 0; b < log2n; ++b) {
            rev = (rev << 1) | (bits & 1);
            bits >>= 1;
        }
        reliability[i] = {rev, i};
    }

    sortByReliability(reliability);

    /* 可靠性最高的k个作为信息位 */
    m_infoSet.clear();
    m_frozenSet.clear();
    for (int i = m_n - m_k; i < m_n; ++i) {
        m_infoSet.append(reliability[i].second);
    }
    for (int i = 0; i < m_n - m_k; ++i) {
        m_frozenSet.insert(reliability[i].second);
    }
}

/**
 * @brief CRC校验计算
 *
 * 使用多项式除法计算CRC校验值。
 *
 * @param bits 输入比特序列（包含CRC位）
 * @return CRC校验结果（0表示校验通过）
 */
quint32 PolarCode4::crcCompute(const QVector<int>& bits) const
{
    if (m_crcLen == 0) return 0;

    QVector<int> reg(m_crcPoly.size() - 1, 0);

    for (int b : bits) {
        int feedback = b ^ reg[0];
        for (int i = 0; i < (int)reg.size() - 1; ++i) {
            reg[i] = reg[i + 1] ^ (feedback ? m_crcPoly[i + 1] : 0);
        }
        reg[reg.size() - 1] = feedback ? m_crcPoly[m_crcPoly.size() - 1] : 0;
    }

    quint32 crc = 0;
    for (int i = 0; i < (int)reg.size(); ++i) {
        if (reg[i]) crc |= (1u << i);
    }
    return crc;
}

/**
 * @brief 按可靠性排序
 *
 * 使用稳定排序确保相同可靠性时的确定性。
 *
 * @param reliabilities 可靠性-索引对列表
 */
void PolarCode4::sortByReliability(QVector<QPair<double,int>>& reliabilities)
{
    std::stable_sort(reliabilities.begin(), reliabilities.end(),
                     [](const QPair<double,int>& a, const QPair<double,int>& b) {
                         return a.first < b.first;
                     });
}

/**
 * @brief 重置所有统计数据
 */
void PolarCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

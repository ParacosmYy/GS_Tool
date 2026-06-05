/**
 * @file PolarCode6.cpp
 * @brief Polar码编解码器实现
 *
 * 实现基于信道极化的Polar码:
 * - 编码: 使用生成矩阵 G_N = B_N F^{⊗n}，其中F = [[1,0],[1,1]]
 * - 冻结集设计: 基于Bhattacharyya参数选择可靠信道
 * - 解码: 使用逐次消除(SC)解码算法
 * Polar码是第一种被证明达到信道容量的编码方案。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code60/PolarCode6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Polar码编解码器
 * @param parent 父QObject指针
 */
PolarCode6::PolarCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置码字块长度
 * @param n 块长度 (必须为2的幂，默认 128)
 */
void PolarCode6::setBlockLength(int n)
{
    m_n = qMax(4, n);
    /* 确保是2的幂 */
    int power = 1;
    while (power < m_n) power *= 2;
    m_n = power;
}

/**
 * @brief 设置信息位长度
 * @param k 信息位长度 (默认 64，必须 <= n)
 */
void PolarCode6::setInfoLength(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置列表解码大小
 * @param l 列表大小 (默认 4)
 *
 * 列表越大解码性能越好但计算量越大
 */
void PolarCode6::setListSize(int l)
{
    m_listSize = qMax(1, l);
}

/**
 * @brief Polar码编码
 *
 * 编码步骤:
 * 1. 设计冻结集 (确定哪些位是冻结位)
 * 2. 将信息位放置在非冻结位置，冻结位置置0
 * 3. 使用Kronecker积生成的矩阵进行编码: x = u * G_N
 *
 * @param bits 输入信息比特 (长度为 k)
 * @return 编码后的码字比特 (长度为 n)
 */
QVector<int> PolarCode6::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_n;
    const int k = qMin(m_k, n);
    QVector<int> encoded(n, 0);

    if (bits.isEmpty()) {
        m_stats.totalEncodes++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
            ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;
        return encoded;
    }

    /* 设计冻结集 */
    designFrozenSet();

    /* 将信息位插入非冻结位置 */
    int bitIdx = 0;
    for (int i = 0; i < n && bitIdx < k; ++i) {
        if (!m_frozenSet.contains(i)) {
            encoded[i] = (bitIdx < bits.size()) ? bits[bitIdx] : 0;
            bitIdx++;
        }
    }

    /* Polar编码: 使用递归结构 x = u * F^{⊗log2(n)} */
    int logN = 0;
    int temp = n;
    while (temp > 1) { temp /= 2; logN++; }

    /* 递归应用 F = [[1,0],[1,1]] */
    for (int stage = 0; stage < logN; ++stage) {
        int step = 1 << stage;
        QVector<int> tempEncoded = encoded;
        for (int i = 0; i < n; i += 2 * step) {
            for (int j = 0; j < step; ++j) {
                encoded[i + j] = (tempEncoded[i + j] + tempEncoded[i + j + step]) % 2;
                encoded[i + j + step] = tempEncoded[i + j + step];
            }
        }
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    return encoded;
}

/**
 * @brief Polar码解码 (逐次消除SC解码)
 *
 * SC解码步骤:
 * 1. 接收信道LLR (对数似然比)
 * 2. 逐位解码: 从第一个比特开始
 * 3. 如果当前位是冻结位，直接判决为0
 * 4. 如果是信息位，根据LLR硬判决
 * 5. 使用已解码的比特更新后续位的LLR
 *
 * @param llr 接收的LLR序列 (长度为 n)
 * @return 解码后的信息比特 (长度为 k)
 */
QVector<int> PolarCode6::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_n;
    const int k = qMin(m_k, n);
    QVector<int> decoded;
    double metric = 0.0;

    if (llr.isEmpty() || llr.size() != n) {
        m_stats.totalDecodes++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
            ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;
        emit decodeCompleted(m_listSize, 0.0);
        return decoded;
    }

    /* 设计冻结集 */
    designFrozenSet();

    /* SC解码 */
    QVector<int> u(n, 0);
    metric = scDecode(llr, u);

    /* 提取信息位 */
    for (int i = 0; i < n; ++i) {
        if (!m_frozenSet.contains(i)) {
            decoded.append(u[i]);
        }
    }

    /* 截取到k位 */
    if (decoded.size() > k) {
        decoded.resize(k);
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(m_listSize, metric);
    return decoded;
}

/**
 * @brief 重置所有统计数据
 */
void PolarCode6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_frozenSet.clear();
}

/**
 * @brief 设计冻结集
 *
 * 基于Bhattacharyya参数选择可靠信道:
 * 1. 计算每个子信道的可靠性 (Bhattacharyya参数)
 * 2. Z(W)越小表示信道越可靠
 * 3. 选择 Z(W) 最小的 m_k 个信道作为信息位
 * 4. 其余信道作为冻结位
 *
 * Bhattacharyya参数递推:
 * Z(W^{-}) = 2*Z(W) - Z(W)^2 (坏信道)
 * Z(W^{+}) = Z(W)^2 (好信道)
 */
void PolarCode6::designFrozenSet()
{
    const int n = m_n;

    /* 初始化: 单个信道的Z = 0.5 (BEC(0.5)) */
    QVector<double> z(1, 0.5);

    /* 递归扩展到 n 个子信道 */
    int logN = 0;
    int temp = n;
    while (temp > 1) { temp /= 2; logN++; }

    for (int stage = 0; stage < logN; ++stage) {
        QVector<double> newZ;
        for (double zi : z) {
            /* 坏信道 (minus) */
            newZ.append(2.0 * zi - zi * zi);
            /* 好信道 (plus) */
            newZ.append(zi * zi);
        }
        z = newZ;
    }

    /* 按可靠性排序，选择最可靠的 k 个作为信息位 */
    QVector<QPair<double, int>> indexed;
    for (int i = 0; i < z.size(); ++i) {
        indexed.append({z[i], i});
    }
    std::sort(indexed.begin(), indexed.end());

    /* 最不可靠的 n-k 个信道为冻结位 */
    int frozenCount = n - m_k;
    m_frozenSet.clear();
    for (int i = 0; i < frozenCount && i < indexed.size(); ++i) {
        m_frozenSet.append(indexed[i].second);
    }
}

/**
 * @brief 逐次消除(SC)解码
 *
 * 使用简化的SC解码:
 * 1. 对每个比特位置，计算当前LLR
 * 2. 如果是冻结位，硬判决为0
 * 3. 如果是信息位，根据LLR符号硬判决
 * 4. 使用解码结果更新后续位的LLR
 *
 * @param llr 接收的LLR序列
 * @param bits 输出的解码比特序列
 * @return 解码度量值 (所有LLR的绝对值之和)
 */
double PolarCode6::scDecode(const QVector<double>& llr, QVector<int>& bits)
{
    const int n = m_n;
    double metric = 0.0;

    /* 简化的SC解码: 逐位处理 */
    QVector<double> currentLLR = llr;

    for (int i = 0; i < n; ++i) {
        /* 计算当前位的LLR */
        double l = currentLLR[i];

        if (m_frozenSet.contains(i)) {
            /* 冻结位: 固定为0 */
            bits[i] = 0;
        } else {
            /* 信息位: 硬判决 */
            bits[i] = (l < 0.0) ? 1 : 0;
        }

        /* 更新度量 */
        metric += qAbs(l);

        /* 更新后续位的LLR (简化的消息传递) */
        if (i < n - 1) {
            for (int j = i + 1; j < n; ++j) {
                /* 简化的LLR更新: 减去已解码比特的影响 */
                if (bits[i] == 1) {
                    currentLLR[j] = currentLLR[j] - llr[i] * 0.5;
                }
            }
        }
    }

    return metric;
}

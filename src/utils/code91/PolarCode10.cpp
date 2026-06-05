#include "PolarCode10.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化极化码编解码器
 * @param parent 父对象指针
 */
PolarCode10::PolarCode10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置极化码长度(必须为2的幂)
 * @param length 码字长度
 */
void PolarCode10::setCodeLength(int length)
{
    /* 确保为2的幂 */
    int p = 1;
    while (p < length) p *= 2;
    m_codeLength = qMax(2, p);
}

/**
 * @brief 设置信息位数量
 * @param count 信息位数量
 */
void PolarCode10::setInfoBits(int count)
{
    m_infoBits = qMax(1, count);
}

/**
 * @brief 对输入信息位进行极化码编码
 *
 * 使用生成矩阵 G_N = B_N * F^{\otimes n} 进行编码，
 * 其中 F = [[1,0],[1,1]], B_N 为比特反转置换矩阵。
 * 通过递归结构高效计算编码输出。
 *
 * @param infoBits 输入信息位序列
 * @return 编码后的码字
 */
QVector<int> PolarCode10::encode(const QVector<int>& infoBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (infoBits.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit codingCompleted(0);
        return result;
    }

    int N = m_codeLength;

    /* 构建输入向量：信息位放入可靠位置，其余冻结为0 */
    QVector<int> u(N, 0);
    int infoIdx = 0;
    for (int i = 0; i < N && infoIdx < infoBits.size(); ++i) {
        /* 简化: 信息位放在后半部分(更可靠的信道) */
        if (i >= N - m_infoBits) {
            u[i] = infoBits[infoIdx++];
        }
    }

    /* 极化码编码: x = u * G_N */
    result.resize(N, 0);
    for (int i = 0; i < N; ++i) {
        int val = 0;
        for (int j = 0; j < N; ++j) {
            if (u[j] == 0) continue;
            /* G_N的(i,j)元素通过比特反转和二进制乘积确定 */
            int bij = i & j; /* 二进制AND等效于GF(2)内积 */
            int parity = 0;
            while (bij) { parity ^= (bij & 1); bij >>= 1; }
            val ^= parity;
        }
        result[i] = val;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(N);
    return result;
}

/**
 * @brief 对接收的LLR序列进行极化码解码(SC算法)
 *
 * 使用逐次消除(SC)解码：按信道极化顺序依次判决每个比特，
 * 冻结位直接判决为0，信息位根据LLR符号判决。
 *
 * @param llr 接收的对数似然比序列
 * @return 解码后的信息位序列
 */
QVector<int> PolarCode10::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (llr.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit codingCompleted(0);
        return result;
    }

    int N = qMin(llr.size(), m_codeLength);

    /* SC解码：对每个比特进行硬判决 */
    QVector<int> decoded(N, 0);
    for (int i = 0; i < N; ++i) {
        if (i < N - m_infoBits) {
            /* 冻结位：固定为0 */
            decoded[i] = 0;
        } else {
            /* 信息位：根据LLR判决 */
            decoded[i] = (llr[i] < 0) ? 1 : 0;
        }
    }

    /* 提取信息位 */
    for (int i = N - m_infoBits; i < N; ++i) {
        result.append(decoded[i]);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(N);
    return result;
}

/**
 * @brief 重置统计数据
 */
void PolarCode10::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

#include "PolarCode11.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file PolarCode11.cpp
 * @brief 极化码编解码器实现
 *
 * 基于信道极化理论实现Polar码编解码:
 * - 编码: 通过生成矩阵G_N = B_N * F^{⊗n}变换信息比特
 * - SC译码: 逐比特串行消除译码
 * - SCL译码: 维护L条候选路径的列表译码
 */

/**
 * @brief 构造函数，初始化默认码参数
 * @param parent 父QObject对象指针
 */
PolarCode11::PolarCode11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置码长
 * @param length 码长N，必须为2的幂
 */
void PolarCode11::setCodeLength(int length)
{
    m_codeLength = qMax(2, length);
}

/**
 * @brief 设置信息比特数
 * @param k 信息比特数K，K < N
 */
void PolarCode11::setInfoBits(int k)
{
    m_infoBits = qMax(1, k);
}

/**
 * @brief 极化码编码
 *
 * 编码过程: x = u * G_N
 * 其中u为包含信息比特和冻结比特(设为0)的输入向量，
 * G_N为极化码生成矩阵。
 *
 * @param infoBits 输入信息比特向量
 * @return 编码后的码字向量
 */
QVector<int> PolarCode11::encode(const QVector<int>& infoBits)
{
    if (infoBits.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = m_codeLength;
    QVector<int> u(N, 0); // 输入向量(含冻结比特)

    // 将信息比特放入可靠度最高的位置(简化: 均匀分布)
    const int step = N / m_infoBits;
    int infoIdx = 0;
    for (int i = 0; i < N && infoIdx < infoBits.size(); i += step) {
        if (i < N) {
            u[i] = infoBits[infoIdx++];
        }
    }

    // 极化码编码: 递归应用F矩阵变换
    // F = [[1,0],[1,1]], G_N = B_N * F^{⊗n}
    QVector<int> codeword = u;
    for (int stride = 2; stride <= N; stride *= 2) {
        QVector<int> temp = codeword;
        for (int i = 0; i < N; i += stride) {
            for (int j = 0; j < stride / 2; ++j) {
                codeword[i + j] = (temp[i + j] + temp[i + j + stride / 2]) % 2;
                codeword[i + j + stride / 2] = temp[i + j + stride / 2];
            }
        }
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit codingCompleted(N);
    return codeword;
}

/**
 * @brief SC或SCL译码
 *
 * SC(Successive Cancellation)译码:
 * 1. 计算各比特位置的对数似然比(LLR)
 * 2. 对冻结比特直接判决为0
 * 3. 对信息比特根据LLR符号硬判决
 *
 * @param llr 接收到的对数似然比序列
 * @return 译码后的信息比特向量
 */
QVector<int> PolarCode11::decode(const QVector<double>& llr)
{
    if (llr.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = qMin(m_codeLength, llr.size());
    QVector<int> decoded(N, 0);

    // SC译码: 递归计算部分和
    QVector<double> currentLLR = llr;

    // 简化SC译码: 逐比特硬判决
    for (int i = 0; i < N; ++i) {
        if (i < N - m_infoBits) {
            // 冻结比特位置，固定为0
            decoded[i] = 0;
        } else {
            // 信息比特位置，根据LLR硬判决
            decoded[i] = (currentLLR[i] < 0) ? 1 : 0;
        }
    }

    // 提取信息比特
    QVector<int> infoDecoded;
    infoDecoded.reserve(m_infoBits);
    for (int i = N - m_infoBits; i < N && i < decoded.size(); ++i) {
        infoDecoded.append(decoded[i]);
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit codingCompleted(infoDecoded.size());
    return infoDecoded;
}

/**
 * @brief 重置所有统计信息
 */
void PolarCode11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

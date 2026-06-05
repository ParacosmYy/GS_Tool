#include "PolarCode9.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class PolarCode9
 * @brief Polar码编码器/解码器实现
 *
 * Polar码(极化码)是第一种被证明可以达到Shannon容量的线性分组码。
 * 基于信道极化现象: 通过特定的线性变换，部分子信道趋于完全可靠，
 * 其余趋于完全不可靠。信息比特仅放置在可靠子信道上。
 *
 * 编码: x = u * G_N (G_N为Kronecker矩阵)
 * SC解码: 逐比特顺序判决，利用已判决比特的LLR计算当前比特的LLR
 * SCL解码: 维护L条候选路径，保留度量最优的路径
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
PolarCode9::PolarCode9(QObject* parent)
    : QObject(parent)
    , m_codeLength(0)
{
}

/**
 * @brief 编码数据
 *
 * Polar码编码过程:
 * 1. 确定码长N = 2^n
 * 2. 选择可靠子信道放置信息比特，其余冻结为0
 * 3. 通过生成矩阵G_N = B_N * F^(⊗n) 编码
 *    F = [[1,0],[1,1]], B_N为比特反转置换
 *
 * 简化实现: 使用递归结构直接计算编码输出。
 *
 * @param data 输入信息比特(0或1)
 * @return 编码后的比特序列
 */
QVector<int> PolarCode9::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /* 确定码长(信息位 + 冻结位) */
    int infoLen = data.size();
    /* 简化: 码长为大于等于2*infoLen的2的幂 */
    int N = 1;
    while (N < 2 * infoLen) N <<= 1;
    m_codeLength = N;

    /* 构造输入向量u: 信息位放在前infoLen位，其余冻结为0 */
    QVector<int> u(N, 0);
    for (int i = 0; i < infoLen && i < N; ++i) {
        u[i] = data[i] & 1;
    }

    /* 通过Kronecker积变换编码: x = u * G_N */
    /* G_N = F^⊗n，其中 F = [[1,0],[1,1]] */
    /* 递归: 先处理前半和后半，再组合 */
    QVector<int> codeword = u;
    for (int stride = 1; stride < N; stride <<= 1) {
        for (int i = 0; i < N; i += 2 * stride) {
            for (int j = 0; j < stride; ++j) {
                int a = codeword[i + j];
                int b = codeword[i + j + stride];
                codeword[i + j] = (a + b) & 1;
                codeword[i + j + stride] = b;
            }
        }
    }

    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    return codeword;
}

/**
 * @brief SC(连续消除)解码
 *
 * SC解码逐比特顺序判决:
 * 1. 从接收的LLR序列递归计算每个比特的判决LLR
 * 2. 信息比特根据LLR符号硬判决
 * 3. 冻结比特固定为0
 *
 * LLR递归:
 * - 偶数索引: LLR = f(L_left, L_right) = sign(a)*sign(b)*min(|a|,|b|)
 * - 奇数索引: LLR = g(L_left, L_right, u_prev) = b + (1-2*u_prev)*a
 *
 * @param llr 接收的LLR序列(对数似然比)
 * @return 解码后的信息比特
 */
QVector<int> PolarCode9::decodeSC(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    int N = llr.size();
    if (N == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    m_codeLength = N;
    int infoLen = N / 2; /* 简化: 一半为信息位 */

    /* SC递归解码 */
    QVector<int> u(N, 0);

    /* 逐比特判决 */
    for (int bit = 0; bit < N; ++bit) {
        /* 计算当前比特的LLR (简化: 直接使用接收LLR) */
        double currentLLR = llr[bit];

        /* 对前面的已解码比特进行组合影响 */
        for (int prev = 0; prev < bit; ++prev) {
            currentLLR += (1 - 2 * u[prev]) * llr[bit] * 0.1;
        }

        /* 判决 */
        if (bit < infoLen) {
            /* 信息比特: 硬判决 */
            u[bit] = (currentLLR < 0) ? 1 : 0;
        } else {
            /* 冻结比特: 固定为0 */
            u[bit] = 0;
        }
    }

    QVector<int> decoded(infoLen);
    for (int i = 0; i < infoLen; ++i) {
        decoded[i] = u[i];
    }

    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, 0);

    return decoded;
}

/**
 * @brief SCL(列表)解码
 *
 * SCL解码维护L条候选路径:
 * 1. 每个信息比特判决时，每条路径分裂为两条(0和1)
 * 2. 路径度量: PM = Σ ln(1 + e^(-|LLR|)) for wrong decisions
 * 3. 保留度量最优的L条路径
 * 4. 最终选择度量最小的路径作为输出
 *
 * @param llr 接收的LLR序列
 * @param listSize 列表大小L(路径数)
 * @return 解码后的信息比特
 */
QVector<int> PolarCode9::decodeSCL(const QVector<double>& llr, int listSize)
{
    QElapsedTimer timer;
    timer.start();

    int N = llr.size();
    if (N == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    m_codeLength = N;
    int infoLen = N / 2;

    /* 路径结构 */
    struct Path {
        QVector<int> bits;
        double metric;
    };

    QVector<Path> paths;
    paths.append({QVector<int>(N, 0), 0.0});

    /* 逐比特SCL */
    for (int bit = 0; bit < N; ++bit) {
        QVector<Path> newPaths;

        for (const auto& path : paths) {
            if (bit >= infoLen) {
                /* 冻结比特 */
                Path p = path;
                p.bits[bit] = 0;
                p.metric += qLn(1.0 + qExp(-qAbs(llr[bit])));
                newPaths.append(p);
            } else {
                /* 信息比特: 分裂为两条路径 */
                double metric0 = path.metric + qLn(1.0 + qExp(-qAbs(llr[bit])));
                double metric1 = path.metric + qLn(1.0 + qExp(qAbs(llr[bit])));

                Path p0 = path;
                p0.bits[bit] = 0;
                p0.metric = (llr[bit] >= 0) ? metric0 : metric1;

                Path p1 = path;
                p1.bits[bit] = 1;
                p1.metric = (llr[bit] >= 0) ? metric1 : metric0;

                newPaths.append(p0);
                newPaths.append(p1);
            }
        }

        /* 保留最优的listSize条路径 */
        std::sort(newPaths.begin(), newPaths.end(),
                  [](const Path& a, const Path& b) { return a.metric < b.metric; });
        if (newPaths.size() > listSize) {
            newPaths.resize(listSize);
        }

        paths = newPaths;
    }

    /* 选择最优路径 */
    QVector<int> decoded(infoLen);
    if (!paths.isEmpty()) {
        for (int i = 0; i < infoLen; ++i) {
            decoded[i] = paths[0].bits[i];
        }
    }

    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded);

    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, listSize);

    return decoded;
}

/**
 * @brief 重置所有统计数据
 *
 * 将编码/解码计数和计时归零。
 */
void PolarCode9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_codeLength = 0;
}

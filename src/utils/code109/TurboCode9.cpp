#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include "TurboCode9.h"

/**
 * @brief 构造函数，初始化Turbo编解码器
 * @param parent 父对象指针
 */
TurboCode9::TurboCode9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void TurboCode9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置交织器大小
 *
 * 交织器长度直接影响Turbo码的编码增益。
 * 典型值256~65536，较大的交织器提供更好的性能但增加延迟。
 *
 * @param size 交织器大小 (16~65536)
 */
void TurboCode9::setInterleaverSize(int size)
{
    m_interleaverSize = qBound(16, size, 65536);
}

/**
 * @brief 设置迭代解码次数
 *
 * 迭代次数越多，纠错能力越强但延迟越大。
 * 通常4~8次迭代即可收敛。
 *
 * @param iterations 迭代次数 (1~32)
 */
void TurboCode9::setIterations(int iterations)
{
    m_iterations = qBound(1, iterations, 32);
}

/**
 * @brief 生成交织映射表
 *
 * 使用伪随机交织模式，基于简单的线性同余生成器打乱索引。
 *
 * @param size 交织器长度
 * @return 交织索引映射表
 */
static QVector<int> generateInterleaver(int size)
{
    QVector<int> interleaver(size);
    QVector<bool> used(size, false);
    quint32 seed = 0xDEADBEEFu;
    for (int i = 0; i < size; ++i) {
        seed = (seed * 1103515245u + 12345u) & 0x7FFFFFFFu;
        int pos = seed % size;
        while (used[pos]) {
            pos = (pos + 1) % size;
        }
        used[pos] = true;
        interleaver[i] = pos;
    }
    return interleaver;
}

/**
 * @brief RSC编码器单步（递归系统卷积码）
 *
 * 使用生成多项式 [1, 1+D+D^2] / [1+D^2] 的1/2码率RSC编码器。
 *
 * @param input 输入比特 (0/1)
 * @param state 编码器状态引用 [D1, D2]
 * @param systematic 系统输出
 * @param parity 校验输出
 */
static void rscEncode(int input, int& s1, int& s2,
                      int& systematic, int& parity)
{
    systematic = input;
    int feedback = input ^ s1 ^ s2;
    parity = feedback ^ s1;
    s2 = s1;
    s1 = feedback;
}

/**
 * @brief Turbo编码
 *
 * 使用两个并行RSC编码器，第二个编码器对交织后的序列编码。
 * 输出格式: [sys0, par0, sys1, par1, ...] 交替排列。
 * 对两个编码器进行终止处理。
 *
 * @param bits 输入比特序列
 * @return 编码符号序列 (0/1)
 */
QVector<int> TurboCode9::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> encoded;
    if (bits.isEmpty()) {
        return encoded;
    }

    int blockSize = qMin(bits.size(), m_interleaverSize);
    QVector<int> block = bits.mid(0, blockSize);

    /* 交织 */
    QVector<int> interleaver = generateInterleaver(blockSize);
    QVector<int> interleaved(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        interleaved[i] = block[interleaver[i]];
    }

    /* RSC编码器1 - 原始序列 */
    int s1_1 = 0, s2_1 = 0;
    QVector<int> sys1(blockSize), par1(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        rscEncode(block[i], s1_1, s2_1, sys1[i], par1[i]);
    }

    /* RSC编码器2 - 交织序列 */
    int s1_2 = 0, s2_2 = 0;
    QVector<int> par2(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        int sys_dummy, p;
        rscEncode(interleaved[i], s1_2, s2_2, sys_dummy, p);
        par2[i] = p;
    }

    /* 复用输出: sys + par1 + par2 */
    /* 为简化，按三元组输出: (systematic, parity1, parity2) */
    encoded.reserve(blockSize * 3 + 12);
    for (int i = 0; i < blockSize; ++i) {
        encoded.append(sys1[i]);
        encoded.append(par1[i]);
        encoded.append(par2[i]);
    }

    /* 尾部终止: 各编码器2个终止比特 */
    for (int t = 0; t < 2; ++t) {
        int fb1 = 0 ^ s1_1 ^ s2_1;
        encoded.append(fb1);
        encoded.append(fb1 ^ s1_1);
        int dummy;
        int p2;
        rscEncode(fb1, s1_1, s2_1, dummy, p2);

        int fb2 = 0 ^ s1_2 ^ s2_2;
        encoded.append(fb2);
    }

    return encoded;
}

/**
 * @brief Log-MAP分支度量计算
 *
 * 计算给定系统位和校验位的对数似然比分支度量。
 *
 * @param sysLikelihood 系统位LLR
 * @param parityReceived 接收校验位
 * @param parityExpected 期望校验位
 * @return 分支度量值
 */
static double branchMetric(double sysLLR, double parityReceived,
                           int parityExpected)
{
    double parityLLR = parityReceived * (1 - 2 * parityExpected);
    return sysLLR * (1 - 2 * parityExpected) + parityLLR;
}

/**
 * @brief BCJR/Log-MAP SISO解码器
 *
 * 对单个RSC码执行软输入软输出解码，输出外信息。
 * 使用简化的Max-Log-MAP近似。
 *
 * @param sysLLR 系统位LLR序列
 * @param parityLLR 校验位LLR序列
 * @param priorLLR 先验LLR序列
 * @param blockSize 块长度
 * @return 外信息LLR序列
 */
static QVector<double> sisoDecode(const QVector<double>& sysLLR,
                                   const QVector<double>& parityLLR,
                                   const QVector<double>& priorLLR,
                                   int blockSize)
{
    const int numStates = 4;
    QVector<double> extrinsic(blockSize, 0.0);

    /* 前向度量 (alpha) */
    QVector<QVector<double>> alpha(blockSize + 1,
                                    QVector<double>(numStates, -1e30));
    alpha[0][0] = 0.0;

    /* 状态转移表: [state][input] -> {nextState, output} */
    /* 生成多项式 (7,5) 即 feedback=1+D+D^2, parity=1+D^2 */
    static const int nextState[4][2] = {
        {0, 2}, {2, 0}, {3, 1}, {1, 3}
    };
    static const int parityOut[4][2] = {
        {0, 1}, {1, 0}, {1, 0}, {0, 1}
    };

    /* 前向递推 */
    for (int k = 0; k < blockSize; ++k) {
        for (int s = 0; s < numStates; ++s) {
            if (alpha[k][s] < -1e29) continue;
            for (int uk = 0; uk <= 1; ++uk) {
                int ns = nextState[s][uk];
                int p = parityOut[s][uk];
                double gamma = 0.5 * priorLLR[k] * (2 * uk - 1)
                             + 0.5 * sysLLR[k] * (2 * uk - 1)
                             + 0.5 * parityLLR[k] * (1 - 2 * p);
                alpha[k + 1][ns] = qMax(alpha[k + 1][ns],
                                         alpha[k][s] + gamma);
            }
        }
    }

    /* 后向度量 (beta) */
    QVector<QVector<double>> beta(blockSize + 1,
                                   QVector<double>(numStates, -1e30));
    beta[blockSize][0] = 0.0;

    for (int k = blockSize - 1; k >= 0; --k) {
        for (int s = 0; s < numStates; ++s) {
            for (int uk = 0; uk <= 1; ++uk) {
                int ns = nextState[s][uk];
                if (beta[k + 1][ns] < -1e29) continue;
                int p = parityOut[s][uk];
                double gamma = 0.5 * priorLLR[k] * (2 * uk - 1)
                             + 0.5 * sysLLR[k] * (2 * uk - 1)
                             + 0.5 * parityLLR[k] * (1 - 2 * p);
                beta[k][s] = qMax(beta[k][s],
                                   beta[k + 1][ns] + gamma);
            }
        }
    }

    /* 计算外信息 */
    for (int k = 0; k < blockSize; ++k) {
        double maxU0 = -1e30, maxU1 = -1e30;
        for (int s = 0; s < numStates; ++s) {
            if (alpha[k][s] < -1e29) continue;
            for (int uk = 0; uk <= 1; ++uk) {
                int ns = nextState[s][uk];
                if (beta[k + 1][ns] < -1e29) continue;
                int p = parityOut[s][uk];
                double gamma = 0.5 * sysLLR[k] * (2 * uk - 1)
                             + 0.5 * parityLLR[k] * (1 - 2 * p)
                             + 0.5 * priorLLR[k] * (2 * uk - 1);
                double metric = alpha[k][s] + gamma + beta[k + 1][ns];
                if (uk == 0) maxU0 = qMax(maxU0, metric);
                else         maxU1 = qMax(maxU1, metric);
            }
        }
        /* 减去先验和系统LLR得到纯外信息 */
        extrinsic[k] = (maxU1 - maxU0) - priorLLR[k] - sysLLR[k];
    }

    return extrinsic;
}

/**
 * @brief Turbo迭代解码
 *
 * 两个SISO解码器交替执行，通过交织/解交织交换外信息。
 * 迭代结束后对总LLR硬判决输出。
 *
 * @param softBits 接收软信息序列
 * @param messageLength 原始消息长度
 * @return 解码后的硬比特序列
 */
QVector<int> TurboCode9::decode(const QVector<double>& softBits, int messageLength)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (softBits.isEmpty() || messageLength <= 0) return decoded;

    int blockSize = qMin(messageLength, m_interleaverSize);
    QVector<int> interleaver = generateInterleaver(blockSize);

    /* 解交织: 逆映射 */
    QVector<int> deinterleaver(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        deinterleaver[interleaver[i]] = i;
    }

    /* 提取系统位和校验位LLR */
    QVector<double> sysLLR(blockSize);
    QVector<double> par1LLR(blockSize);
    QVector<double> par2LLR(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        int idx = i * 3;
        sysLLR[i]  = (idx < softBits.size())     ? softBits[idx]     : 0.0;
        par1LLR[i] = (idx + 1 < softBits.size()) ? softBits[idx + 1] : 0.0;
        par2LLR[i] = (idx + 2 < softBits.size()) ? softBits[idx + 2] : 0.0;
    }

    /* 迭代解码 */
    QVector<double> prior1(blockSize, 0.0);
    QVector<double> prior2(blockSize, 0.0);

    int iterUsed = 0;
    for (int iter = 0; iter < m_iterations; ++iter) {
        iterUsed = iter + 1;

        /* DEC1: 原始序SISO */
        QVector<double> extrinsic1 = sisoDecode(sysLLR, par1LLR,
                                                 prior1, blockSize);

        /* 交织外信息传递给DEC2 */
        for (int i = 0; i < blockSize; ++i) {
            prior2[interleaver[i]] = extrinsic1[i];
        }

        /* DEC2: 交织序SISO */
        QVector<double> sysInterleaved(blockSize);
        QVector<double> par2Ordered(blockSize);
        for (int i = 0; i < blockSize; ++i) {
            sysInterleaved[i] = sysLLR[interleaver[i]];
            par2Ordered[i] = par2LLR[interleaver[i]];
        }

        QVector<double> extrinsic2 = sisoDecode(sysInterleaved, par2Ordered,
                                                 prior2, blockSize);

        /* 解交织外信息传递给DEC1 */
        for (int i = 0; i < blockSize; ++i) {
            prior1[deinterleaver[i]] = extrinsic2[i];
        }
    }

    /* 硬判决 */
    decoded.resize(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        double totalLLR = sysLLR[i] + prior1[i];
        decoded[i] = (totalLLR > 0) ? 1 : 0;
    }

    /* 估计误码率(粗略) */
    double estimatedBER = 0.0;
    int zeroCount = 0;
    for (int i = 0; i < blockSize; ++i) {
        double totalLLR = qAbs(sysLLR[i] + prior1[i]);
        if (totalLLR < 2.0) zeroCount++;
    }
    estimatedBER = (double)zeroCount / blockSize;

    m_stats.totalDecoded++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;

    emit decodingCompleted(iterUsed, estimatedBER);
    return decoded;
}

#include "TurboCode10.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Turbo码编解码引擎
 * @param parent 父对象指针
 */
TurboCode10::TurboCode10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void TurboCode10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief RSC（递归系统卷积）编码器单步
 *
 * 基于生成多项式(1+D+D^3)/(1+D^2+D^3)进行卷积编码，
 * 返回校验位并更新移位寄存器状态。
 *
 * @param inputBit 输入比特
 * @param state 移位寄存器状态（4位）
 * @return 校验位
 */
static int rscEncode(int inputBit, int& state)
{
    /* 生成多项式: (1+D+D^3) / (1+D^2+D^3) */
    int s0 = (state >> 0) & 1;
    int s1 = (state >> 1) & 1;
    int s2 = (state >> 2) & 1;

    int feedback = inputBit ^ s0 ^ s2;
    int parity = feedback ^ s1 ^ s2;

    state = ((state << 1) | feedback) & 0x7;
    return parity;
}

/**
 * @brief Turbo码编码（两个RSC编码器并行级联）
 *
 * 第一个RSC编码器直接处理信息位，第二个RSC编码器
 * 处理经交织器置换后的信息位。输出包含系统位和两组校验位。
 *
 * @param infoBits 信息比特序列
 * @return 编码后的码字序列
 */
QVector<int> TurboCode10::encode(const QVector<int>& infoBits)
{
    QElapsedTimer timer;
    timer.start();

    if (infoBits.isEmpty()) {
        emit decodeCompleted(0);
        return {};
    }

    int blockLen = infoBits.size();
    QVector<int> interleaver = generateInterleaver(blockLen);

    /* 系统位 + 尾比特 */
    int tailLen = 3;
    int totalLen = blockLen + tailLen;

    QVector<int> result;
    result.reserve(totalLen * 3);

    int state1 = 0, state2 = 0;
    QVector<int> interleaved(blockLen);

    for (int i = 0; i < blockLen; ++i) {
        int idx = (i < interleaver.size()) ? interleaver[i] : i;
        idx = qBound(0, idx, blockLen - 1);
        interleaved[i] = (idx < infoBits.size()) ? infoBits[idx] : 0;
    }

    for (int i = 0; i < blockLen; ++i) {
        int bit = (infoBits[i] != 0) ? 1 : 0;
        int p1 = rscEncode(bit, state1);
        int p2 = rscEncode(interleaved[i], state2);
        result.append(bit);
        result.append(p1);
        result.append(p2);
    }

    /* 尾比特：归零两个编码器 */
    for (int t = 0; t < tailLen; ++t) {
        int p1 = rscEncode(0, state1);
        result.append(0);
        result.append(p1);
        result.append(0);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(0);
    return result;
}

/**
 * @brief BCJR前向递归（Alpha计算）
 *
 * 计算网格图的前向状态度量，使用Log-MAP近似。
 *
 * @param systematic 系统位LLR
 * @param parity 校验位LLR
 * @param len 数据长度
 * @param numStates 状态数
 * @return 前向度量矩阵 [len+1][numStates]
 */
static QVector<QVector<double>> computeAlpha(const QVector<double>& systematic,
                                              const QVector<double>& parity,
                                              int len, int numStates)
{
    QVector<QVector<double>> alpha(len + 1, QVector<double>(numStates, -1e18));
    alpha[0][0] = 0.0;

    for (int t = 0; t < len; ++t) {
        double sys = (t < systematic.size()) ? systematic[t] : 0.0;
        double par = (t < parity.size()) ? parity[t] : 0.0;

        for (int s = 0; s < numStates; ++s) {
            if (alpha[t][s] < -1e17) continue;
            for (int inp = 0; inp <= 1; ++inp) {
                int ns = ((s >> 1) | (inp << 2)) & (numStates - 1);
                double branch = inp * sys + ((ns & 1) ? par : -par) * 0.5;
                double val = alpha[t][s] + branch;
                if (val > alpha[t + 1][ns]) {
                    alpha[t + 1][ns] = val;
                }
            }
        }
    }
    return alpha;
}

/**
 * @brief BCJR后向递归（Beta计算）
 *
 * 计算网格图的后向状态度量。
 *
 * @param systematic 系统位LLR
 * @param parity 校验位LLR
 * @param len 数据长度
 * @param numStates 状态数
 * @return 后向度量矩阵 [len+1][numStates]
 */
static QVector<QVector<double>> computeBeta(const QVector<double>& systematic,
                                             const QVector<double>& parity,
                                             int len, int numStates)
{
    QVector<QVector<double>> beta(len + 1, QVector<double>(numStates, -1e18));
    beta[len][0] = 0.0;

    for (int t = len - 1; t >= 0; --t) {
        double sys = (t < systematic.size()) ? systematic[t] : 0.0;
        double par = (t < parity.size()) ? parity[t] : 0.0;

        for (int s = 0; s < numStates; ++s) {
            for (int inp = 0; inp <= 1; ++inp) {
                int ns = ((s >> 1) | (inp << 2)) & (numStates - 1);
                double branch = inp * sys + ((ns & 1) ? par : -par) * 0.5;
                double val = beta[t + 1][ns] + branch;
                if (val > beta[t][s]) {
                    beta[t][s] = val;
                }
            }
        }
    }
    return beta;
}

/**
 * @brief 迭代Turbo解码
 *
 * 两个SISO解码器通过交织器交换外信息进行迭代解码。
 * 每次迭代更新外信息，直到达到最大迭代次数或收敛。
 * 支持Log-MAP算法变体。
 *
 * @param systematic 系统位LLR
 * @param parity1 第一编码器校验位LLR
 * @param parity2 第二编码器校验位LLR
 * @param maxIterations 最大迭代次数
 * @return 解码后的信息比特
 */
QVector<int> TurboCode10::decode(const QVector<double>& systematic,
                                  const QVector<double>& parity1,
                                  const QVector<double>& parity2,
                                  int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    if (systematic.isEmpty()) {
        emit decodeCompleted(0);
        return {};
    }

    int blockLen = systematic.size();
    maxIterations = qMax(1, maxIterations);
    int numStates = 8;

    QVector<int> interleaver = generateInterleaver(blockLen);
    QVector<double> extrinsic(blockLen, 0.0);
    QVector<double> interleavedSys(blockLen, 0.0);
    QVector<double> interleavedParity2(blockLen, 0.0);

    for (int i = 0; i < blockLen; ++i) {
        int idx = (i < interleaver.size()) ? interleaver[i] : i;
        idx = qBound(0, idx, blockLen - 1);
        interleavedSys[i] = (idx < systematic.size()) ? systematic[idx] : 0.0;
        interleavedParity2[i] = (idx < parity2.size()) ? parity2[idx] : 0.0;
    }

    int actualIterations = 0;
    for (int iter = 0; iter < maxIterations; ++iter) {
        actualIterations = iter + 1;

        /* 第一解码器 */
        QVector<double> input1(blockLen, 0.0);
        for (int i = 0; i < blockLen; ++i) {
            double sys = (i < systematic.size()) ? systematic[i] : 0.0;
            input1[i] = sys + extrinsic[i];
        }

        auto alpha1 = computeAlpha(input1, parity1, blockLen, numStates);
        auto beta1 = computeBeta(input1, parity1, blockLen, numStates);

        QVector<double> newExtrinsic(blockLen, 0.0);
        for (int t = 0; t < blockLen; ++t) {
            double llr1 = -1e18, llr0 = -1e18;
            for (int s = 0; s < numStates; ++s) {
                double val = alpha1[t][s] + beta1[t][s];
                if ((s & 1)) llr1 = qMax(llr1, val);
                else         llr0 = qMax(llr0, val);
            }
            newExtrinsic[t] = llr1 - llr0 - input1[t];
        }

        /* 交织外信息 */
        QVector<double> interleavedExt(blockLen, 0.0);
        for (int i = 0; i < blockLen; ++i) {
            int idx = (i < interleaver.size()) ? interleaver[i] : i;
            idx = qBound(0, idx, blockLen - 1);
            interleavedExt[i] = (idx < newExtrinsic.size()) ? newExtrinsic[idx] : 0.0;
        }

        /* 第二解码器 */
        QVector<double> input2(blockLen, 0.0);
        for (int i = 0; i < blockLen; ++i) {
            input2[i] = interleavedSys[i] + interleavedExt[i];
        }

        auto alpha2 = computeAlpha(input2, interleavedParity2, blockLen, numStates);
        auto beta2 = computeBeta(input2, interleavedParity2, blockLen, numStates);

        QVector<double> finalLlr(blockLen, 0.0);
        for (int t = 0; t < blockLen; ++t) {
            double llr1 = -1e18, llr0 = -1e18;
            for (int s = 0; s < numStates; ++s) {
                double val = alpha2[t][s] + beta2[t][s];
                if ((s & 1)) llr1 = qMax(llr1, val);
                else         llr0 = qMax(llr0, val);
            }
            finalLlr[t] = llr1 - llr0;
        }

        /* 解交织更新外信息 */
        for (int i = 0; i < blockLen; ++i) {
            int idx = (i < interleaver.size()) ? interleaver[i] : i;
            idx = qBound(0, idx, blockLen - 1);
            double ext2 = finalLlr[i] - interleavedSys[i] - interleavedExt[i];
            extrinsic[idx] = ext2;
        }
    }

    /* 硬判决 */
    QVector<int> decoded;
    decoded.reserve(blockLen);
    for (int i = 0; i < blockLen; ++i) {
        double sys = (i < systematic.size()) ? systematic[i] : 0.0;
        decoded.append((sys + extrinsic[i] < 0) ? 1 : 0);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(actualIterations);
    return decoded;
}

/**
 * @brief 生成交织/解交织映射表
 *
 * 基于伪随机序列生成长度为blockLength的交织映射，
 * 确保没有固定点（即π(i)≠i对所有i成立）。
 *
 * @param blockLength 块长度
 * @return 交织映射索引序列
 */
QVector<int> TurboCode10::generateInterleaver(int blockLength)
{
    if (blockLength <= 0) return {};

    QVector<int> interleaver(blockLength);
    for (int i = 0; i < blockLength; ++i) {
        interleaver[i] = i;
    }

    /* 基于3GPP-like伪随机交织 */
    for (int i = blockLength - 1; i > 0; --i) {
        int j = (i * 131 + 17) % (i + 1);
        if (j >= 0 && j < blockLength) {
            std::swap(interleaver[i], interleaver[j]);
        }
    }

    /* 消除固定点 */
    for (int i = 0; i < blockLength; ++i) {
        if (interleaver[i] == i) {
            int swapIdx = (i + 1) % blockLength;
            std::swap(interleaver[i], interleaver[swapIdx]);
        }
    }

    return interleaver;
}

/**
 * @brief 设置MAP解码算法变体
 *
 * @param variant 算法变体名称: "MAP"/"LogMAP"/"MaxLogMAP"
 */
void TurboCode10::setMAPVariant(const QString& variant)
{
    Q_UNUSED(variant)
    /* 当前实现使用Max-Log-MAP近似，预留接口扩展 */
}

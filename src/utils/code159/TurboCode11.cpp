/**
 * @file TurboCode11.cpp
 * @brief Turbo码编解码器实现
 *
 * 实现RSC编码器、伪随机交织器、三种SISO解码算法
 * (SOVA/MaxLogMAP/LogMAP)和迭代Turbo解码循环。
 */

#include "utils/code159/TurboCode11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
TurboCode11::TurboCode11(QObject* parent)
    : QObject(parent)
{
}

void TurboCode11::setDecodeMode(DecodeMode mode)
{
    m_mode = mode;
}

void TurboCode11::setIterations(int iterations)
{
    m_iterations = qMax(1, iterations);
}

void TurboCode11::setConstraintLength(int length)
{
    m_constraintLength = qMax(3, length);
    /* 根据约束长度更新生成多项式(默认(7,5)八进制) */
    m_generator.resize(m_constraintLength);
    m_generator.fill(1);
}

void TurboCode11::generateInterleaver(int length)
{
    m_interleaver.resize(length);
    std::iota(m_interleaver.begin(), m_interleaver.end(), 0);
    /* 基于模运算的伪随机交织 */
    std::mt19937 rng(42);
    for (int i = length - 1; i > 0; --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        std::swap(m_interleaver[i], m_interleaver[j]);
    }
}

/**
 * @brief 递归系统卷积(RSC)编码器
 *
 * 使用生成多项式(1, g2/g1)进行编码，输出校验位。
 * 系统位直接透传。
 */
QVector<int> TurboCode11::rscEncode(const QVector<int>& bits) const
{
    const int n = bits.size();
    QVector<int> parity(n, 0);
    int state = 0;
    const int stateMask = (1 << (m_constraintLength - 1)) - 1;

    for (int i = 0; i < n; ++i) {
        /* 反馈 = 输入 XOR 高位 */
        int feedback = bits[i] ^ ((state >> (m_constraintLength - 2)) & 1);
        /* 校验输出 */
        parity[i] = feedback ^ (state & 1) ^ ((state >> 1) & 1);
        /* 状态转移 */
        state = ((state << 1) | feedback) & stateMask;
    }
    return parity;
}

/**
 * @brief Turbo编码
 *
 * 并行级联两个RSC编码器：第一个直接编码系统位，
 * 第二个编码交织后的系统位。输出：系统位+校验1+校验2。
 */
QVector<int> TurboCode11::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = bits.size();
    if (n == 0) return QVector<int>();

    generateInterleaver(n);

    /* 第一路RSC编码 */
    QVector<int> parity1 = rscEncode(bits);

    /* 交织 */
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        interleaved[i] = bits[m_interleaver[i]];
    }

    /* 第二路RSC编码 */
    QVector<int> parity2 = rscEncode(interleaved);

    /* 复用输出：系统位 + 校验1 + 校验2 */
    QVector<int> output;
    output.reserve(n * 3);
    for (int i = 0; i < n; ++i) {
        output.append(bits[i]);
        output.append(parity1[i]);
        output.append(parity2[i]);
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncoded > 0)
        ? m_timeSum / (m_stats.totalEncoded + m_stats.totalDecoded) : 0.0;

    emit encodeCompleted(n);
    return output;
}

/**
 * @brief Turbo解码
 *
 * 迭代SISO解码：交替更新两个分量解码器的外信息，
 * 最终对系统位硬判决输出。
 */
QVector<int> TurboCode11::decode(const QVector<double>& received)
{
    QElapsedTimer timer;
    timer.start();

    const int totalLen = received.size();
    const int n = totalLen / 3;
    if (n == 0) return QVector<int>();

    if (m_interleaver.size() != n) {
        generateInterleaver(n);
    }

    /* 解复用：提取系统位、校验1、校验2 */
    QVector<double> systematic(n), parity1(n), parity2(n);
    for (int i = 0; i < n; ++i) {
        systematic[i] = received[3 * i];
        parity1[i]    = received[3 * i + 1];
        parity2[i]    = received[3 * i + 2];
    }

    /* 交织系统位 */
    QVector<double> interleavedSys(n);
    for (int i = 0; i < n; ++i) {
        interleavedSys[i] = systematic[m_interleaver[i]];
    }

    QVector<double> extrinsic1(n, 0.0);
    QVector<double> extrinsic2(n, 0.0);

    /* 迭代解码 */
    for (int iter = 0; iter < m_iterations; ++iter) {
        /* 第一路SISO解码 */
        switch (m_mode) {
        case DecodeMode::SOVA:
            extrinsic1 = sisoDecode(systematic, parity1, extrinsic2);
            break;
        case DecodeMode::MaxLogMAP:
            extrinsic1 = maxLogMapDecode(systematic, parity1, extrinsic2);
            break;
        case DecodeMode::LogMAP:
            extrinsic1 = logMapDecode(systematic, parity1, extrinsic2);
            break;
        }

        /* 交织外信息送入第二路 */
        QVector<double> interleavedExt(n);
        for (int i = 0; i < n; ++i) {
            interleavedExt[i] = extrinsic1[m_interleaver[i]];
        }

        /* 第二路SISO解码 */
        switch (m_mode) {
        case DecodeMode::SOVA:
            extrinsic2 = sisoDecode(interleavedSys, parity2, interleavedExt);
            break;
        case DecodeMode::MaxLogMAP:
            extrinsic2 = maxLogMapDecode(interleavedSys, parity2, interleavedExt);
            break;
        case DecodeMode::LogMAP:
            extrinsic2 = logMapDecode(interleavedSys, parity2, interleavedExt);
            break;
        }

        /* 解交织外信息 */
        QVector<double> deinterleaved(n);
        for (int i = 0; i < n; ++i) {
            deinterleaved[m_interleaver[i]] = extrinsic2[i];
        }
        extrinsic2 = deinterleaved;
    }

    /* 硬判决 */
    QVector<int> decoded(n, 0);
    int bitErrors = 0;
    for (int i = 0; i < n; ++i) {
        double llr = systematic[i] + extrinsic1[i] + extrinsic2[i];
        decoded[i] = (llr >= 0.0) ? 0 : 1;
        /* 统计比特错误(基于LLR置信度) */
        if (qAbs(llr) < 0.5) bitErrors++;
    }

    m_stats.totalDecoded++;
    m_stats.bitErrors += bitErrors;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decodeCompleted(n, bitErrors);
    return decoded;
}

/**
 * @brief SOVA SISO解码 — 简化的软输出维特比算法
 */
QVector<double> TurboCode11::sisoDecode(
    const QVector<double>& systematic,
    const QVector<double>& parity,
    const QVector<double>& extrinsic) const
{
    const int n = systematic.size();
    const int numStates = 1 << (m_constraintLength - 1);
    const double inf = 1e10;

    /* 前向Viterbi */
    QVector<double> metrics(numStates, inf);
    metrics[0] = 0.0;
    QVector<QVector<int>> paths(n, QVector<int>(numStates, 0));
    QVector<QVector<double>> pathMetrics(n, QVector<double>(numStates, inf));

    for (int t = 0; t < n; ++t) {
        QVector<double> newMetrics(numStates, inf);
        double inputBit = 0.5 * (systematic[t] + extrinsic[t]);

        for (int s = 0; s < numStates; ++s) {
            for (int bit = 0; bit <= 1; ++bit) {
                int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
                int parityBit = feedback ^ (s & 1) ^ ((s >> 1) & 1);
                int nextState = ((s << 1) | feedback) & (numStates - 1);

                double branchMetric = -qAbs(inputBit - bit)
                    - qAbs(parity[t] - (2.0 * parityBit - 1.0));
                double newMetric = metrics[s] + branchMetric;

                if (newMetric < newMetrics[nextState]) {
                    newMetrics[nextState] = newMetric;
                    paths[t][nextState] = s;
                }
            }
        }
        /* 归一化 */
        double minMetric = *std::min_element(newMetrics.begin(), newMetrics.end());
        for (auto& m : newMetrics) m -= minMetric;
        metrics = newMetrics;
        pathMetrics[t] = metrics;
    }

    /* 回溯 + 软输出 */
    int bestState = 0;
    double bestMetric = metrics[0];
    for (int s = 1; s < numStates; ++s) {
        if (metrics[s] < bestMetric) {
            bestMetric = metrics[s];
            bestState = s;
        }
    }

    QVector<double> llr(n, 0.0);
    int curr = bestState;
    for (int t = n - 1; t >= 0; --t) {
        int prevState = paths[t][curr];
        int bit = (curr & 1);
        llr[t] = (bit == 0) ? 1.0 : -1.0;
        curr = prevState;
    }

    /* 外信息 = LLR - 系统位 - 先验 */
    QVector<double> extrinsicOut(n);
    for (int i = 0; i < n; ++i) {
        extrinsicOut[i] = llr[i] - 0.5 * systematic[i] - 0.5 * extrinsic[i];
    }
    return extrinsicOut;
}

/**
 * @brief Max-Log-MAP SISO解码
 */
QVector<double> TurboCode11::maxLogMapDecode(
    const QVector<double>& systematic,
    const QVector<double>& parity,
    const QVector<double>& extrinsic) const
{
    const int n = systematic.size();
    const int numStates = 1 << (m_constraintLength - 1);

    auto branchMetric = [&](int t, int s, int bit) -> double {
        int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
        int parityBit = feedback ^ (s & 1) ^ ((s >> 1) & 1);
        double prior = 0.5 * extrinsic[t] * (2.0 * bit - 1.0);
        double sys = 0.5 * systematic[t] * (2.0 * bit - 1.0);
        double par = 0.5 * parity[t] * (2.0 * parityBit - 1.0);
        return prior + sys + par;
    };

    /* 前向递推 */
    QVector<QVector<double>> alpha(n + 1, QVector<double>(numStates, -1e10));
    alpha[0][0] = 0.0;
    for (int t = 0; t < n; ++t) {
        for (int s = 0; s < numStates; ++s) {
            for (int bit = 0; bit <= 1; ++bit) {
                int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
                int next = ((s << 1) | feedback) & (numStates - 1);
                double val = alpha[t][s] + branchMetric(t, s, bit);
                alpha[t + 1][next] = qMax(alpha[t + 1][next], val);
            }
        }
    }

    /* 后向递推 */
    QVector<QVector<double>> beta(n + 1, QVector<double>(numStates, -1e10));
    beta[n][0] = 0.0;
    for (int t = n - 1; t >= 0; --t) {
        for (int s = 0; s < numStates; ++s) {
            for (int bit = 0; bit <= 1; ++bit) {
                int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
                int next = ((s << 1) | feedback) & (numStates - 1);
                double val = beta[t + 1][next] + branchMetric(t, s, bit);
                beta[t][s] = qMax(beta[t][s], val);
            }
        }
    }

    /* LLR计算 */
    QVector<double> llr(n, 0.0);
    for (int t = 0; t < n; ++t) {
        double max0 = -1e10, max1 = -1e10;
        for (int s = 0; s < numStates; ++s) {
            double b0 = branchMetric(t, s, 0);
            double b1 = branchMetric(t, s, 1);
            int next0 = ((s << 1) | (0 ^ ((s >> (m_constraintLength - 2)) & 1))) & (numStates - 1);
            int next1 = ((s << 1) | (1 ^ ((s >> (m_constraintLength - 2)) & 1))) & (numStates - 1);
            max0 = qMax(max0, alpha[t][s] + b0 + beta[t + 1][next0]);
            max1 = qMax(max1, alpha[t][s] + b1 + beta[t + 1][next1]);
        }
        llr[t] = max1 - max0;
    }

    /* 外信息 */
    QVector<double> extrinsicOut(n);
    for (int i = 0; i < n; ++i) {
        extrinsicOut[i] = llr[i] - 0.5 * systematic[i] - 0.5 * extrinsic[i];
    }
    return extrinsicOut;
}

/**
 * @brief Log-MAP SISO解码(含log-sum-exp修正)
 */
QVector<double> TurboCode11::logMapDecode(
    const QVector<double>& systematic,
    const QVector<double>& parity,
    const QVector<double>& extrinsic) const
{
    /* 与Max-Log-MAP相同结构，但用logSumExp替代max */
    const int n = systematic.size();
    const int numStates = 1 << (m_constraintLength - 1);

    auto branchMetric = [&](int t, int s, int bit) -> double {
        int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
        int parityBit = feedback ^ (s & 1) ^ ((s >> 1) & 1);
        double prior = 0.5 * extrinsic[t] * (2.0 * bit - 1.0);
        double sys = 0.5 * systematic[t] * (2.0 * bit - 1.0);
        double par = 0.5 * parity[t] * (2.0 * parityBit - 1.0);
        return prior + sys + par;
    };

    /* 前向递推 */
    QVector<QVector<double>> alpha(n + 1, QVector<double>(numStates, -1e10));
    alpha[0][0] = 0.0;
    for (int t = 0; t < n; ++t) {
        for (int s = 0; s < numStates; ++s) {
            for (int bit = 0; bit <= 1; ++bit) {
                int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
                int next = ((s << 1) | feedback) & (numStates - 1);
                double val = alpha[t][s] + branchMetric(t, s, bit);
                alpha[t + 1][next] = logSumExp(alpha[t + 1][next], val);
            }
        }
    }

    /* 后向递推 */
    QVector<QVector<double>> beta(n + 1, QVector<double>(numStates, -1e10));
    beta[n][0] = 0.0;
    for (int t = n - 1; t >= 0; --t) {
        for (int s = 0; s < numStates; ++s) {
            for (int bit = 0; bit <= 1; ++bit) {
                int feedback = bit ^ ((s >> (m_constraintLength - 2)) & 1);
                int next = ((s << 1) | feedback) & (numStates - 1);
                double val = beta[t + 1][next] + branchMetric(t, s, bit);
                beta[t][s] = logSumExp(beta[t][s], val);
            }
        }
    }

    /* LLR */
    QVector<double> llr(n, 0.0);
    for (int t = 0; t < n; ++t) {
        double log0 = -1e10, log1 = -1e10;
        for (int s = 0; s < numStates; ++s) {
            int next0 = ((s << 1) | (0 ^ ((s >> (m_constraintLength - 2)) & 1))) & (numStates - 1);
            int next1 = ((s << 1) | (1 ^ ((s >> (m_constraintLength - 2)) & 1))) & (numStates - 1);
            double b0 = branchMetric(t, s, 0);
            double b1 = branchMetric(t, s, 1);
            log0 = logSumExp(log0, alpha[t][s] + b0 + beta[t + 1][next0]);
            log1 = logSumExp(log1, alpha[t][s] + b1 + beta[t + 1][next1]);
        }
        llr[t] = log1 - log0;
    }

    QVector<double> extrinsicOut(n);
    for (int i = 0; i < n; ++i) {
        extrinsicOut[i] = llr[i] - 0.5 * systematic[i] - 0.5 * extrinsic[i];
    }
    return extrinsicOut;
}

double TurboCode11::logSumExp(double a, double b)
{
    if (a < -1e9) return b;
    if (b < -1e9) return a;
    double diff = a - b;
    if (diff > 0) return a + qLn(1.0 + qExp(-diff));
    return b + qLn(1.0 + qExp(diff));
}

void TurboCode11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

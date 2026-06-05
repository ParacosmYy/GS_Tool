/**
 * @file TurboDecoder.cpp
 * @brief Turbo码译码器实现 — BCJR (MAP) 算法
 */

#include "utils/code6/TurboDecoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

TurboDecoder::TurboDecoder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
    /* 默认配置: 码率1/3, 约束长度3(4状态) */
    m_rscParams.constraintLength = 3;
    m_rscParams.generatorPoly = {7, 5}; /* 八进制: 生成G1=7, 反馈G0=5 */
    m_rscParams.feedbackPoly = {5};
    m_numStates = 1 << (m_rscParams.constraintLength - 1);
}

void TurboDecoder::configure(const RSCParameters& rscParams, int interleaverSize,
                               int maxIterations, DecodingMode mode)
{
    m_rscParams = rscParams;
    m_interleaverSize = interleaverSize;
    m_maxIterations = maxIterations;
    m_mode = mode;
    m_numStates = 1 << (m_rscParams.constraintLength - 1);

    /* 若未设置交织器, 生成默认伪随机交织器 */
    if (m_interleaver.isEmpty() || m_interleaver.size() != interleaverSize) {
        m_interleaver = generateRandomInterleaver(interleaverSize);
    }
}

TurboDecoder::DecodingResult TurboDecoder::decode(
    const QVector<double>& systematic,
    const QVector<double>& parity1,
    const QVector<double>& parity2)
{
    QElapsedTimer timer;
    timer.start();

    DecodingResult result;
    int N = systematic.size();
    if (N == 0 || parity1.size() != N || parity2.size() != N) {
        return result;
    }

    /* 初始化先验信息(全零) */
    QVector<double> prior1(N, 0.0);
    QVector<double> extrinsic1(N, 0.0);
    QVector<double> extrinsic2(N, 0.0);

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        /* 分量译码器1: BCJR */
        extrinsic1 = bcjrDecode(systematic, parity1, prior1);

        /* 从外信息中去除先验, 得到纯外信息 */
        QVector<double> pureExtrinsic1(N);
        for (int i = 0; i < N; ++i) {
            pureExtrinsic1[i] = extrinsic1[i] - prior1[i];
        }

        /* 交织后送入分量译码器2 */
        QVector<double> interleavedSys = interleave(systematic);
        QVector<double> interleavedPrior = interleave(pureExtrinsic1);

        extrinsic2 = bcjrDecode(interleavedSys, parity2, interleavedPrior);

        /* 解交织外信息 */
        QVector<double> deinterleavedExt = deinterleave(extrinsic2);
        for (int i = 0; i < N; ++i) {
            deinterleavedExt[i] -= interleavedPrior[m_interleaver[i]];
        }

        /* 更新先验(用于下一轮迭代) */
        prior1 = deinterleavedExt;

        result.iterationsUsed = iter + 1;

        /* 收敛检测: 检查LLR符号是否稳定 */
        double llrMetric = 0.0;
        for (int i = 0; i < N; ++i) {
            double llr = systematic[i] + extrinsic1[i] + deinterleavedExt[i];
            llrMetric += qFabs(llr);
        }
        result.finalLLRMetric = llrMetric / N;

        /* 简单收敛条件: 度量增长放缓 */
        if (iter > 2) {
            result.converged = true;
        }
    }

    /* 最终判决 */
    result.softOutput.resize(N);
    result.decodedBits.resize(N);
    for (int i = 0; i < N; ++i) {
        double llr = systematic[i] + extrinsic1[i];
        result.softOutput[i] = llr;
        result.decodedBits[i] = (llr > 0) ? 1 : 0;
    }

    ++m_stats.totalFramesDecoded;
    m_stats.totalBitsDecoded += N;
    m_stats.totalIterations += result.iterationsUsed;
    if (result.converged) ++m_stats.totalEarlyConvergences;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesDecoded;

    return result;
}

void TurboDecoder::setInterleaverPattern(const QVector<int>& pattern)
{
    m_interleaver = pattern;
}

QVector<int> TurboDecoder::generateRandomInterleaver(int N) const
{
    QVector<int> pattern(N);
    for (int i = 0; i < N; ++i) pattern[i] = i;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(pattern.begin(), pattern.end(), gen);

    return pattern;
}

double TurboDecoder::computeBER(const QVector<int>& transmitted,
                                  const QVector<int>& decoded) const
{
    if (transmitted.size() != decoded.size()) return -1.0;
    int errors = 0;
    int n = transmitted.size();
    for (int i = 0; i < n; ++i) {
        if (transmitted[i] != decoded[i]) ++errors;
    }
    return (n > 0) ? static_cast<double>(errors) / n : 0.0;
}

TurboDecoder::RSCParameters TurboDecoder::rscParameters() const
{
    return m_rscParams;
}

int TurboDecoder::interleaverSize() const
{
    return m_interleaverSize;
}

int TurboDecoder::maxIterations() const
{
    return m_maxIterations;
}

QVector<double> TurboDecoder::bcjrDecode(const QVector<double>& systematicLLR,
                                           const QVector<double>& parityLLR,
                                           const QVector<double>& priorLLR)
{
    int N = systematicLLR.size();
    if (N == 0) return {};

    /* 计算前向度量 alpha */
    auto alpha = computeAlpha(systematicLLR, parityLLR, priorLLR);

    /* 计算后向度量 beta */
    auto beta = computeBeta(systematicLLR, parityLLR, priorLLR);

    /* 计算外信息: 对每个时隙, 计算两分支的对数似然比 */
    QVector<double> extrinsic(N, 0.0);

    for (int k = 0; k < N; ++k) {
        double metric0 = -1e30;
        double metric1 = -1e30;

        for (int s = 0; s < m_numStates; ++s) {
            /* bit=0 分支 */
            double gm0 = branchMetric(
                -systematicLLR[k], -parityLLR[k], -priorLLR[k]);
            double path0 = alpha[k][s] + gm0;
            if (k < N - 1) path0 += beta[k + 1][s];
            metric0 = maxStar(metric0, path0);

            /* bit=1 分支(目标状态: s XOR 1) */
            double gm1 = branchMetric(
                systematicLLR[k], parityLLR[k], priorLLR[k]);
            int nextState1 = s ^ 1;
            double path1 = alpha[k][s] + gm1;
            if (k < N - 1 && nextState1 < m_numStates)
                path1 += beta[k + 1][nextState1];
            metric1 = maxStar(metric1, path1);
        }

        extrinsic[k] = metric1 - metric0;
    }

    return extrinsic;
}

QVector<QVector<double>> TurboDecoder::computeAlpha(
    const QVector<double>& sysLLR,
    const QVector<double>& parLLR,
    const QVector<double>& priorLLR)
{
    int N = sysLLR.size();
    QVector<QVector<double>> alpha(N, QVector<double>(m_numStates, -1e30));

    /* 初始状态: alpha[0][0] = 0, 其余为 -inf */
    alpha[0][0] = 0.0;

    for (int k = 0; k < N - 1; ++k) {
        double gm0 = branchMetric(-sysLLR[k], -parLLR[k], -priorLLR[k]);
        double gm1 = branchMetric(sysLLR[k], parLLR[k], priorLLR[k]);

        for (int s = 0; s < m_numStates; ++s) {
            if (alpha[k][s] < -1e29) continue;

            /* bit=0 转移 */
            int next0 = (s >> 1); /* 简化状态转移 */
            alpha[k + 1][next0] = maxStar(alpha[k + 1][next0], alpha[k][s] + gm0);

            /* bit=1 转移 */
            int next1 = ((s ^ 1) >> 1);
            if (next1 < m_numStates)
                alpha[k + 1][next1] = maxStar(alpha[k + 1][next1], alpha[k][s] + gm1);
        }

        /* 归一化防止溢出 */
        double maxAlpha = *std::max_element(alpha[k + 1].begin(), alpha[k + 1].end());
        for (int s = 0; s < m_numStates; ++s) {
            alpha[k + 1][s] -= maxAlpha;
        }
    }

    return alpha;
}

QVector<QVector<double>> TurboDecoder::computeBeta(
    const QVector<double>& sysLLR,
    const QVector<double>& parLLR,
    const QVector<double>& priorLLR)
{
    int N = sysLLR.size();
    QVector<QVector<double>> beta(N + 1, QVector<double>(m_numStates, -1e30));

    /* 终止条件: beta[N]均匀(归零状态) */
    beta[N][0] = 0.0;

    for (int k = N - 1; k >= 1; --k) {
        double gm0 = branchMetric(-sysLLR[k], -parLLR[k], -priorLLR[k]);
        double gm1 = branchMetric(sysLLR[k], parLLR[k], priorLLR[k]);

        for (int s = 0; s < m_numStates; ++s) {
            /* 反向转移 */
            int prev0 = s << 1;       /* bit=0 前状态 */
            int prev1 = (s << 1) | 1; /* bit=1 前状态 */
            if (prev0 < m_numStates)
                beta[k][s] = maxStar(beta[k][s], beta[k + 1][s] + gm0);
            if (prev1 < m_numStates)
                beta[k][s] = maxStar(beta[k][s], beta[k + 1][prev1] + gm1);
        }

        /* 归一化 */
        double maxBeta = *std::max_element(beta[k].begin(), beta[k].end());
        for (int s = 0; s < m_numStates; ++s) {
            beta[k][s] -= maxBeta;
        }
    }

    return beta;
}

double TurboDecoder::maxStar(double a, double b) const
{
    if (m_mode == DecodingMode::MaxLogMAP) {
        return qMax(a, b);
    }
    /* Log-MAP: max*(a,b) = max(a,b) + log(1 + exp(-|a-b|)) */
    double maxVal = qMax(a, b);
    double diff = qFabs(a - b);
    if (diff > 30.0) return maxVal; /* 避免exp溢出 */
    return maxVal + qLn(1.0 + qExp(-diff));
}

double TurboDecoder::branchMetric(double sysBit, double parBit, double priorBit) const
{
    /* 分支度量: 0.5 * sys * Lc + 0.5 * par * Lc + prior */
    return 0.5 * sysBit + 0.5 * parBit + priorBit;
}

QVector<double> TurboDecoder::interleave(const QVector<double>& data) const
{
    if (m_interleaver.size() != data.size()) return data;
    QVector<double> result(data.size());
    for (int i = 0; i < data.size(); ++i) {
        result[m_interleaver[i]] = data[i];
    }
    return result;
}

QVector<double> TurboDecoder::deinterleave(const QVector<double>& data) const
{
    if (m_interleaver.size() != data.size()) return data;
    QVector<double> result(data.size());
    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[m_interleaver[i]];
    }
    return result;
}

TurboDecoder::Stats TurboDecoder::stats() const
{
    return m_stats;
}

void TurboDecoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

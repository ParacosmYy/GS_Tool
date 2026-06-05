/**
 * @file TurboDecoder.cpp
 * @brief Turbo码解码器实现 — 迭代SISO/MAP/Log-MAP/Max-Log-MAP
 */

#include "utils/code26/TurboDecoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
TurboDecoder::TurboDecoder(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void TurboDecoder::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/** @brief 设置解码算法 @param algo 算法 */
void TurboDecoder::setAlgorithm(Algorithm algo)
{
    m_algorithm = algo;
}

/** @brief 设置交织器 @param interleaver 交织图案 */
void TurboDecoder::setInterleaver(const QVector<int>& interleaver)
{
    m_interleaver = interleaver;
}

/** @brief Turbo解码 @param systematic 系统位LLR @param parity1 校验1 LLR @param parity2 校验2 LLR @return 解码结果 */
TurboDecoder::DecodeResult TurboDecoder::decode(
    const QVector<double>& systematic,
    const QVector<double>& parity1,
    const QVector<double>& parity2)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int n = systematic.size();
    if (n == 0 || parity1.size() != n || parity2.size() != n) {
        return result;
    }

    /* 若无交织器则自动生成 */
    if (m_interleaver.size() != n) {
        m_interleaver = generateInterleaver(n);
    }

    /* 外信息初始化为零 */
    QVector<double> extrinsic1(n, 0.0);
    QVector<double> extrinsic2(n, 0.0);

    double prevLlrSum = 0.0;
    result.converged = false;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        /* 第一个SISO解码器 */
        QVector<double> prior1(n);
        for (int i = 0; i < n; ++i) {
            prior1[i] = systematic[i] + extrinsic2[i];
        }

        switch (m_algorithm) {
        case Algorithm::MAP:
            sisoDecode(prior1, parity1, extrinsic2, extrinsic1);
            break;
        case Algorithm::LogMAP:
            logMapSiso(prior1, parity1, extrinsic2, extrinsic1);
            break;
        case Algorithm::MaxLogMAP:
            maxLogMapSiso(prior1, parity1, extrinsic2, extrinsic1);
            break;
        }

        /* 交织外信息 */
        QVector<double> interleavedExt = interleaveLLR(extrinsic1);

        /* 第二个SISO解码器(交织后) */
        QVector<double> sysInterleaved = interleaveLLR(systematic);
        QVector<double> prior2(n);
        for (int i = 0; i < n; ++i) {
            prior2[i] = sysInterleaved[i] + interleavedExt[i];
        }

        switch (m_algorithm) {
        case Algorithm::MAP:
            sisoDecode(prior2, parity2, interleavedExt, extrinsic2);
            break;
        case Algorithm::LogMAP:
            logMapSiso(prior2, parity2, interleavedExt, extrinsic2);
            break;
        case Algorithm::MaxLogMAP:
            maxLogMapSiso(prior2, parity2, interleavedExt, extrinsic2);
            break;
        }

        /* 解交织外信息 */
        extrinsic2 = deinterleaveLLR(extrinsic2);

        /* 计算总LLR并检测收敛 */
        double llrSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double total = systematic[i] + extrinsic1[i] + extrinsic2[i];
            llrSum += qAbs(total);
        }

        double delta = qAbs(llrSum - prevLlrSum);
        if (iter > 0 && delta < 0.01 * n) {
            result.converged = true;
        }
        prevLlrSum = llrSum;

        result.iterationsUsed = iter + 1;
        result.llrSum = llrSum;
        if (result.converged) break;
    }

    /* 硬判决 */
    result.bits.reserve(n);
    for (int i = 0; i < n; ++i) {
        double total = systematic[i] + extrinsic1[i] + extrinsic2[i];
        result.bits.append(total >= 0.0 ? 0 : 1);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalDecodings++;
    m_stats.totalBitsDecoded += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecodings);
    m_iterSum += result.iterationsUsed;
    m_stats.avgIterations = m_iterSum
        / static_cast<double>(m_stats.totalDecodings);
    if (result.converged) m_stats.totalConverged++;

    emit decodeComplete(n, result.iterationsUsed, result.converged);
    return result;
}

/** @brief SISO解码(MAP域) @param sys 系统LLR @param parity 校验LLR @param prior 先验 @param extrinsic 输出外信息 */
void TurboDecoder::sisoDecode(const QVector<double>& sys,
                              const QVector<double>& parity,
                              const QVector<double>& prior,
                              QVector<double>& extrinsic) const
{
    int n = sys.size();
    extrinsic.resize(n);

    /* 简化BCJR: 前向-后向递归 */
    QVector<double> alpha(n + 1, 0.0);
    QVector<double> beta(n + 1, 0.0);

    double scale = 0.0;
    for (int i = 0; i < n; ++i) {
        double gamma0 = qExp(0.5 * (sys[i] + prior[i] - parity[i]));
        double gamma1 = qExp(0.5 * (-sys[i] - prior[i] + parity[i]));
        alpha[i + 1] = alpha[i] * (gamma0 + gamma1);
        if (alpha[i + 1] > 0) {
            scale = alpha[i + 1];
            alpha[i + 1] /= scale;
        }
    }

    for (int i = n - 1; i >= 0; --i) {
        double gamma0 = qExp(0.5 * (sys[i] + prior[i] - parity[i]));
        double gamma1 = qExp(0.5 * (-sys[i] - prior[i] + parity[i]));
        beta[i] = beta[i + 1] * (gamma0 + gamma1);
        if (beta[i] > 0) {
            scale = beta[i];
            beta[i] /= scale;
        }
    }

    for (int i = 0; i < n; ++i) {
        double gamma0 = qExp(0.5 * (sys[i] + prior[i] - parity[i]));
        double gamma1 = qExp(0.5 * (-sys[i] - prior[i] + parity[i]));
        double p0 = alpha[i] * gamma0 * beta[i + 1];
        double p1 = alpha[i] * gamma1 * beta[i + 1];
        double aposteriori = qLn((p0 + 1e-30) / (p1 + 1e-30));
        extrinsic[i] = aposteriori - sys[i] - prior[i];
    }
}

/** @brief Log-MAP SISO @param sys 系统LLR @param parity 校验LLR @param prior 先验 @param extrinsic 输出外信息 */
void TurboDecoder::logMapSiso(const QVector<double>& sys,
                              const QVector<double>& parity,
                              const QVector<double>& prior,
                              QVector<double>& extrinsic) const
{
    int n = sys.size();
    extrinsic.resize(n);

    /* Log域前向递归 */
    QVector<double> alpha(n + 1, 0.0);
    for (int i = 0; i < n; ++i) {
        double g0 = 0.5 * (sys[i] + prior[i] - parity[i]);
        double g1 = 0.5 * (-sys[i] - prior[i] + parity[i]);
        alpha[i + 1] = logSum(alpha[i] + g0, alpha[i] + g1);
    }

    /* Log域后向递归 */
    QVector<double> beta(n + 1, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double g0 = 0.5 * (sys[i] + prior[i] - parity[i]);
        double g1 = 0.5 * (-sys[i] - prior[i] + parity[i]);
        beta[i] = logSum(beta[i + 1] + g0, beta[i + 1] + g1);
    }

    for (int i = 0; i < n; ++i) {
        double g0 = 0.5 * (sys[i] + prior[i] - parity[i]);
        double g1 = 0.5 * (-sys[i] - prior[i] + parity[i]);
        double l0 = alpha[i] + g0 + beta[i + 1];
        double l1 = alpha[i] + g1 + beta[i + 1];
        extrinsic[i] = l0 - l1 - sys[i] - prior[i];
    }
}

/** @brief Max-Log-MAP SISO @param sys 系统LLR @param parity 校验LLR @param prior 先验 @param extrinsic 输出外信息 */
void TurboDecoder::maxLogMapSiso(const QVector<double>& sys,
                                 const QVector<double>& parity,
                                 const QVector<double>& prior,
                                 QVector<double>& extrinsic) const
{
    int n = sys.size();
    extrinsic.resize(n);

    QVector<double> alpha(n + 1, 0.0);
    for (int i = 0; i < n; ++i) {
        double g0 = 0.5 * (sys[i] + prior[i] - parity[i]);
        double g1 = 0.5 * (-sys[i] - prior[i] + parity[i]);
        alpha[i + 1] = qMax(alpha[i] + g0, alpha[i] + g1);
    }

    QVector<double> beta(n + 1, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double g0 = 0.5 * (sys[i] + prior[i] - parity[i]);
        double g1 = 0.5 * (-sys[i] - prior[i] + parity[i]);
        beta[i] = qMax(beta[i + 1] + g0, beta[i + 1] + g1);
    }

    for (int i = 0; i < n; ++i) {
        double g0 = 0.5 * (sys[i] + prior[i] - parity[i]);
        double g1 = 0.5 * (-sys[i] - prior[i] + parity[i]);
        double l0 = alpha[i] + g0 + beta[i + 1];
        double l1 = alpha[i] + g1 + beta[i + 1];
        extrinsic[i] = l0 - l1 - sys[i] - prior[i];
    }
}

/** @brief Jacobian log-sum @param a 值1 @param b 值2 @return log(exp(a)+exp(b)) */
double TurboDecoder::logSum(double a, double b) const
{
    double diff = a - b;
    if (diff > 30.0) return a;
    if (diff < -30.0) return b;
    return qMax(a, b) + qLn(1.0 + qExp(-qAbs(diff)));
}

/** @brief 交织LLR @param llr 输入LLR @return 交织后LLR */
QVector<double> TurboDecoder::interleaveLLR(
    const QVector<double>& llr) const
{
    int n = llr.size();
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[m_interleaver[i]] = llr[i];
    }
    return result;
}

/** @brief 解交织LLR @param llr 输入LLR @return 解交织后LLR */
QVector<double> TurboDecoder::deinterleaveLLR(
    const QVector<double>& llr) const
{
    int n = llr.size();
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = llr[m_interleaver[i]];
    }
    return result;
}

/** @brief 生成交织图案 @param length 长度 @return 交织索引 */
QVector<int> TurboDecoder::generateInterleaver(int length)
{
    QVector<int> pattern(length);
    for (int i = 0; i < length; ++i) {
        pattern[i] = i;
    }
    std::mt19937 rng(42);
    std::shuffle(pattern.begin(), pattern.end(), rng);
    return pattern;
}

/** @brief 重置统计 */
void TurboDecoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterSum = 0.0;
}

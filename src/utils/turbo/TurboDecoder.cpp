/**
 * @file TurboDecoder.cpp
 * @brief Turbo解码器实现
 */

#include "TurboDecoder.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <limits>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

TurboDecoder::TurboDecoder(QObject* parent)
    : QObject(parent)
    , m_numStates(1 << (3 - 1)) // 默认约束长度3, 4个状态
{
}

TurboDecoder::~TurboDecoder() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void TurboDecoder::setMaxIterations(int maxIter)
{
    m_maxIter = std::min({std::max(maxIter, 1), 50});
}

int TurboDecoder::maxIterations() const { return m_maxIter; }

void TurboDecoder::setConstraintLength(int length)
{
    m_constraintLen = std::min({std::max(length, 3), 7});
    m_numStates = 1 << (m_constraintLen - 1);
}

void TurboDecoder::setGeneratorPolynomials(int g1, int g2)
{
    m_g1 = g1;
    m_g2 = g2;
}

// ═══════════════════════════════════════════════════════════
// 解码
// ═══════════════════════════════════════════════════════════

QVector<quint8> TurboDecoder::decode(const QVector<double>& sysReceived,
                                      const QVector<double>& par1Received,
                                      const QVector<double>& par2Received,
                                      const QVector<int>& interleaver)
{
    if (sysReceived.isEmpty()) {
        emit error(tr("Turbo解码错误: 输入为空"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    const int dataLen = sysReceived.size();
    QVector<double> extrinsic1(dataLen, 0.0);
    QVector<double> extrinsic2(dataLen, 0.0);
    int actualIters = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        actualIters++;

        // DEC1: 使用系统位 + 校验1 + 外信息2
        QVector<double> input1(dataLen);
        for (int i = 0; i < dataLen; ++i) {
            input1[i] = sysReceived[i] + par1Received[i] + extrinsic2[i];
        }
        extrinsic1 = bcjrLogMap(sysReceived, par1Received, extrinsic2);

        // 交织外信息
        QVector<double> interleavedExt(dataLen);
        for (int i = 0; i < dataLen; ++i) {
            const int iidx = (i < interleaver.size()) ? interleaver[i] : i;
            interleavedExt[i] = (iidx < dataLen) ? extrinsic1[iidx] : 0.0;
        }

        // DEC2: 使用交织后的系统位 + 校验2 + 交织后的外信息
        extrinsic2 = bcjrLogMap(sysReceived, par2Received, interleavedExt);

        // 解交织
        QVector<double> deinterleaved(dataLen);
        for (int i = 0; i < dataLen && i < interleaver.size(); ++i) {
            deinterleaved[interleaver[i]] = extrinsic2[i];
        }
        extrinsic2 = deinterleaved;
    }

    // 硬判决
    QVector<quint8> result;
    result.reserve(dataLen);
    for (int i = 0; i < dataLen; ++i) {
        const double llr = sysReceived[i] + extrinsic1[i] + extrinsic2[i];
        result.append(llr >= 0 ? 0 : 1);
    }

    m_stats.totalDecodes++;
    m_stats.totalIterations += static_cast<quint64>(actualIters);
    m_stats.totalBits += static_cast<quint64>(dataLen);
    m_stats.avgIterations = static_cast<double>(m_stats.totalIterations) /
                            static_cast<double>(m_stats.totalDecodes);
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalDecodes - 1) +
                         timer.elapsed()) / static_cast<double>(m_stats.totalDecodes);

    emit decoded(dataLen, actualIters);
    return result;
}

QVector<quint8> TurboDecoder::decodeMaxLogMap(const QVector<double>& sysReceived,
                                               const QVector<double>& par1Received,
                                               const QVector<double>& par2Received,
                                               const QVector<int>& interleaver)
{
    // Max-Log-MAP近似版本(使用max代替log-sum-exp)
    if (sysReceived.isEmpty()) {
        emit error(tr("Turbo Max-Log-MAP解码错误: 输入为空"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    const int dataLen = sysReceived.size();
    QVector<double> extrinsic1(dataLen, 0.0);
    QVector<double> extrinsic2(dataLen, 0.0);
    int actualIters = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        actualIters++;
        extrinsic1 = bcjrMaxLogMap(sysReceived, par1Received, extrinsic2);

        QVector<double> interleavedExt(dataLen);
        for (int i = 0; i < dataLen; ++i) {
            const int iidx = (i < interleaver.size()) ? interleaver[i] : i;
            interleavedExt[i] = (iidx < dataLen) ? extrinsic1[iidx] : 0.0;
        }

        extrinsic2 = bcjrMaxLogMap(sysReceived, par2Received, interleavedExt);

        QVector<double> deinterleaved(dataLen);
        for (int i = 0; i < dataLen && i < interleaver.size(); ++i) {
            deinterleaved[interleaver[i]] = extrinsic2[i];
        }
        extrinsic2 = deinterleaved;
    }

    QVector<quint8> result;
    result.reserve(dataLen);
    for (int i = 0; i < dataLen; ++i) {
        const double llr = sysReceived[i] + extrinsic1[i] + extrinsic2[i];
        result.append(llr >= 0 ? 0 : 1);
    }

    m_stats.totalDecodes++;
    m_stats.totalIterations += static_cast<quint64>(actualIters);
    m_stats.totalBits += static_cast<quint64>(dataLen);
    m_stats.avgIterations = static_cast<double>(m_stats.totalIterations) /
                            static_cast<double>(m_stats.totalDecodes);
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalDecodes - 1) +
                         timer.elapsed()) / static_cast<double>(m_stats.totalDecodes);

    emit decoded(dataLen, actualIters);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 交织器生成
// ═══════════════════════════════════════════════════════════

QVector<int> TurboDecoder::generateInterleaver(int length, quint32 seed)
{
    QVector<int> interleaver(length);
    for (int i = 0; i < length; ++i) interleaver[i] = i;

    // 简单的伪随机交织(基于种子)
    quint32 s = seed;
    for (int i = length - 1; i > 0; --i) {
        s = s * 1103515245 + 12345;
        const int j = static_cast<int>((s >> 16) % static_cast<quint32>(i + 1));
        std::swap(interleaver[i], interleaver[j]);
    }
    return interleaver;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

TurboDecoder::Stats TurboDecoder::stats() const { return m_stats; }
void TurboDecoder::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部: BCJR Log-MAP
// ═══════════════════════════════════════════════════════════

QVector<double> TurboDecoder::bcjrLogMap(const QVector<double>& sys,
                                          const QVector<double>& par,
                                          const QVector<double>& extrinsic)
{
    const int dataLen = sys.size();
    const int numStates = m_numStates;

    // 分支度量
    QVector<double> branchMetrics = computeBranchMetrics(sys, par, extrinsic);

    // Alpha(前向)
    QVector<QVector<double>> alpha(dataLen + 1, QVector<double>(numStates,
                                      -std::numeric_limits<double>::infinity()));
    alpha[0][0] = 0.0;
    computeAlpha(branchMetrics, alpha);

    // Beta(后向)
    QVector<QVector<double>> beta(dataLen + 1, QVector<double>(numStates,
                                      -std::numeric_limits<double>::infinity()));
    beta[dataLen][0] = 0.0;
    computeBeta(branchMetrics, beta);

    // 计算外信息(LLR)
    QVector<double> llr(dataLen, 0.0);
    for (int k = 0; k < dataLen; ++k) {
        double maxP1 = -std::numeric_limits<double>::infinity();
        double maxP0 = -std::numeric_limits<double>::infinity();

        for (int s = 0; s < numStates; ++s) {
            for (int u = 0; u <= 1; ++u) {
                const int ns = nextState(s, u);
                const double metric = alpha[k][s] + branchMetrics[k * numStates * 2 + s * 2 + u] + beta[k + 1][ns];
                if (u == 1) maxP1 = logSum(maxP1, metric);
                else maxP0 = logSum(maxP0, metric);
            }
        }

        llr[k] = maxP1 - maxP0 - sys[k] -
                 ((k < extrinsic.size()) ? extrinsic[k] : 0.0);
    }

    return llr;
}

QVector<double> TurboDecoder::bcjrMaxLogMap(const QVector<double>& sys,
                                             const QVector<double>& par,
                                             const QVector<double>& extrinsic)
{
    const int dataLen = sys.size();
    const int numStates = m_numStates;

    QVector<double> branchMetrics = computeBranchMetrics(sys, par, extrinsic);

    QVector<QVector<double>> alpha(dataLen + 1, QVector<double>(numStates,
                                      -std::numeric_limits<double>::infinity()));
    alpha[0][0] = 0.0;
    computeAlpha(branchMetrics, alpha);

    QVector<QVector<double>> beta(dataLen + 1, QVector<double>(numStates,
                                      -std::numeric_limits<double>::infinity()));
    beta[dataLen][0] = 0.0;
    computeBeta(branchMetrics, beta);

    QVector<double> llr(dataLen, 0.0);
    for (int k = 0; k < dataLen; ++k) {
        double maxP1 = -std::numeric_limits<double>::infinity();
        double maxP0 = -std::numeric_limits<double>::infinity();

        for (int s = 0; s < numStates; ++s) {
            for (int u = 0; u <= 1; ++u) {
                const int ns = nextState(s, u);
                const double metric = alpha[k][s] + branchMetrics[k * numStates * 2 + s * 2 + u] + beta[k + 1][ns];
                if (u == 1) maxP1 = std::max(maxP1, metric);
                else maxP0 = std::max(maxP0, metric);
            }
        }

        llr[k] = maxP1 - maxP0 - sys[k] -
                 ((k < extrinsic.size()) ? extrinsic[k] : 0.0);
    }

    return llr;
}

// ═══════════════════════════════════════════════════════════
// 内部: Alpha/Beta递归
// ═══════════════════════════════════════════════════════════

void TurboDecoder::computeAlpha(const QVector<double>& branchMetrics,
                                 QVector<QVector<double>>& alpha)
{
    const int dataLen = alpha.size() - 1;
    const int numStates = m_numStates;

    for (int k = 0; k < dataLen; ++k) {
        for (int ns = 0; ns < numStates; ++ns) {
            double maxVal = -std::numeric_limits<double>::infinity();
            for (int ps = 0; ps < numStates; ++ps) {
                for (int u = 0; u <= 1; ++u) {
                    if (nextState(ps, u) == ns) {
                        const double val = alpha[k][ps] +
                            branchMetrics[k * numStates * 2 + ps * 2 + u];
                        maxVal = logSum(maxVal, val);
                    }
                }
            }
            alpha[k + 1][ns] = maxVal;
        }
    }
}

void TurboDecoder::computeBeta(const QVector<double>& branchMetrics,
                                QVector<QVector<double>>& beta)
{
    const int dataLen = beta.size() - 1;
    const int numStates = m_numStates;

    for (int k = dataLen - 1; k >= 0; --k) {
        for (int ps = 0; ps < numStates; ++ps) {
            double maxVal = -std::numeric_limits<double>::infinity();
            for (int ns = 0; ns < numStates; ++ns) {
                for (int u = 0; u <= 1; ++u) {
                    if (nextState(ps, u) == ns) {
                        const double val = beta[k + 1][ns] +
                            branchMetrics[k * numStates * 2 + ps * 2 + u];
                        maxVal = logSum(maxVal, val);
                    }
                }
            }
            beta[k][ps] = maxVal;
        }
    }
}

QVector<double> TurboDecoder::computeBranchMetrics(const QVector<double>& sys,
                                                     const QVector<double>& par,
                                                     const QVector<double>& extrinsic)
{
    const int dataLen = sys.size();
    const int numStates = m_numStates;
    QVector<double> metrics(dataLen * numStates * 2, 0.0);

    for (int k = 0; k < dataLen; ++k) {
        for (int s = 0; s < numStates; ++s) {
            for (int u = 0; u <= 1; ++u) {
                const int out = output(s, u);
                const double sysPart = (u == 1) ? sys[k] : -sys[k];
                const double parPart = (out & 1) ? par[k] : -par[k];
                const double extPart = (k < extrinsic.size()) ?
                    ((u == 1) ? extrinsic[k] : -extrinsic[k]) : 0.0;
                metrics[k * numStates * 2 + s * 2 + u] = 0.5 * (sysPart + parPart + extPart);
            }
        }
    }
    return metrics;
}

int TurboDecoder::stateCount() const { return m_numStates; }

int TurboDecoder::nextState(int state, int input) const
{
    // RSC编码器: 移位寄存器
    const int feedback = input ^ ((state >> 0) & 1) ^ ((state >> 1) & 1);
    return ((state << 1) | feedback) & (m_numStates - 1);
}

int TurboDecoder::output(int state, int input) const
{
    // 校验输出(简化: 使用m_g2多项式)
    const int bit0 = (state >> 0) & 1;
    const int bit1 = (state >> 1) & 1;
    return input ^ bit0 ^ bit1;
}

double TurboDecoder::logSum(double a, double b)
{
    if (a == -std::numeric_limits<double>::infinity()) return b;
    if (b == -std::numeric_limits<double>::infinity()) return a;
    const double maxVal = std::max(a, b);
    return maxVal + std::log1p(std::exp(-std::abs(a - b)));
}

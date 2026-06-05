/**
 * @file SignalAligner.cpp
 * @brief 信号对齐器实现 — 互相关配准
 */

#include "utils/align/SignalAligner.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SignalAligner::SignalAligner(QObject* parent)
    : QObject(parent)
    , m_maxOffset(0)
    , m_timeSum(0.0)
{
}

/** @brief 对齐单个信号到参考信号
 *  @param reference 参考信号
 *  @param signal 待对齐信号
 *  @return 对齐结果 */
SignalAligner::AlignResult SignalAligner::align(
    const QVector<double>& reference,
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    AlignResult result;
    int nRef = reference.size();
    int nSig = signal.size();

    if (nRef < 4 || nSig < 4) {
        result.aligned = signal;
        emit alignmentCompleted(0, 0.0);
        return result;
    }

    /* 互相关找最佳偏移 */
    auto best = crossCorrelate(reference, signal);
    result.offset = best.first;
    result.correlation = best.second;

    /* 构建对齐后信号 */
    int outLen = nSig;
    result.aligned.resize(outLen);
    for (int i = 0; i < outLen; ++i) {
        int refIdx = i - result.offset;
        if (refIdx >= 0 && refIdx < nRef) {
            result.aligned[i] = signal[i];
        } else {
            result.aligned[i] = 0.0;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalAlignments;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAlignments);

    emit alignmentCompleted(result.offset, result.correlation);
    return result;
}

/** @brief 批量对齐多个信号
 *  @param signalList 待对齐信号列表
 *  @param reference 参考信号
 *  @return 对齐结果列表 */
QVector<SignalAligner::AlignResult> SignalAligner::alignMultiple(
    const QVector<QVector<double>>& signalList,
    const QVector<double>& reference)
{
    QElapsedTimer timer;
    timer.start();

    QVector<AlignResult> results;
    results.reserve(signalList.size());

    for (const auto& sig : signalList) {
        results.push_back(align(reference, sig));
    }

    double elapsed = static_cast<double>(timer.elapsed());
    /* align()已经更新过统计，这里不重复更新 */
    Q_UNUSED(elapsed);

    return results;
}

/** @brief 设置最大搜索范围 @param maxOffset 最大偏移 */
void SignalAligner::setMaxOffset(int maxOffset)
{
    m_maxOffset = qMax(0, maxOffset);
}

/** @brief 重置统计 */
void SignalAligner::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 归一化互相关
 *  @param ref 参考信号
 *  @param sig 目标信号
 *  @return (最佳偏移, 相关系数) */
QPair<int, double> SignalAligner::crossCorrelate(
    const QVector<double>& ref,
    const QVector<double>& sig) const
{
    int nRef = ref.size();
    int nSig = sig.size();
    int maxOff = (m_maxOffset > 0) ? qMin(m_maxOffset, nRef)
                                    : nRef / 2;

    /* 参考信号能量 */
    double refEnergy = 0.0;
    for (double v : ref) refEnergy += v * v;
    if (refEnergy < 1e-30) return {0, 0.0};

    int bestOffset = 0;
    double bestCorr = -2.0;

    for (int off = -maxOff; off <= maxOff; ++off) {
        double corr = 0.0;
        double sigEnergy = 0.0;
        int count = 0;

        int startRef = qMax(0, off);
        int endRef = qMin(nRef, nSig + off);
        for (int i = startRef; i < endRef; ++i) {
            int j = i - off;
            if (j >= 0 && j < nSig) {
                corr += ref[i] * sig[j];
                sigEnergy += sig[j] * sig[j];
                ++count;
            }
        }

        if (sigEnergy > 1e-30 && count > 0) {
            double normCorr = corr / qSqrt(refEnergy * sigEnergy);
            if (normCorr > bestCorr) {
                bestCorr = normCorr;
                bestOffset = off;
            }
        }
    }

    return {bestOffset, bestCorr};
}

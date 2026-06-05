/**
 * @file PolarDecoder.cpp
 * @brief Polar码解码器实现
 */

#include "utils/code25/PolarDecoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <numeric>

PolarDecoder::PolarDecoder(QObject* parent)
    : QObject(parent)
    , m_codeLength(8)
    , m_infoLength(4)
    , m_listSize(8)
    , m_algorithm(Algorithm::SCL)
    , m_crcPoly(0x04C11DB7)
    , m_timeSum(0.0)
{
    m_frozenSet = generateFrozenSet();
}

void PolarDecoder::setCodeLength(int n)
{
    /* 码长必须是2的幂 */
    int p = 1;
    while (p < n) p <<= 1;
    m_codeLength = p;
    m_frozenSet = generateFrozenSet();
}

void PolarDecoder::setInfoLength(int k)
{
    m_infoLength = qBound(1, k, m_codeLength - 1);
    m_frozenSet = generateFrozenSet();
}

void PolarDecoder::setListSize(int l)
{
    m_listSize = qMax(1, l);
}

void PolarDecoder::setAlgorithm(Algorithm algo)
{
    m_algorithm = algo;
}

void PolarDecoder::setCrcPolynomial(quint32 poly)
{
    m_crcPoly = poly;
}

void PolarDecoder::setFrozenSet(const QVector<int>& frozenIndices)
{
    m_frozenSet = frozenIndices;
    std::sort(m_frozenSet.begin(), m_frozenSet.end());
}

QVector<int> PolarDecoder::generateFrozenSet() const
{
    QVector<double> rel = computeReliability();
    QVector<QPair<double, int>> indexed;
    indexed.reserve(rel.size());
    for (int i = 0; i < rel.size(); ++i)
        indexed.append({rel[i], i});
    /* 可靠度低的作为冻结位 */
    std::sort(indexed.begin(), indexed.end());
    int frozenCount = m_codeLength - m_infoLength;
    QVector<int> frozen;
    for (int i = 0; i < frozenCount && i < indexed.size(); ++i)
        frozen.append(indexed[i].second);
    std::sort(frozen.begin(), frozen.end());
    return frozen;
}

PolarDecoder::DecodeResult PolarDecoder::decode(const QVector<double>& llrInput)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    if (llrInput.isEmpty()) {
        result.crcPassed = false;
        return result;
    }

    if (m_algorithm == Algorithm::SC) {
        result.decodedBits = scDecode(llrInput);
        result.pathMetric = 0.0;
        for (int i = 0; i < llrInput.size() && i < result.decodedBits.size(); ++i) {
            double llr = llrInput[i];
            int bit = result.decodedBits[i];
            result.pathMetric += (bit == 0)
                ? qLn(1.0 + qExp(-llr)) : qLn(1.0 + qExp(llr));
        }
    } else {
        result = sclDecode(llrInput);
    }

    /* CRC校验(取信息位) */
    QVector<int> infoBits;
    int frozenIdx = 0;
    for (int i = 0; i < result.decodedBits.size(); ++i) {
        if (frozenIdx < m_frozenSet.size() && m_frozenSet[frozenIdx] == i) {
            ++frozenIdx;
        } else {
            infoBits.append(result.decodedBits[i]);
        }
    }
    result.crcPassed = checkCrc(infoBits);

    ++m_stats.totalDecodes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodes;

    emit decodeComplete(result.crcPassed, result.pathMetric);
    return result;
}

QVector<int> PolarDecoder::scDecode(const QVector<double>& llr)
{
    QVector<int> bits(m_codeLength, 0);
    scRecursive(llr, bits, 0, m_codeLength);
    return bits;
}

void PolarDecoder::scRecursive(const QVector<double>& llr, QVector<int>& bits,
                                int offset, int len)
{
    if (len == 1) {
        /* 叶节点: 冻结位判0，否则看LLR符号 */
        bool isFrozen = m_frozenSet.contains(offset);
        if (isFrozen) {
            bits[offset] = 0;
        } else {
            bits[offset] = (llr[offset] < 0) ? 1 : 0;
        }
        return;
    }

    int half = len / 2;
    /* f函数: 上层LLR计算 */
    QVector<double> upperLlr(half, 0.0);
    for (int i = 0; i < half; ++i) {
        double a = llr[offset + i];
        double b = llr[offset + half + i];
        upperLlr[i] = qAtanh(qTanh(a / 2.0) * qTanh(b / 2.0)) * 2.0;
        /* 限幅防止溢出 */
        upperLlr[i] = qBound(-30.0, upperLlr[i], 30.0);
    }

    QVector<double> savedLlr = llr;
    scRecursive(upperLlr, bits, offset, half);

    /* g函数: 下层LLR计算 */
    QVector<double> lowerLlr(half, 0.0);
    for (int i = 0; i < half; ++i) {
        int u = bits[offset + i];
        double a = savedLlr[offset + i];
        double b = savedLlr[offset + half + i];
        lowerLlr[i] = (u == 0) ? (b - a) : (b + a);
        lowerLlr[i] = qBound(-30.0, lowerLlr[i], 30.0);
    }

    scRecursive(lowerLlr, bits, offset + half, half);
}

PolarDecoder::DecodeResult PolarDecoder::sclDecode(const QVector<double>& llr)
{
    QList<QVector<int>> paths;
    QList<double> metrics;

    paths.append(QVector<int>(m_codeLength, 0));
    metrics.append(0.0);

    for (int bitIdx = 0; bitIdx < m_codeLength; ++bitIdx) {
        bool isFrozen = m_frozenSet.contains(bitIdx);
        extendPaths(paths, metrics, llr, bitIdx);

        if (isFrozen) {
            /* 冻结位: 只保留bit=0的路径 */
            for (int p = paths.size() - 1; p >= 0; --p) {
                if (paths[p][bitIdx] != 0) {
                    paths.removeAt(p);
                    metrics.removeAt(p);
                }
            }
        }
        prunePaths(paths, metrics);
        m_stats.totalPathExtensions += paths.size();
    }

    /* 选最优路径 */
    int bestP = 0;
    double bestMetric = std::numeric_limits<double>::max();
    for (int p = 0; p < paths.size(); ++p) {
        if (metrics[p] < bestMetric) {
            bestMetric = metrics[p];
            bestP = p;
        }
    }

    m_stats.avgActivePaths = (m_stats.avgActivePaths * (m_stats.totalDecodes)
        + paths.size()) / (m_stats.totalDecodes + 1);

    DecodeResult result;
    result.decodedBits = (bestP < paths.size()) ? paths[bestP]
        : QVector<int>(m_codeLength, 0);
    result.pathMetric = bestMetric;
    result.crcPassed = false;
    return result;
}

void PolarDecoder::extendPaths(QList<QVector<int>>& paths,
    QList<double>& metrics, const QVector<double>& llr, int bitIdx)
{
    int pathCount = paths.size();
    double llrVal = (bitIdx < llr.size()) ? llr[bitIdx] : 0.0;

    for (int p = 0; p < pathCount; ++p) {
        QVector<int> path0 = paths[p];
        double metric0 = metrics[p];

        /* bit=0 的路径度量 */
        double m0 = metric0 + qLn(1.0 + qExp(-llrVal));
        /* bit=1 的路径度量 */
        double m1 = metric0 + qLn(1.0 + qExp(llrVal));

        /* 保留bit=0在原位 */
        paths[p][bitIdx] = 0;
        metrics[p] = m0;

        /* 新增bit=1路径 */
        QVector<int> path1 = path0;
        path1[bitIdx] = 1;
        paths.append(path1);
        metrics.append(m1);
    }

    emit pathExtended(paths.size(), m_listSize);
}

void PolarDecoder::prunePaths(QList<QVector<int>>& paths, QList<double>& metrics)
{
    if (paths.size() <= m_listSize) return;

    /* 按度量排序，保留前L条 */
    QList<int> indices;
    for (int i = 0; i < paths.size(); ++i) indices.append(i);
    std::sort(indices.begin(), indices.end(), [&metrics](int a, int b) {
        return metrics[a] < metrics[b];
    });

    QList<QVector<int>> newPaths;
    QList<double> newMetrics;
    for (int i = 0; i < m_listSize && i < indices.size(); ++i) {
        newPaths.append(paths[indices[i]]);
        newMetrics.append(metrics[indices[i]]);
    }
    paths = newPaths;
    metrics = newMetrics;
}

bool PolarDecoder::checkCrc(const QVector<int>& infoBits) const
{
    if (infoBits.isEmpty()) return true;
    quint32 crc = 0xFFFFFFFF;
    for (int bit : infoBits) {
        quint32 mask = (crc & 0x80000000) ? 1 : 0;
        mask ^= (bit ? 1 : 0);
        crc <<= 1;
        if (mask) crc ^= m_crcPoly;
    }
    return (crc & 0xFF) == 0;
}

QVector<double> PolarDecoder::computeReliability() const
{
    /* 巴塔查里亚参数近似 */
    int n = m_codeLength;
    QVector<double> rel(n, 0.0);
    rel[0] = 0.5;
    int currentLen = 1;
    while (currentLen < n) {
        QVector<double> newRel(2 * currentLen, 0.0);
        for (int i = 0; i < currentLen; ++i) {
            double z = rel[i];
            newRel[i] = z * z;
            newRel[currentLen + i] = 2.0 * z - z * z;
        }
        rel = newRel;
        currentLen *= 2;
    }
    /* 可靠度 = 1 - Z (Z越小越可靠) */
    for (double& r : rel) r = 1.0 - r;
    return rel;
}

void PolarDecoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

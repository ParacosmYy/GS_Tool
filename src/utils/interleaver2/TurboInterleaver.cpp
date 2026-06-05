/**
 * @file TurboInterleaver.cpp
 * @brief Turbo码交织器实现
 */

#include "utils/interleaver2/TurboInterleaver.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/* UMTS标准辅助素数表(部分) */
static const int s_umtsPrimeList[] = {
    7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139,
    149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
    223, 227, 229, 233, 239, 241, 251, 257
};
static const int s_numPrimes = sizeof(s_umtsPrimeList) / sizeof(s_umtsPrimeList[0]);

TurboInterleaver::TurboInterleaver(QObject* parent)
    : QObject(parent)
{
}

QVector<int> TurboInterleaver::generateSRandom(int length, int spread) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> perm(length, -1);
    QVector<bool> used(length, false);
    QRandomGenerator* rng = QRandomGenerator::global();

    int maxAttempts = length * 100;
    int attempts = 0;
    int pos = 0;

    while (pos < length && attempts < maxAttempts) {
        int candidate = rng->bounded(length);
        if (used[candidate]) {
            ++attempts;
            continue;
        }

        /* 检查S约束: 新位置与已选位置的距离不小于spread */
        bool valid = true;
        int checkStart = qMax(0, pos - spread);
        int checkEnd = pos;
        for (int i = checkStart; i < checkEnd; ++i) {
            if (qAbs(candidate - perm[i]) < spread) {
                valid = false;
                break;
            }
        }

        if (valid) {
            perm[pos] = candidate;
            used[candidate] = true;
            ++pos;
            attempts = 0;
        } else {
            ++attempts;
        }
    }

    /* 回退: 未填满的位置用剩余位置顺序填充 */
    if (pos < length) {
        for (int i = 0; i < length; ++i) {
            if (!used[i]) {
                perm[pos++] = i;
                used[i] = true;
            }
        }
    }

    m_stats.totalGenerations++;
    m_stats.totalElementsProcessed += length;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerations;

    emit generationCompleted(length, Mode::SRandom);
    return perm;
}

QVector<int> TurboInterleaver::generateUMTS(int length) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> perm(length);

    /* 查找满足 R >= K 的最小素数 p */
    int p = 0;
    for (int i = 0; i < s_numPrimes; ++i) {
        if (s_umtsPrimeList[i] >= length) {
            p = s_umtsPrimeList[i];
            break;
        }
    }
    /* 若表不足, 取最接近的素数 */
    if (p == 0) {
        p = s_umtsPrimeList[s_numPrimes - 1];
    }

    /* 生成内排列 */
    QVector<int> intraP = generateIntraPermutation(p);

    /* UMTS外排列: π(i) = (i * R) mod p + intraP[i mod p] */
    int v = 0;
    for (int i = 0; i < p && v < length; ++i) {
        int interleaved = ((i * length) % p) + intraP[i % p];
        if (interleaved < length) {
            perm[v++] = interleaved;
        }
    }

    m_stats.totalGenerations++;
    m_stats.totalElementsProcessed += length;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerations;

    emit generationCompleted(length, Mode::UMTS);
    return perm;
}

QVector<int> TurboInterleaver::generateDeinterleaver(
    const QVector<int>& interleaver) const
{
    int n = interleaver.size();
    QVector<int> deinter(n);
    for (int i = 0; i < n; ++i) {
        if (interleaver[i] >= 0 && interleaver[i] < n) {
            deinter[interleaver[i]] = i;
        }
    }
    return deinter;
}

QVector<double> TurboInterleaver::applyPermutation(
    const QVector<double>& data,
    const QVector<int>& permutation) const
{
    int n = qMin(data.size(), permutation.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        int idx = permutation[i];
        if (idx >= 0 && idx < data.size()) {
            result[i] = data[idx];
        }
    }
    return result;
}

QVector<int> TurboInterleaver::generateIntraPermutation(int p) const
{
    /* 简化UMTS内排列: 基于(q, r)生成伪随机排列 */
    QVector<int> intra(p);
    QVector<bool> used(p, false);
    QRandomGenerator* rng = QRandomGenerator::global();

    /* 找p的一个原根g */
    int g = 2;
    for (; g < p; ++g) {
        int val = 1;
        bool isPrimitive = true;
        for (int k = 1; k < p - 1; ++k) {
            val = (val * g) % p;
            if (val == 1) { isPrimitive = false; break; }
        }
        if (isPrimitive) break;
    }

    /* 生成内排列 */
    for (int i = 0; i < p; ++i) {
        int val = 1;
        for (int k = 0; k <= i; ++k) {
            val = (val * g) % p;
        }
        intra[i] = val % p;
    }

    /* 确保唯一性: 冲突时用递增 */
    QVector<bool> taken(p, false);
    for (int i = 0; i < p; ++i) {
        int v = intra[i];
        while (taken[v]) v = (v + 1) % p;
        intra[i] = v;
        taken[v] = true;
    }
    return intra;
}

TurboInterleaver::Stats TurboInterleaver::stats() const
{
    return m_stats;
}

void TurboInterleaver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

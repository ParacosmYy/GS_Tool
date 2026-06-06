/**
 * @file ReedMullerCode2.cpp
 * @brief ReedMullerCode2 实现
 *
 * 实现Reed-Muller码：生成矩阵构造、模2编码、
 * Hadamard变换快速多数逻辑译码。
 */

#include "utils/code163/ReedMullerCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

ReedMullerCode2::ReedMullerCode2(QObject* parent)
    : QObject(parent)
{
}

ReedMullerCode2::~ReedMullerCode2() = default;

bool ReedMullerCode2::setParameters(int r, int m)
{
    if (r < 0 || m < 1 || r > m) return false;
    m_params.r = r;
    m_params.m = m;
    m_params.n = 1 << m;
    m_params.d = 1 << (m - r);

    /* k = sum_{i=0}^{r} C(m,i) */
    m_params.k = 0;
    for (int i = 0; i <= r; ++i) {
        m_params.k += binomial(m, i);
    }

    buildGeneratorMatrix();
    return true;
}

int ReedMullerCode2::binomial(int n, int k)
{
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    k = qMin(k, n - k);
    int result = 1;
    for (int i = 0; i < k; ++i) {
        result = result * (n - i) / (i + 1);
    }
    return result;
}

QVector<QVector<int>> ReedMullerCode2::enumerateRows(int weight) const
{
    QVector<QVector<int>> result;
    int m = m_params.m;
    int total = 1 << m;

    if (weight == 0) {
        result.append(QVector<int>(total, 1));
        return result;
    }

    /* Generate all combinations of 'weight' positions from {0..m-1} */
    QVector<int> comb(weight);
    for (int i = 0; i < weight; ++i) comb[i] = i;

    while (true) {
        /* Build row from combination */
        QVector<int> row(total);
        for (int x = 0; x < total; ++x) {
            int prod = 1;
            for (int j = 0; j < weight; ++j) {
                prod &= ((x >> comb[j]) & 1) ? 0 : 1;
            }
            row[x] = prod;
        }
        result.append(row);

        /* Next combination */
        int i = weight - 1;
        while (i >= 0 && comb[i] == m - weight + i) i--;
        if (i < 0) break;
        comb[i]++;
        for (int j = i + 1; j < weight; ++j) {
            comb[j] = comb[j - 1] + 1;
        }
    }
    return result;
}

void ReedMullerCode2::buildGeneratorMatrix()
{
    m_generator.clear();
    for (int w = 0; w <= m_params.r; ++w) {
        auto rows = enumerateRows(w);
        for (const auto& row : rows) {
            m_generator.append(row);
        }
    }
}

QVector<int> ReedMullerCode2::xorVectors(const QVector<int>& a, const QVector<int>& b)
{
    int n = qMin(a.size(), b.size());
    QVector<int> result(n);
    for (int i = 0; i < n; ++i) result[i] = a[i] ^ b[i];
    return result;
}

QVector<int> ReedMullerCode2::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    if (message.size() != m_params.k || m_params.n == 0) return QVector<int>();

    /* c = message * G (mod 2) */
    QVector<int> codeword(m_params.n, 0);
    for (int j = 0; j < m_params.n; ++j) {
        int sum = 0;
        for (int i = 0; i < m_params.k; ++i) {
            sum ^= (message[i] & m_generator[i][j]);
        }
        codeword[j] = sum;
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit encodeCompleted(m_params.n);
    return codeword;
}

void ReedMullerCode2::hadamardTransform(QVector<double>& data) const
{
    int n = data.size();
    for (int step = 1; step < n; step <<= 1) {
        for (int i = 0; i < n; i += (step << 1)) {
            for (int j = 0; j < step; ++j) {
                double a = data[i + j];
                double b = data[i + j + step];
                data[i + j] = a + b;
                data[i + j + step] = a - b;
            }
        }
    }
}

QVector<int> ReedMullerCode2::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    if (received.size() != m_params.n || m_params.n == 0) return QVector<int>();

    int totalErrors = 0;

    /* Majority-logic decoding for RM(1,m) via fast Hadamard transform */
    if (m_params.r == 1) {
        /* Map 0->+1, 1->-1 */
        QVector<double> f(m_params.n);
        for (int i = 0; i < m_params.n; ++i) {
            f[i] = (received[i] == 0) ? 1.0 : -1.0;
        }

        hadamardTransform(f);

        /* Find index of maximum absolute value */
        double maxVal = 0;
        int maxIdx = 0;
        int sign = 1;
        for (int i = 0; i < m_params.n; ++i) {
            if (qAbs(f[i]) > maxVal) {
                maxVal = qAbs(f[i]);
                maxIdx = i;
                sign = (f[i] >= 0) ? 1 : -1;
            }
        }

        /* Reconstruct message from maxIdx and sign */
        QVector<int> message(m_params.k, 0);
        message[0] = (sign < 0) ? 1 : 0;
        for (int j = 0; j < m_params.m; ++j) {
            message[j + 1] = (maxIdx >> j) & 1;
        }

        /* Count errors by re-encoding and comparing */
        QVector<int> recoded = encode(message);
        for (int i = 0; i < m_params.n; ++i) {
            if (recoded[i] != received[i]) totalErrors++;
        }

        m_stats.totalBitErrors += totalErrors;
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

        emit decodeCompleted(totalErrors);
        return message;
    }

    /* General case: majority-logic decoding layer by layer */
    QVector<int> currentWord = received;
    QVector<int> decodedMsg(m_params.k, 0);

    for (int layer = m_params.r; layer >= 1; --layer) {
        int layerStart = 0;
        for (int l = 0; l < layer; ++l) layerStart += binomial(m_params.m, l);
        int layerLen = binomial(m_params.m, layer);

        /* For each basis vector in this layer, majority vote */
        for (int bi = 0; bi < layerLen; ++bi) {
            QVector<int> basis = m_generator[layerStart + bi];
            /* Compute dot products for majority vote */
            int votes0 = 0, votes1 = 0;
            for (int pos = 0; pos < m_params.n; ++pos) {
                int val = currentWord[pos] & basis[pos];
                if (val) votes1++; else votes0++;
            }
            int bitVal = (votes1 > votes0) ? 1 : 0;
            decodedMsg[layerStart + bi] = bitVal;

            if (bitVal) {
                currentWord = xorVectors(currentWord, basis);
            }
        }
    }

    /* Layer 0: all-ones vector */
    int votes0 = 0, votes1 = 0;
    for (int pos = 0; pos < m_params.n; ++pos) {
        if (currentWord[pos]) votes1++; else votes0++;
    }
    decodedMsg[0] = (votes1 > votes0) ? 1 : 0;

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decodeCompleted(totalErrors);
    return decodedMsg;
}

QVector<int> ReedMullerCode2::addNoise(const QVector<int>& codeword, int numErrors) const
{
    QVector<int> noisy = codeword;
    int n = noisy.size();
    numErrors = qMin(numErrors, n);
    for (int i = 0; i < numErrors; ++i) {
        int pos = i % n;
        noisy[pos] ^= 1;
    }
    return noisy;
}

void ReedMullerCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

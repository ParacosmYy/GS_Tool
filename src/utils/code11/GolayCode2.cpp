/**
 * @file GolayCode2.cpp
 * @brief 扩展 Golay (24,12,8) 码实现 — 编码 + 硬/软判决译码
 */

#include "utils/code11/GolayCode2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 *
 * 初始化生成矩阵的 B 部分 (12x12 循环矩阵)。
 * Golay 码的生成矩阵 G = [I12 | B]，其中 B 是:
 *
 * 1 1 0 0 0 1 1 1 0 1 0 1
 * 1 0 0 0 1 1 1 0 1 0 1 1
 * 0 0 0 1 1 1 0 1 0 1 1 1  (循环移位)
 * ... (12 行)
 */
GolayCode2::GolayCode2(QObject* parent)
    : QObject(parent)
{
    initGeneratorMatrix();
}

/**
 * @brief 初始化生成矩阵和校验矩阵
 *
 * B 矩阵第一行为 [1,1,0,0,0,1,1,1,0,1,0,1]，
 * 后续每行是前一行的循环右移。
 */
void GolayCode2::initGeneratorMatrix()
{
    /* Golay 码 B 矩阵的第一行 */
    int b0[] = {1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1};
    m_generatorB.resize(12);
    for (int i = 0; i < 12; ++i) {
        m_generatorB[i].resize(12);
        for (int j = 0; j < 12; ++j) {
            /* 循环右移: 第 i 行是第 0 行右移 i 位 */
            m_generatorB[i][j] = b0[(j - i + 12) % 12];
        }
    }

    /* 校验矩阵 H = [B^T | I12] */
    m_parityCheck.resize(12);
    for (int i = 0; i < 12; ++i) {
        m_parityCheck[i].resize(24);
        for (int j = 0; j < 12; ++j) {
            m_parityCheck[i][j] = m_generatorB[j][i]; /* B^T */
        }
        for (int j = 0; j < 12; ++j) {
            m_parityCheck[i][12 + j] = (i == j) ? 1 : 0; /* I12 */
        }
    }
}

/**
 * @brief 编码 12 位信息为 24 位码字
 * @param message 12 位信息向量
 * @return 24 位码字
 *
 * 码字 c = [m | m*B]，前 12 位为信息位，后 12 位为校验位。
 */
QVector<int> GolayCode2::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result(24, 0);
    if (message.size() != 12) {
        return result;
    }

    /* 信息位 */
    for (int i = 0; i < 12; ++i) {
        result[i] = message[i];
    }

    /* 校验位: p = m * B (GF(2) 矩阵乘) */
    QVector<int> parity = matrixMultiply(message, m_generatorB);
    for (int i = 0; i < 12; ++i) {
        result[12 + i] = parity[i] % 2;
    }

    ++m_stats.totalEncodes;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit encoded(24);
    return result;
}

/**
 * @brief 硬判决译码
 * @param received 接收到的 24 位硬判决序列
 * @return 译码结果
 *
 * 使用伴随式译码: 计算伴随式 s = r*H^T，查表或穷举
 * 最接近的有效码字。
 */
GolayCode2::DecodeResult GolayCode2::decodeHard(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    if (received.size() != 24) {
        ++m_stats.totalUncorrectable;
        return result;
    }

    /* 计算伴随式 */
    QVector<int> syn = syndrome(received);
    int synWeight = hammingWeight(syn);

    /* 如果伴随式为 0，无错误 */
    if (synWeight == 0) {
        result.decodedBits = received.mid(0, 12);
        result.codeword = received;
        result.correctedErrors = 0;
        result.success = true;

        ++m_stats.totalDecodes;
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);
        emit decoded(0, true);
        return result;
    }

    /* 穷举搜索: 尝试翻转 ≤3 个位，找到最小重量伴随式 */
    int minErrors = 4; /* 超过纠错能力 */
    QVector<int> bestCodeword = received;
    int bestWeight = hammingWeight(syn);

    /* 尝试翻转 1 位 */
    for (int i = 0; i < 24; ++i) {
        QVector<int> flipped = received;
        flipped[i] = 1 - flipped[i];
        QVector<int> s = syndrome(flipped);
        int w = hammingWeight(s);
        if (w < bestWeight) {
            bestWeight = w;
            bestCodeword = flipped;
            minErrors = 1;
            if (w == 0) break;
        }
    }

    if (bestWeight == 0) {
        result.decodedBits = bestCodeword.mid(0, 12);
        result.codeword = bestCodeword;
        result.correctedErrors = minErrors;
        result.success = true;
    } else {
        /* 尝试翻转 2 位 */
        for (int i = 0; i < 24 && bestWeight > 0; ++i) {
            for (int j = i + 1; j < 24 && bestWeight > 0; ++j) {
                QVector<int> flipped = received;
                flipped[i] = 1 - flipped[i];
                flipped[j] = 1 - flipped[j];
                QVector<int> s = syndrome(flipped);
                int w = hammingWeight(s);
                if (w < bestWeight) {
                    bestWeight = w;
                    bestCodeword = flipped;
                    minErrors = 2;
                    if (w == 0) { i = 24; break; }
                }
            }
        }

        if (bestWeight == 0) {
            result.decodedBits = bestCodeword.mid(0, 12);
            result.codeword = bestCodeword;
            result.correctedErrors = minErrors;
            result.success = true;
        } else {
            /* 尝试翻转 3 位 */
            bool found = false;
            for (int i = 0; i < 24 && !found; ++i) {
                for (int j = i + 1; j < 24 && !found; ++j) {
                    for (int k = j + 1; k < 24 && !found; ++k) {
                        QVector<int> flipped = received;
                        flipped[i] = 1 - flipped[i];
                        flipped[j] = 1 - flipped[j];
                        flipped[k] = 1 - flipped[k];
                        QVector<int> s = syndrome(flipped);
                        if (hammingWeight(s) == 0) {
                            result.decodedBits = flipped.mid(0, 12);
                            result.codeword = flipped;
                            result.correctedErrors = 3;
                            result.success = true;
                            found = true;
                        }
                    }
                }
            }
            if (!found) {
                ++m_stats.totalUncorrectable;
            }
        }
    }

    if (result.success) {
        m_stats.totalCorrectedBits += result.correctedErrors;
    }

    ++m_stats.totalDecodes;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit decoded(result.correctedErrors, result.success);
    return result;
}

/**
 * @brief 软判决译码 (Chase 算法)
 * @param softValues 接收到的 24 个软判决值
 * @return 译码结果
 *
 * Chase 算法:
 * 1. 对软值硬判决得到初始码字
 * 2. 找到可靠性最低的 3 个位置
 * 3. 枚举这些位置的所有翻转组合
 * 4. 对每个组合进行硬判决译码
 * 5. 选择相关度量最好的结果
 */
GolayCode2::DecodeResult GolayCode2::decodeSoft(const QVector<double>& softValues)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    if (softValues.size() != 24) {
        ++m_stats.totalUncorrectable;
        return result;
    }

    /* 硬判决: 正值 -> 0, 负值 -> 1 */
    QVector<int> hardDecision(24);
    for (int i = 0; i < 24; ++i) {
        hardDecision[i] = (softValues[i] < 0) ? 1 : 0;
    }

    /* 找可靠性最低的 3 个位置 (|软值| 最小) */
    QVector<QPair<double, int>> reliability;
    for (int i = 0; i < 24; ++i) {
        reliability.append({qAbs(softValues[i]), i});
    }
    std::sort(reliability.begin(), reliability.end());

    QVector<int> weakPositions;
    for (int i = 0; i < qMin(3, reliability.size()); ++i) {
        weakPositions.append(static_cast<int>(reliability[i].second));
    }

    /* 枚举弱位置的所有翻转组合 (2^3 = 8 种) */
    double bestMetric = -1e30;
    DecodeResult bestResult;

    int combos = 1 << weakPositions.size();
    for (int mask = 0; mask < combos; ++mask) {
        QVector<int> testWord = hardDecision;
        for (int b = 0; b < weakPositions.size(); ++b) {
            if (mask & (1 << b)) {
                int pos = weakPositions[b];
                testWord[pos] = 1 - testWord[pos];
            }
        }

        DecodeResult attempt = decodeHard(testWord);
        if (!attempt.success) continue;

        /* 计算相关度量: sum(softValues[i] * (1 - 2*codeword[i])) */
        double metric = 0.0;
        for (int i = 0; i < 24; ++i) {
            metric += softValues[i] * (1.0 - 2.0 * attempt.codeword[i]);
        }

        if (metric > bestMetric) {
            bestMetric = metric;
            bestResult = attempt;
        }
    }

    if (bestResult.success) {
        result = bestResult;
    } else {
        ++m_stats.totalUncorrectable;
    }

    ++m_stats.totalDecodes;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit decoded(result.correctedErrors, result.success);
    return result;
}

/**
 * @brief 计算伴随式
 * @param codeword 24 位码字
 * @return 12 位伴随式
 *
 * s = r * H^T (GF(2) 矩阵向量乘)
 */
QVector<int> GolayCode2::syndrome(const QVector<int>& codeword) const
{
    QVector<int> syn(12, 0);
    if (codeword.size() != 24) return syn;

    for (int i = 0; i < 12; ++i) {
        int sum = 0;
        for (int j = 0; j < 24; ++j) {
            sum += codeword[j] * m_parityCheck[i][j];
        }
        syn[i] = sum % 2;
    }

    return syn;
}

/**
 * @brief 计算汉明重量
 * @param bits 比特序列
 * @return 重量 (1 的个数)
 */
int GolayCode2::hammingWeight(const QVector<int>& bits) const
{
    int w = 0;
    for (int b : bits) {
        w += (b != 0) ? 1 : 0;
    }
    return w;
}

/** @brief 重置统计信息 */
void GolayCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief GF(2) 矩阵乘法
 * @param vec 12 维向量
 * @param mat 12x12 矩阵
 * @return 结果向量
 */
QVector<int> GolayCode2::matrixMultiply(const QVector<int>& vec,
                                        const QVector<QVector<int>>& mat) const
{
    int n = vec.size();
    QVector<int> result(n, 0);
    for (int i = 0; i < n; ++i) {
        int sum = 0;
        for (int j = 0; j < n; ++j) {
            sum += vec[j] * mat[i][j];
        }
        result[i] = sum % 2;
    }
    return result;
}

/**
 * @brief 向量异或
 * @param a 向量 a
 * @param b 向量 b
 * @return a XOR b
 */
QVector<int> GolayCode2::xorVectors(const QVector<int>& a,
                                    const QVector<int>& b) const
{
    QVector<int> result(a.size());
    for (int i = 0; i < a.size(); ++i) {
        result[i] = a[i] ^ b[i];
    }
    return result;
}

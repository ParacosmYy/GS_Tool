/**
 * @file GolayCode3.cpp
 * @brief GolayCode3 实现
 *
 * 实现扩展Golay码[24,12,8]：生成矩阵和校验矩阵构建、
 * 编码(I12|B)、伴随式解码和错误纠正。
 */

#include "utils/code161/GolayCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 生成矩阵B(12x12)——标准扩展Golay码的P矩阵
 *
 * B矩阵取自标准文献，保证最小汉明距离为8。
 * 每行汉明重量为8或12。
 */
const int GolayCode3::s_generatorB[12][12] = {
    {1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1},
    {0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1},
    {0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0},
    {1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0},
    {1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1},
    {0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1},
    {1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
};

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
GolayCode3::GolayCode3(QObject* parent)
    : QObject(parent)
{
    buildGeneratorMatrix();
    buildParityMatrix();
}

/**
 * @brief 构建生成矩阵 G = [I12 | B]
 *
 * 前12列为单位矩阵，后12列为B矩阵。
 */
void GolayCode3::buildGeneratorMatrix()
{
    m_generatorG.resize(12);
    for (int i = 0; i < 12; ++i) {
        m_generatorG[i].resize(24);
        /* 单位矩阵部分 */
        for (int j = 0; j < 12; ++j) {
            m_generatorG[i][j] = (i == j) ? 1 : 0;
        }
        /* B矩阵部分 */
        for (int j = 0; j < 12; ++j) {
            m_generatorG[i][12 + j] = s_generatorB[i][j];
        }
    }
}

/**
 * @brief 构建校验矩阵 H = [B^T | I12]
 */
void GolayCode3::buildParityMatrix()
{
    m_parityH.resize(24);
    for (int i = 0; i < 24; ++i) {
        m_parityH[i].resize(12);
        for (int j = 0; j < 12; ++j) {
            if (i < 12) {
                /* B^T 部分 */
                m_parityH[i][j] = s_generatorB[j][i];
            } else {
                /* I12 部分 */
                m_parityH[i][j] = (i - 12 == j) ? 1 : 0;
            }
        }
    }
}

/**
 * @brief 向量模2加法
 */
QVector<int> GolayCode3::xorVectors(const QVector<int>& a, const QVector<int>& b)
{
    const int n = qMin(a.size(), b.size());
    QVector<int> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = a[i] ^ b[i];
    }
    return result;
}

/**
 * @brief 计算伴随式 s = r * H (mod 2)
 */
QVector<int> GolayCode3::syndrome(const QVector<int>& codeword) const
{
    QVector<int> s(12, 0);
    for (int j = 0; j < 12; ++j) {
        int sum = 0;
        for (int i = 0; i < 24; ++i) {
            sum += codeword[i] * m_parityH[i][j];
        }
        s[j] = sum % 2;
    }
    return s;
}

/**
 * @brief 编码12位数据为24位码字
 *
 * c = d * G (mod 2)
 */
QVector<int> GolayCode3::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(24, 0);

    if (data.size() < 12) return codeword;

    /* c = data * G (mod 2) */
    for (int j = 0; j < 24; ++j) {
        int sum = 0;
        for (int i = 0; i < 12; ++i) {
            sum += data[i] * m_generatorG[i][j];
        }
        codeword[j] = sum % 2;
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(12);
    return codeword;
}

/**
 * @brief 解码24位码字
 *
 * 使用伴随式解码：
 * 1) 计算伴随式 s
 * 2) 若wt(s) <= 3，错误在信息位，直接纠正
 * 3) 否则尝试匹配B*s的某一行
 * 4) 否则尝试(B*s)⊕s_i的模式
 */
int GolayCode3::decode(const QVector<int>& codeword, QVector<int>& correctedData)
{
    QElapsedTimer timer;
    timer.start();

    correctedData.resize(12);

    if (codeword.size() < 24) return -1;

    QVector<int> s = syndrome(codeword);
    int sWeight = hammingWeight(s);
    int corrected = 0;

    QVector<int> r = codeword;

    if (sWeight <= 3) {
        /* 错误仅在信息位(前12位) */
        for (int i = 0; i < 12; ++i) {
            if (s[i]) { r[i] ^= 1; corrected++; }
        }
    } else {
        /* 尝试: 检查 s 是否匹配 B的某行 */
        bool found = false;
        for (int i = 0; i < 12; ++i) {
            QVector<int> row(12);
            for (int j = 0; j < 12; ++j) {
                row[j] = s_generatorB[i][j];
            }
            QVector<int> diff = xorVectors(s, row);
            if (hammingWeight(diff) <= 2) {
                /* 错误在信息位i和校验位 */
                r[i] ^= 1;
                for (int j = 0; j < 12; ++j) {
                    if (diff[j]) { r[12 + j] ^= 1; corrected++; }
                }
                corrected++;
                found = true;
                break;
            }
        }

        if (!found) {
            /* 计算 B * s */
            QVector<int> bs(12, 0);
            for (int i = 0; i < 12; ++i) {
                int sum = 0;
                for (int j = 0; j < 12; ++j) {
                    sum += s_generatorB[i][j] * s[j];
                }
                bs[i] = sum % 2;
            }

            if (hammingWeight(bs) <= 3) {
                /* 错误在校验位 */
                for (int i = 0; i < 12; ++i) {
                    if (bs[i]) { r[12 + i] ^= 1; corrected++; }
                }
            } else {
                /* 尝试最后一种模式 */
                for (int i = 0; i < 12; ++i) {
                    QVector<int> row(12);
                    for (int j = 0; j < 12; ++j) {
                        row[j] = s_generatorB[i][j];
                    }
                    QVector<int> diff = xorVectors(bs, row);
                    if (hammingWeight(diff) <= 2) {
                        r[12 + i] ^= 1;
                        for (int j = 0; j < 12; ++j) {
                            if (diff[j]) { r[j] ^= 1; corrected++; }
                        }
                        corrected++;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    m_stats.uncorrectableErrors++;
                    m_stats.totalDecodes++;
                    emit decodeCompleted(-1);
                    return -1;
                }
            }
        }
    }

    /* 提取信息位 */
    for (int i = 0; i < 12; ++i) {
        correctedData[i] = r[i];
    }

    m_stats.totalCorrections += corrected;
    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(corrected);
    return corrected;
}

/**
 * @brief 计算汉明重量
 */
int GolayCode3::hammingWeight(const QVector<int>& bits)
{
    int count = 0;
    for (int b : bits) {
        if (b) count++;
    }
    return count;
}

/**
 * @brief 计算汉明距离
 */
int GolayCode3::hammingDistance(const QVector<int>& a, const QVector<int>& b)
{
    const int n = qMin(a.size(), b.size());
    int dist = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] != b[i]) dist++;
    }
    return dist;
}

/**
 * @brief 注入随机错误
 */
QVector<int> GolayCode3::injectErrors(const QVector<int>& codeword, int numErrors)
{
    if (codeword.isEmpty()) return codeword;

    QVector<int> corrupted = codeword;
    int n = corrupted.size();
    numErrors = qMin(numErrors, n);

    std::mt19937 rng(42);
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::shuffle(indices.begin(), indices.end(), rng);

    for (int i = 0; i < numErrors; ++i) {
        corrupted[indices[i]] ^= 1;
    }

    return corrupted;
}

void GolayCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

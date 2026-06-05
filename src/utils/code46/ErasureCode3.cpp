/**
 * @file ErasureCode3.cpp
 * @brief 擦除码3 — Cauchy Reed-Solomon+矩阵编码 实现
 *
 * 基于 GF(256) 上的 Cauchy 矩阵实现 Reed-Solomon 擦除编码。
 * 编码时将数据分片与编码矩阵相乘生成校验分片；
 * 解码时选取可逆子矩阵恢复被擦除的数据分片。
 */

#include "utils/code46/ErasureCode3.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认编码参数并构建Cauchy矩阵
 * @param parent 父QObject
 */
ErasureCode3::ErasureCode3(QObject* parent)
    : QObject(parent)
{
    buildCauchyMatrix();
}

/**
 * @brief 设置擦除码参数
 * @param dataShards 数据分片数
 * @param parityShards 校验分片数
 * @param shardSize 每个分片的字节大小
 */
void ErasureCode3::setParameters(int dataShards, int parityShards, int shardSize)
{
    m_dataShards = qMax(1, dataShards);
    m_parityShards = qMax(1, parityShards);
    m_shardSize = qMax(1, shardSize);
    buildCauchyMatrix();
}

/**
 * @brief 编码：从数据分片生成校验分片
 *
 * 编码矩阵为 (totalShards x dataShards) 的 Cauchy 矩阵。
 * 上部为单位阵（对应数据分片），下部为校验生成矩阵。
 *
 * @param data 数据分片集合，大小为 m_dataShards
 * @return 全部分片（数据+校验），大小为 totalShards
 */
QVector<QVector<quint8>> ErasureCode3::encode(const QVector<QVector<quint8>>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<quint8>> result;
    int total = m_dataShards + m_parityShards;

    /* 复制数据分片 */
    for (int i = 0; i < m_dataShards && i < data.size(); ++i) {
        result.append(data[i]);
    }
    /* 补齐数据分片 */
    while (result.size() < m_dataShards) {
        result.append(QVector<quint8>(m_shardSize, 0));
    }

    /* 通过矩阵乘法生成校验分片 */
    for (int p = 0; p < m_parityShards; ++p) {
        QVector<quint8> parity(m_shardSize, 0);
        int rowIdx = m_dataShards + p;

        for (int j = 0; j < m_shardSize; ++j) {
            quint8 val = 0;
            for (int d = 0; d < m_dataShards; ++d) {
                val = val ^ gfMul(static_cast<quint8>(m_codingMatrix[rowIdx][d]),
                                  data[d][j]);
            }
            parity[j] = val;
        }
        result.append(parity);
    }

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodes++;
    m_stats.totalShardsProcessed += total;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(m_dataShards, m_parityShards);
    return result;
}

/**
 * @brief 解码：从存活分片恢复被擦除的分片
 *
 * 选择存活分片对应的矩阵行，构建可逆子矩阵，
 * 求逆后乘以存活分片数据恢复擦除分片。
 *
 * @param shards 分片集合，被擦除的位置可为空
 * @param erased 被擦除分片的索引列表
 * @return 解码是否成功
 */
bool ErasureCode3::decode(QVector<QVector<quint8>>& shards, const QVector<int>& erased)
{
    QElapsedTimer timer;
    timer.start();

    if (erased.size() > m_parityShards) {
        return false; /* 擦除数超过校验数，无法恢复 */
    }

    int nSurviving = m_dataShards + m_parityShards - erased.size();
    if (nSurviving < m_dataShards) {
        return false;
    }

    /* 选取存活分片的前 m_dataShards 个用于恢复 */
    QVector<int> surviving;
    QSet<int> erasedSet;
    for (int e : erased) {
        erasedSet.insert(e);
    }
    for (int i = 0; i < m_dataShards + m_parityShards; ++i) {
        if (!erasedSet.contains(i)) {
            surviving.append(i);
            if (surviving.size() >= m_dataShards) break;
        }
    }

    /* 构建解码子矩阵并求逆 */
    QVector<QVector<quint8>> subMatrix(m_dataShards, QVector<quint8>(m_dataShards, 0));
    for (int i = 0; i < m_dataShards; ++i) {
        for (int j = 0; j < m_dataShards; ++j) {
            subMatrix[i][j] = m_codingMatrix[surviving[i]][j];
        }
    }

    /* 高斯消元求逆矩阵 */
    int n = m_dataShards;
    QVector<QVector<quint8>> inv(n, QVector<quint8>(n, 0));
    for (int i = 0; i < n; ++i) {
        inv[i][i] = 1;
    }

    /* 前向消元 */
    for (int col = 0; col < n; ++col) {
        /* 寻找主元 */
        int pivotRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (subMatrix[row][col] != 0) {
                pivotRow = row;
                break;
            }
        }
        if (subMatrix[pivotRow][col] == 0) return false;

        /* 交换行 */
        if (pivotRow != col) {
            qSwap(subMatrix[col], subMatrix[pivotRow]);
            qSwap(inv[col], inv[pivotRow]);
        }

        /* 主元归一化 */
        quint8 pivotVal = subMatrix[col][col];
        quint8 pivotInv = gfInv(pivotVal);
        for (int j = 0; j < n; ++j) {
            subMatrix[col][j] = gfMul(subMatrix[col][j], pivotInv);
            inv[col][j] = gfMul(inv[col][j], pivotInv);
        }

        /* 消去其他行 */
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            quint8 factor = subMatrix[row][col];
            if (factor == 0) continue;
            for (int j = 0; j < n; ++j) {
                subMatrix[row][j] = subMatrix[row][j] ^ gfMul(factor, subMatrix[col][j]);
                inv[row][j] = inv[row][j] ^ gfMul(factor, inv[col][j]);
            }
        }
    }

    /* 使用逆矩阵恢复擦除分片 */
    for (int e : erased) {
        if (e >= m_dataShards) continue; /* 校验分片可由数据分片重新编码 */
        QVector<quint8> recovered(m_shardSize, 0);
        for (int j = 0; j < m_shardSize; ++j) {
            quint8 val = 0;
            for (int i = 0; i < m_dataShards; ++i) {
                val = val ^ gfMul(inv[e][i], shards[surviving[i]][j]);
            }
            recovered[j] = val;
        }
        shards[e] = recovered;
    }

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodes++;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(erased.size());
    return true;
}

/**
 * @brief 检查分片集合的有效性
 * @param shards 待检查的分片集合
 * @return 无效或被擦除的分片索引列表
 */
QVector<int> ErasureCode3::checkShards(const QVector<QVector<quint8>>& shards) const
{
    QVector<int> invalid;
    for (int i = 0; i < shards.size(); ++i) {
        if (shards[i].size() != m_shardSize) {
            invalid.append(i);
        }
    }
    return invalid;
}

/**
 * @brief 重置统计信息
 */
void ErasureCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建 Cauchy 编码矩阵
 *
 * 矩阵大小为 (totalShards x dataShards)。
 * 上部为单位阵，下部为 Cauchy 矩阵元素: 1/(i XOR j)，其中 i != j。
 */
void ErasureCode3::buildCauchyMatrix()
{
    int rows = m_dataShards + m_parityShards;
    int cols = m_dataShards;
    m_codingMatrix.resize(rows, QVector<quint8>(cols, 0));

    /* 上部：单位阵 */
    for (int i = 0; i < m_dataShards; ++i) {
        m_codingMatrix[i][i] = 1;
    }

    /* 下部：Cauchy 矩阵元素 1/(i XOR j) in GF(256) */
    for (int i = m_dataShards; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            quint8 a = static_cast<quint8>(i);
            quint8 b = static_cast<quint8>(j);
            if (a != b) {
                m_codingMatrix[i][j] = gfInv(a ^ b);
            } else {
                m_codingMatrix[i][j] = 1;
            }
        }
    }
}

/**
 * @brief GF(256) 矩阵乘法
 * @param A 左矩阵
 * @param B 右矩阵
 * @param C 输出矩阵
 */
void ErasureCode3::gfMatrixMultiply(const QVector<QVector<quint8>>& A,
                                     const QVector<QVector<quint8>>& B,
                                     QVector<QVector<quint8>>& C) const
{
    int rA = A.size();
    int cA = (rA > 0) ? A[0].size() : 0;
    int cB = (B.size() > 0) ? B[0].size() : 0;
    C.resize(rA, QVector<quint8>(cB, 0));

    for (int i = 0; i < rA; ++i) {
        for (int j = 0; j < cB; ++j) {
            quint8 sum = 0;
            for (int k = 0; k < cA; ++k) {
                sum = sum ^ gfMul(A[i][k], B[k][j]);
            }
            C[i][j] = sum;
        }
    }
}

/**
 * @brief GF(256) 乘法（使用对数/反对数表）
 * @param a 操作数a
 * @param b 操作数b
 * @return a * b in GF(256)
 */
quint8 ErasureCode3::gfMul(quint8 a, quint8 b) const
{
    if (a == 0 || b == 0) return 0;

    /* 基于 Carry-Lassigne 算法的简化 GF 乘法 */
    int result = 0;
    int aa = a, bb = b;
    while (bb > 0) {
        if (bb & 1) {
            result ^= aa;
        }
        aa <<= 1;
        if (aa & 0x100) {
            aa ^= 0x11D; /* GF(256) 不可约多项式 x^8+x^4+x^3+x^2+1 */
        }
        bb >>= 1;
    }
    return static_cast<quint8>(result);
}

/**
 * @brief GF(256) 乘法逆元
 * @param a 非零元素
 * @return a^{-1} in GF(256)
 */
quint8 ErasureCode3::gfInv(quint8 a) const
{
    if (a == 0) return 0;
    /* 通过费马小定理: a^{-1} = a^{254} in GF(256) */
    quint8 result = 1;
    int power = 254;
    quint8 base = a;
    while (power > 0) {
        if (power & 1) {
            result = gfMul(result, base);
        }
        base = gfMul(base, base);
        power >>= 1;
    }
    return result;
}

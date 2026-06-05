/**
 * @file ReedSolomon3.cpp
 * @brief Reed-Solomon纠删编码引擎实现 — GF(2^8)域运算
 */

#include "utils/code9/ReedSolomon3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * GF(2^8) 不可约多项式: x^8 + x^4 + x^3 + x^2 + 1 = 0x11D
 * 使用 0x1D (去掉x^8位) 作为低8位表示
 */
static const quint8 GF_POLY = 0x1D;

/* GF(2^8) 指数表和对数表(延迟初始化) */
static quint8 gfExp[512];  /* 双倍长度方便查表 */
static quint8 gfLog[256];
static bool gfTablesReady = false;

/** @brief 初始化GF(2^8)指数/对数表 */
static void initGfTables()
{
    if (gfTablesReady) return;

    quint32 x = 1;
    for (int i = 0; i < 255; ++i) {
        gfExp[i] = static_cast<quint8>(x);
        gfLog[static_cast<quint8>(x)] = static_cast<quint8>(i);
        x <<= 1;
        if (x & 0x100) {
            x ^= GF_POLY;
        }
    }
    /* 复制前255项到后255项,方便加法不取模 */
    for (int i = 255; i < 512; ++i) {
        gfExp[i] = gfExp[i - 255];
    }
    gfLog[0] = 0; /* 特殊值,不使用 */
    gfTablesReady = true;
}

/** @brief 构造函数 @param parent 父对象 */
ReedSolomon3::ReedSolomon3(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
    initGfTables();
}

/** @brief GF(2^8)乘法 @param a @param b @return a*b */
quint8 ReedSolomon3::gfMul(quint8 a, quint8 b)
{
    initGfTables();
    if (a == 0 || b == 0) return 0;
    return gfExp[gfLog[a] + gfLog[b]];
}

/** @brief GF(2^8)除法 @param a @param b @return a/b */
quint8 ReedSolomon3::gfDiv(quint8 a, quint8 b)
{
    initGfTables();
    if (a == 0) return 0;
    if (b == 0) return 0; /* 除零保护 */
    return gfExp[(gfLog[a] + 255 - gfLog[b]) % 255];
}

/** @brief GF(2^8)求逆 @param a @return a^{-1} */
quint8 ReedSolomon3::gfInv(quint8 a)
{
    initGfTables();
    if (a == 0) return 0;
    return gfExp[255 - gfLog[a]];
}

/** @brief GF(2^8)指数运算 @param a 底数 @param n 指数 @return a^n */
quint8 ReedSolomon3::gfPow(quint8 a, int n)
{
    initGfTables();
    if (a == 0) return (n == 0) ? 1 : 0;
    return gfExp[(static_cast<int>(gfLog[a]) * n) % 255];
}

/** @brief GF(2^8)加法(异或) @param a @param b @return a+b */
quint8 ReedSolomon3::gfAdd(quint8 a, quint8 b)
{
    return a ^ b;
}

/** @brief 编码: 数据分片 + 生成校验分片 @param data 原始数据 @param dataShards 数据分片数 @param parityShards 校验分片数 @return 所有分片 */
QList<QByteArray> ReedSolomon3::encode(const QByteArray& data,
                                        int dataShards,
                                        int parityShards)
{
    QElapsedTimer timer;
    timer.start();

    int totalShards = dataShards + parityShards;
    QList<QByteArray> result;

    if (dataShards <= 0 || parityShards <= 0 || data.isEmpty()) {
        return result;
    }

    /* 计算分片大小(向上取整) */
    int shardSize = (data.size() + dataShards - 1) / dataShards;

    /* 切分数据分片(不足补零) */
    for (int i = 0; i < dataShards; ++i) {
        QByteArray shard(shardSize, 0);
        int copyLen = qMin(shardSize, data.size() - i * shardSize);
        if (copyLen > 0) {
            memcpy(shard.data(), data.constData() + i * shardSize, copyLen);
        }
        result.append(shard);
    }

    /* 生成编码矩阵并计算校验分片 */
    QVector<QVector<quint8>> matrix = buildMatrix(totalShards, dataShards);

    for (int i = 0; i < parityShards; ++i) {
        QByteArray parity(shardSize, 0);
        for (int byteIdx = 0; byteIdx < shardSize; ++byteIdx) {
            quint8 val = 0;
            for (int j = 0; j < dataShards; ++j) {
                val = gfAdd(val, gfMul(matrix[dataShards + i][j],
                                       static_cast<quint8>(result[j][byteIdx])));
            }
            parity[byteIdx] = static_cast<char>(val);
        }
        result.append(parity);
    }

    m_stats.totalEncodes++;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(dataShards, totalShards);
    return result;
}

/** @brief 解码/恢复丢失分片 @param shards 分片数组 @param dataShards 数据分片数 @param parityShards 校验分片数 @param shardSize 分片大小 @return 原始数据 */
QByteArray ReedSolomon3::decode(const QList<QByteArray>& shards,
                                 int dataShards,
                                 int parityShards,
                                 int shardSize)
{
    QElapsedTimer timer;
    timer.start();

    int totalShards = dataShards + parityShards;
    QByteArray result;

    if (shards.size() != totalShards || shardSize <= 0) {
        return result;
    }

    /* 找出可用和丢失的分片索引 */
    QList<int> availableRows;
    QList<int> missingDataRows;

    for (int i = 0; i < totalShards; ++i) {
        if (shards[i].size() == shardSize) {
            availableRows.append(i);
        } else if (i < dataShards) {
            missingDataRows.append(i);
        }
    }

    if (availableRows.size() < dataShards) {
        return result; /* 可用分片不足,无法恢复 */
    }

    /* 如果没有丢失,直接拼接 */
    if (missingDataRows.isEmpty()) {
        for (int i = 0; i < dataShards; ++i) {
            result.append(shards[i]);
        }
        /* 去掉尾部补零 */
        return result;
    }

    /* 构建解码矩阵: 从编码矩阵中选取可用行,求逆 */
    QVector<QVector<quint8>> encMatrix = buildMatrix(totalShards, dataShards);

    /* 取前dataShards个可用行构成方阵 */
    QVector<QVector<quint8>> subMatrix(dataShards, QVector<quint8>(dataShards));
    for (int i = 0; i < dataShards; ++i) {
        int row = availableRows[i];
        for (int j = 0; j < dataShards; ++j) {
            subMatrix[i][j] = encMatrix[row][j];
        }
    }

    QVector<QVector<quint8>> invSub = invertMatrix(subMatrix);
    if (invSub.isEmpty()) return result;

    /* 使用逆矩阵恢复所有数据分片 */
    QList<QByteArray> recoveredData(dataShards);
    for (int i = 0; i < dataShards; ++i) {
        recoveredData[i].resize(shardSize);
    }

    for (int byteIdx = 0; byteIdx < shardSize; ++byteIdx) {
        /* 收集可用分片的当前字节 */
        QVector<quint8> availBytes(dataShards);
        for (int i = 0; i < dataShards; ++i) {
            availBytes[i] = static_cast<quint8>(
                shards[availableRows[i]][byteIdx]);
        }

        /* 矩阵向量乘法恢复原始字节 */
        for (int i = 0; i < dataShards; ++i) {
            quint8 val = 0;
            for (int j = 0; j < dataShards; ++j) {
                val = gfAdd(val, gfMul(invSub[i][j], availBytes[j]));
            }
            recoveredData[i][byteIdx] = static_cast<char>(val);
        }
    }

    /* 拼接数据分片 */
    for (int i = 0; i < dataShards; ++i) {
        result.append(recoveredData[i]);
    }

    m_stats.totalDecodes++;
    m_stats.totalRecoveredShards += missingDataRows.size();
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(missingDataRows.size(), result.size());
    return result;
}

/** @brief 生成Vandermonde编码矩阵 @param rows 行数 @param cols 列数 @return 编码矩阵 */
QVector<QVector<quint8>> ReedSolomon3::buildMatrix(int rows, int cols)
{
    initGfTables();
    QVector<QVector<quint8>> matrix(rows, QVector<quint8>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            /* Vandermonde元素: i^j in GF(2^8) */
            if (j == 0) {
                matrix[i][j] = 1;
            } else {
                matrix[i][j] = gfPow(static_cast<quint8>(i + 1), j);
            }
        }
    }

    /* 上半部分(数据分片)为单位矩阵 */
    for (int i = 0; i < qMin(rows, cols); ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = (i == j) ? 1 : 0;
        }
    }

    return matrix;
}

/** @brief GF(2^8)上矩阵求逆(Gauss-Jordan) @param matrix 输入方阵 @return 逆矩阵 */
QVector<QVector<quint8>> ReedSolomon3::invertMatrix(
    const QVector<QVector<quint8>>& matrix)
{
    initGfTables();
    int n = matrix.size();
    if (n <= 0) return {};

    /* 增广矩阵 [M | I] */
    QVector<QVector<quint8>> aug(n, QVector<quint8>(2 * n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            aug[i][j] = matrix[i][j];
        }
        aug[i][n + i] = 1;
    }

    /* Gauss-Jordan消元(在GF(2^8)上) */
    for (int col = 0; col < n; ++col) {
        /* 寻找主元 */
        int pivotRow = -1;
        for (int row = col; row < n; ++row) {
            if (aug[row][col] != 0) {
                pivotRow = row;
                break;
            }
        }
        if (pivotRow < 0) return {}; /* 奇异 */

        /* 交换行 */
        if (pivotRow != col) {
            std::swap(aug[col], aug[pivotRow]);
        }

        /* 归一化主元行 */
        quint8 pivotVal = aug[col][col];
        quint8 pivotInv = gfInv(pivotVal);
        for (int j = 0; j < 2 * n; ++j) {
            aug[col][j] = gfMul(aug[col][j], pivotInv);
        }

        /* 消去其他行 */
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            quint8 factor = aug[row][col];
            if (factor == 0) continue;
            for (int j = 0; j < 2 * n; ++j) {
                aug[row][j] = gfAdd(aug[row][j], gfMul(factor, aug[col][j]));
            }
        }
    }

    /* 提取逆矩阵 */
    QVector<QVector<quint8>> inv(n, QVector<quint8>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inv[i][j] = aug[i][n + j];
        }
    }
    return inv;
}

/** @brief 重置统计信息 */
void ReedSolomon3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

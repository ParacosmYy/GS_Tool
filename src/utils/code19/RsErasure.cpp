/**
 * @file RsErasure.cpp
 * @brief Reed-Solomon纠删码引擎实现 — Cauchy RS/Vandermonde/高斯消元解码
 */

#include "utils/code19/RsErasure.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief GF(256)原始多项式 x^8+x^4+x^3+x^2+1 = 0x11D */
static constexpr quint8 GF_PRIM = 0x1D;

/** @brief 构造函数 @param parent 父对象 */
RsErasure::RsErasure(QObject* parent)
    : QObject(parent)
    , m_dataShards(0)
    , m_parityShards(0)
    , m_matrixType(MatrixType::Cauchy)
    , m_initialized(false)
    , m_timeSum(0.0)
{
    buildGfTables();
}

/** @brief 初始化编解码参数 @param dataShards 数据分片数 @param parityShards 校验分片数 @param type 矩阵类型 @return 是否成功 */
bool RsErasure::initialize(int dataShards, int parityShards, MatrixType type)
{
    if (dataShards < 1 || parityShards < 1
        || dataShards + parityShards > 255) {
        return false;
    }

    m_dataShards = dataShards;
    m_parityShards = parityShards;
    m_matrixType = type;

    if (type == MatrixType::Vandermonde) {
        buildVandermondeMatrix();
    } else {
        buildCauchyMatrix();
    }

    m_initialized = true;
    return true;
}

/** @brief 编码数据 @param data 原始数据 @return 编码后的分片列表(数据+校验) */
QVector<QByteArray> RsErasure::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QByteArray> result;
    if (!m_initialized || data.isEmpty()) return result;

    int totalShards = m_dataShards + m_parityShards;
    int shardSize = (data.size() + m_dataShards - 1) / m_dataShards;

    /* 将数据分成dataShards个等长片 */
    QVector<quint8*> dataPtrs(m_dataShards);
    QVector<QByteArray> paddedData(m_dataShards);
    for (int i = 0; i < m_dataShards; ++i) {
        paddedData[i].resize(shardSize, 0);
        int offset = i * shardSize;
        int len = qMin(shardSize, data.size() - offset);
        if (len > 0) {
            memcpy(paddedData[i].data(), data.constData() + offset, len);
        }
        dataPtrs[i] = reinterpret_cast<quint8*>(paddedData[i].data());
    }

    /* 先放入数据分片 */
    for (int i = 0; i < m_dataShards; ++i) {
        result.append(paddedData[i]);
    }

    /* 计算校验分片: parity[j] = sum(encodeMatrix[j][i] * data[i]) */
    for (int j = 0; j < m_parityShards; ++j) {
        QByteArray parity(shardSize, 0);
        quint8* pParity = reinterpret_cast<quint8*>(parity.data());
        const quint8* row = m_encodeMatrix.constData()
                            + j * m_dataShards;

        for (int byteIdx = 0; byteIdx < shardSize; ++byteIdx) {
            quint8 val = 0;
            for (int i = 0; i < m_dataShards; ++i) {
                val = gfAdd(val, gfMultiply(row[i], dataPtrs[i][byteIdx]));
            }
            pParity[byteIdx] = val;
        }
        result.append(parity);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalEncodes;
    m_stats.totalShardsProcessed += static_cast<quint64>(totalShards);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeComplete(totalShards, data.size());
    return result;
}

/** @brief 解码恢复数据 @param shards 分片列表(缺失位置为空) @return 恢复的原始数据 */
QByteArray RsErasure::decode(const QVector<QByteArray>& shards)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) return {};

    int totalShards = m_dataShards + m_parityShards;
    if (shards.size() != totalShards) {
        emit decodeFailed(tr("分片数量不匹配"));
        return {};
    }

    /* 统计缺失分片 */
    QVector<int> missingIndices;
    QVector<int> presentIndices;
    for (int i = 0; i < totalShards; ++i) {
        if (shards[i].isEmpty()) {
            missingIndices.append(i);
        } else {
            presentIndices.append(i);
        }
    }

    /* 无缺失: 直接拼接数据分片 */
    if (missingIndices.isEmpty()) {
        QByteArray result;
        for (int i = 0; i < m_dataShards; ++i) {
            result.append(shards[i]);
        }
        return result;
    }

    /* 缺失超过parityShards个: 无法恢复 */
    if (missingIndices.size() > m_parityShards) {
        ++m_stats.totalDecodesFailed;
        emit decodeFailed(tr("擦除分片数超过校验容量"));
        return {};
    }

    int shardSize = shards[presentIndices[0]].size();

    /* 构建解码矩阵: 从编码矩阵中选取存在的行 */
    QVector<quint8> decodeMat(m_dataShards * m_dataShards);
    QVector<int> selectedRows;

    /* 优先选数据分片对应的行(单位矩阵部分) */
    for (int i = 0; i < m_dataShards && selectedRows.size() < m_dataShards; ++i) {
        if (!shards[i].isEmpty()) {
            selectedRows.append(i);
        }
    }
    /* 补充校验分片行 */
    for (int i = m_dataShards; i < totalShards && selectedRows.size() < m_dataShards; ++i) {
        if (!shards[i].isEmpty()) {
            selectedRows.append(i);
        }
    }

    /* 构建选取的矩阵(取前dataShards列) */
    for (int r = 0; r < m_dataShards; ++r) {
        int srcRow = selectedRows[r];
        if (srcRow < m_dataShards) {
            /* 单位矩阵行 */
            for (int c = 0; c < m_dataShards; ++c) {
                decodeMat[r * m_dataShards + c] = (c == srcRow) ? 1 : 0;
            }
        } else {
            /* 编码矩阵行 */
            int parityIdx = srcRow - m_dataShards;
            for (int c = 0; c < m_dataShards; ++c) {
                decodeMat[r * m_dataShards + c] =
                    m_encodeMatrix[parityIdx * m_dataShards + c];
            }
        }
    }

    /* 高斯消元求逆 */
    QVector<int> pivotCols;
    QVector<quint8> inverse(m_dataShards * m_dataShards, 0);
    for (int i = 0; i < m_dataShards; ++i) {
        inverse[i * m_dataShards + i] = 1;
    }

    /* 增广矩阵 [decodeMat | I] 做高斯消元 */
    QVector<quint8> aug(m_dataShards * (2 * m_dataShards));
    for (int r = 0; r < m_dataShards; ++r) {
        for (int c = 0; c < m_dataShards; ++c) {
            aug[r * 2 * m_dataShards + c] = decodeMat[r * m_dataShards + c];
            aug[r * 2 * m_dataShards + m_dataShards + c] =
                inverse[r * m_dataShards + c];
        }
    }

    /* 前向消元 */
    for (int col = 0; col < m_dataShards; ++col) {
        /* 找主元 */
        int pivotRow = -1;
        for (int r = col; r < m_dataShards; ++r) {
            if (aug[r * 2 * m_dataShards + col] != 0) {
                pivotRow = r;
                break;
            }
        }
        if (pivotRow < 0) {
            ++m_stats.totalDecodesFailed;
            emit decodeFailed(tr("矩阵奇异,无法解码"));
            return {};
        }

        /* 交换行 */
        if (pivotRow != col) {
            for (int c = 0; c < 2 * m_dataShards; ++c) {
                std::swap(aug[col * 2 * m_dataShards + c],
                          aug[pivotRow * 2 * m_dataShards + c]);
            }
        }

        /* 主元归一化 */
        quint8 pivotVal = aug[col * 2 * m_dataShards + col];
        quint8 pivotInv = gfInverse(pivotVal);
        for (int c = 0; c < 2 * m_dataShards; ++c) {
            aug[col * 2 * m_dataShards + c] =
                gfMultiply(aug[col * 2 * m_dataShards + c], pivotInv);
        }

        /* 消元 */
        for (int r = 0; r < m_dataShards; ++r) {
            if (r == col) continue;
            quint8 factor = aug[r * 2 * m_dataShards + col];
            if (factor == 0) continue;
            for (int c = 0; c < 2 * m_dataShards; ++c) {
                aug[r * 2 * m_dataShards + c] = gfAdd(
                    aug[r * 2 * m_dataShards + c],
                    gfMultiply(factor, aug[col * 2 * m_dataShards + c]));
            }
        }
    }

    /* 提取逆矩阵(右半部分) */
    for (int r = 0; r < m_dataShards; ++r) {
        for (int c = 0; c < m_dataShards; ++c) {
            inverse[r * m_dataShards + c] =
                aug[r * 2 * m_dataShards + m_dataShards + c];
        }
    }

    /* 用逆矩阵恢复数据 */
    QByteArray recovered(m_dataShards * shardSize, 0);
    for (int byteIdx = 0; byteIdx < shardSize; ++byteIdx) {
        for (int i = 0; i < m_dataShards; ++i) {
            quint8 val = 0;
            for (int r = 0; r < m_dataShards; ++r) {
                int srcIdx = selectedRows[r];
                quint8 byteVal = static_cast<quint8>(shards[srcIdx][byteIdx]);
                val = gfAdd(val, gfMultiply(inverse[i * m_dataShards + r], byteVal));
            }
            recovered[i * shardSize + byteIdx] = static_cast<char>(val);
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalDecodes;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeComplete(recovered.size(), missingIndices.size());
    return recovered;
}

/** @brief 重置统计 */
void RsErasure::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 构建GF(256)对数/反对数查找表 */
void RsErasure::buildGfTables()
{
    quint8 x = 1;
    for (int i = 0; i < 255; ++i) {
        m_gfExp[i] = x;
        m_gfLog[x] = static_cast<quint8>(i);
        x = static_cast<quint8>((x << 1) ^ ((x & 0x80) ? GF_PRIM : 0));
    }
    /* 扩展指数表避免取模 */
    for (int i = 255; i < 512; ++i) {
        m_gfExp[i] = m_gfExp[i - 255];
    }
    m_gfLog[0] = 0;
}

/** @brief GF(256)乘法 @param a @param b @return a*b mod GF */
quint8 RsErasure::gfMultiply(quint8 a, quint8 b) const
{
    if (a == 0 || b == 0) return 0;
    return m_gfExp[m_gfLog[a] + m_gfLog[b]];
}

/** @brief GF(256)求逆 @param a @return a^(-1) */
quint8 RsErasure::gfInverse(quint8 a) const
{
    if (a == 0) return 0;
    return m_gfExp[255 - m_gfLog[a]];
}

/** @brief GF(256)除法 @param a @param b @return a/b */
quint8 RsErasure::gfDivide(quint8 a, quint8 b) const
{
    if (a == 0) return 0;
    if (b == 0) return 0; /* 除零 */
    return m_gfExp[m_gfLog[a] + 255 - m_gfLog[b]];
}

/** @brief 构建Vandermonde编码矩阵 */
void RsErasure::buildVandermondeMatrix()
{
    m_encodeMatrix.resize(m_parityShards * m_dataShards);
    for (int j = 0; j < m_parityShards; ++j) {
        for (int i = 0; i < m_dataShards; ++i) {
            /* V[j][i] = (j+1)^i in GF(256) */
            quint8 val = 1;
            for (int p = 0; p < i; ++p) {
                val = gfMultiply(val, static_cast<quint8>(j + 1));
            }
            m_encodeMatrix[j * m_dataShards + i] = val;
        }
    }
}

/** @brief 构建Cauchy编码矩阵(1/(i XOR j)保证可逆) */
void RsErasure::buildCauchyMatrix()
{
    m_encodeMatrix.resize(m_parityShards * m_dataShards);
    for (int j = 0; j < m_parityShards; ++j) {
        for (int i = 0; i < m_dataShards; ++i) {
            /* Cauchy元素: 1/(i XOR (j + dataShards)) */
            quint8 row = static_cast<quint8>(j + m_dataShards);
            quint8 col = static_cast<quint8>(i);
            quint8 denom = row ^ col;
            if (denom == 0) {
                m_encodeMatrix[j * m_dataShards + i] = 0;
            } else {
                m_encodeMatrix[j * m_dataShards + i] = gfInverse(denom);
            }
        }
    }
}

/** @brief 高斯消元(辅助方法) @param mat 矩阵 @param rows 行数 @param cols 列数 @param pivotCols 主元列 */
void RsErasure::gaussianElimination(QVector<quint8>& mat, int rows, int cols,
                                    QVector<int>& pivotCols) const
{
    pivotCols.clear();
    for (int col = 0; col < cols && pivotCols.size() < rows; ++col) {
        int pivot = -1;
        for (int r = static_cast<int>(pivotCols.size()); r < rows; ++r) {
            if (mat[r * cols + col] != 0) {
                pivot = r;
                break;
            }
        }
        if (pivot < 0) continue;

        int curRow = static_cast<int>(pivotCols.size());
        if (pivot != curRow) {
            for (int c = 0; c < cols; ++c) {
                std::swap(mat[curRow * cols + c], mat[pivot * cols + c]);
            }
        }

        quint8 pivVal = mat[curRow * cols + col];
        quint8 pivInv = gfInverse(pivVal);
        for (int c = 0; c < cols; ++c) {
            mat[curRow * cols + c] = gfMultiply(mat[curRow * cols + c], pivInv);
        }

        for (int r = 0; r < rows; ++r) {
            if (r == curRow) continue;
            quint8 factor = mat[r * cols + col];
            if (factor == 0) continue;
            for (int c = 0; c < cols; ++c) {
                mat[r * cols + c] = gfAdd(mat[r * cols + c],
                    gfMultiply(factor, mat[curRow * cols + c]));
            }
        }
        pivotCols.append(col);
    }
}

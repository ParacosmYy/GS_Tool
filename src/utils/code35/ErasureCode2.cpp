/**
 * @file ErasureCode2.cpp
 * @brief Reed-Solomon纠删码实现 - 基于Vandermonde矩阵的编解码
 *
 * 使用有限域GF(256)上的Vandermonde矩阵构造编码矩阵，
 * 解码时通过高斯消元重建丢失的数据分片。
 */

#include "utils/code35/ErasureCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数(4数据+2校验)
 * @param parent 父QObject
 */
ErasureCode2::ErasureCode2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置纠删码参数
 * @param dataShards 数据分片数量
 * @param parityShards 校验分片数量
 */
void ErasureCode2::configure(int dataShards, int parityShards)
{
    m_data = qMax(1, dataShards);
    m_parity = qMax(1, parityShards);
}

/** @return 数据分片数量 */
int ErasureCode2::dataShards() const { return m_data; }

/** @return 校验分片数量 */
int ErasureCode2::parityShards() const { return m_parity; }

/** @return 总分片数量 */
int ErasureCode2::totalShards() const { return m_data + m_parity; }

/**
 * @brief GF(256)乘法 - 使用对数/反对数查表
 *
 * 基于本原多项式 x^8 + x^4 + x^3 + x^2 + 1 (0x11D)
 * @param a 第一个操作数
 * @param b 第二个操作数
 * @return GF(256)乘积
 */
static quint8 gfMul(quint8 a, quint8 b)
{
    if (a == 0 || b == 0) return 0;

    /* 使用 exp/log 查表加速 */
    static bool tablesInit = false;
    static quint8 gfExp[512];
    static quint8 gfLog[256];

    if (!tablesInit) {
        quint8 x = 1;
        for (int i = 0; i < 255; ++i) {
            gfExp[i] = x;
            gfLog[x] = static_cast<quint8>(i);
            quint16 nx = static_cast<quint16>(x) << 1;
            if (nx & 0x100) nx ^= 0x11D;
            x = static_cast<quint8>(nx);
        }
        for (int i = 255; i < 512; ++i)
            gfExp[i] = gfExp[i - 255];
        tablesInit = true;
    }

    int idx = static_cast<int>(gfLog[a]) + static_cast<int>(gfLog[b]);
    return gfExp[idx];
}

/**
 * @brief GF(256)加法(异或)
 */
static quint8 gfAdd(quint8 a, quint8 b) { return a ^ b; }

/**
 * @brief GF(256)求逆
 */
static quint8 gfInv(quint8 a)
{
    if (a == 0) return 0;
    static quint8 invTable[256];
    static bool init = false;
    if (!init) {
        invTable[0] = 0;
        for (int i = 1; i < 256; ++i) {
            for (int j = 1; j < 256; ++j) {
                if (gfMul(static_cast<quint8>(i), static_cast<quint8>(j)) == 1) {
                    invTable[i] = static_cast<quint8>(j);
                    break;
                }
            }
        }
        init = true;
    }
    return invTable[a];
}

/**
 * @brief 对数据进行纠删编码
 *
 * 构建Vandermonde编码矩阵，将数据分片与编码矩阵相乘生成校验分片。
 *
 * @param data 输入数据分片，每个元素为一个字节向量(长度必须一致)
 * @return 编码后的所有分片(数据+校验)
 */
QVector<QVector<quint8>> ErasureCode2::encode(const QVector<QVector<quint8>>& data) const
{
    QElapsedTimer timer;
    timer.start();

    const int total = m_data + m_parity;
    const int chunkSize = (data.isEmpty()) ? 0 : data[0].size();

    QVector<QVector<quint8>> output(total);
    /* 复制数据分片 */
    for (int i = 0; i < m_data && i < data.size(); ++i)
        output[i] = data[i];

    /* 构建Vandermonde编码矩阵 (parity x data) */
    /* V[i][j] = j^i in GF(256) */
    QVector<QVector<quint8>> encMatrix(m_parity, QVector<quint8>(m_data, 0));
    for (int i = 0; i < m_parity; ++i) {
        for (int j = 0; j < m_data; ++j) {
            quint8 val = 1;
            for (int p = 0; p < i; ++p)
                val = gfMul(val, static_cast<quint8>(j + 1));
            encMatrix[i][j] = val;
        }
    }

    /* 生成校验分片 */
    for (int p = 0; p < m_parity; ++p) {
        output[m_data + p].resize(chunkSize, 0);
        for (int b = 0; b < chunkSize; ++b) {
            quint8 sum = 0;
            for (int d = 0; d < m_data; ++d)
                sum = gfAdd(sum, gfMul(encMatrix[p][d], data[d][b]));
            output[m_data + p][b] = sum;
        }
    }

    const_cast<ErasureCode2*>(this)->m_stats.totalEncodes++;
    const_cast<ErasureCode2*>(this)->m_stats.totalSymbolsProcessed += chunkSize * total;
    const_cast<ErasureCode2*>(this)->m_timeSum += timer.elapsed();
    const_cast<ErasureCode2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    const_cast<ErasureCode2*>(this)->emit encodeComplete(total);
    return output;
}

/**
 * @brief 对丢失分片进行解码恢复
 *
 * 识别丢失的分片，从存活分片构建解码矩阵，通过高斯消元求解。
 *
 * @param shards 所有分片(丢失的可以填空向量)
 * @param present 标识每个分片是否存活
 * @return true 解码成功，false 无法恢复(存活分片不足)
 */
bool ErasureCode2::decode(QVector<QVector<quint8>>& shards, const QVector<bool>& present)
{
    QElapsedTimer timer;
    timer.start();

    const int total = m_data + m_parity;
    const int chunkSize = (shards.isEmpty()) ? 0 : shards[0].size();

    /* 统计存活分片数 */
    int alive = 0;
    for (int i = 0; i < qMin(present.size(), total); ++i)
        if (present[i]) alive++;

    if (alive < m_data) {
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return false;
    }

    /* 收集存活的行索引(优先取数据分片) */
    QVector<int> aliveRows;
    for (int i = 0; i < total && aliveRows.size() < m_data; ++i)
        if (i < present.size() && present[i]) aliveRows.push_back(i);

    /* 构建解码矩阵: 选取Vandermonde中存活行对应的行 */
    /* 先构建完整的Vandermonde矩阵 */
    auto buildVandermondeRow = [](int row, int cols) -> QVector<quint8> {
        QVector<quint8> r(cols, 0);
        for (int j = 0; j < cols; ++j) {
            quint8 val = 1;
            for (int p = 0; p < row; ++p)
                val = gfMul(val, static_cast<quint8>(j + 1));
            r[j] = val;
        }
        return r;
    };

    /* 构建需要求解的方阵 */
    int n = m_data;
    QVector<QVector<quint8>> mat(n, QVector<quint8>(n, 0));
    for (int i = 0; i < n; ++i) {
        mat[i] = buildVandermondeRow(aliveRows[i], m_data);
    }

    /* 高斯消元求逆 */
    /* 构建增广矩阵 [mat | I] */
    QVector<QVector<quint8>> aug(n, QVector<quint8>(2 * n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            aug[i][j] = mat[i][j];
        aug[i][n + i] = 1;
    }

    /* 前向消元 */
    for (int col = 0; col < n; ++col) {
        /* 寻找主元行 */
        int pivot = -1;
        for (int row = col; row < n; ++row) {
            if (aug[row][col] != 0) { pivot = row; break; }
        }
        if (pivot < 0) {
            m_stats.totalDecodes++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
            return false;
        }
        /* 交换行 */
        if (pivot != col) std::swap(aug[pivot], aug[col]);

        /* 归一化主元行 */
        quint8 invPivot = gfInv(aug[col][col]);
        for (int j = 0; j < 2 * n; ++j)
            aug[col][j] = gfMul(aug[col][j], invPivot);

        /* 消元其他行 */
        for (int row = 0; row < n; ++row) {
            if (row == col || aug[row][col] == 0) continue;
            quint8 factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j)
                aug[row][j] = gfAdd(aug[row][j], gfMul(factor, aug[col][j]));
        }
    }

    /* 提取逆矩阵 */
    QVector<QVector<quint8>> invMat(n, QVector<quint8>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            invMat[i][j] = aug[i][n + j];

    /* 重建丢失的数据分片 */
    int recovered = 0;
    for (int i = 0; i < m_data; ++i) {
        if (i < present.size() && present[i]) continue;
        /* 该数据分片丢失，使用逆矩阵恢复 */
        shards[i].resize(chunkSize, 0);
        for (int b = 0; b < chunkSize; ++b) {
            quint8 sum = 0;
            for (int j = 0; j < n; ++j)
                sum = gfAdd(sum, gfMul(invMat[i][j], shards[aliveRows[j]][b]));
            shards[i][b] = sum;
        }
        recovered++;
    }

    /* 使用数据分片重建丢失的校验分片 */
    for (int p = m_data; p < total; ++p) {
        if (p < present.size() && present[p]) continue;
        auto row = buildVandermondeRow(p - m_data, m_data);
        shards[p].resize(chunkSize, 0);
        for (int b = 0; b < chunkSize; ++b) {
            quint8 sum = 0;
            for (int d = 0; d < m_data; ++d)
                sum = gfAdd(sum, gfMul(row[d], shards[d][b]));
            shards[p][b] = sum;
        }
        recovered++;
    }

    m_stats.totalDecodes++;
    m_stats.totalSymbolsProcessed += chunkSize * recovered;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeComplete(recovered);
    return true;
}

/**
 * @brief 重置所有统计数据
 */
void ErasureCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

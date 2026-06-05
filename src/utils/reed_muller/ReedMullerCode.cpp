/**
 * @file ReedMullerCode.cpp
 * @brief Reed-Muller编解码引擎实现 — RM(r,m)分组码
 */

#include "utils/reed_muller/ReedMullerCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ReedMullerCode::ReedMullerCode(QObject* parent)
    : QObject(parent)
    , m_r(1)
    , m_m(3)
    , m_n(8)
    , m_k(4)
    , m_timeSum(0.0)
{
    buildGeneratorMatrix();
}

/** @brief 设置RM码参数 @param r 阶数 @param m 参数 */
void ReedMullerCode::setOrder(int r, int m)
{
    m_r = qMax(0, qMin(r, m));
    m_m = qMax(1, m);
    m_n = 1 << m_m;

    /* 信息位长度 k = sum_{i=0}^{r} C(m, i) */
    m_k = 0;
    for (int i = 0; i <= m_r; ++i) {
        m_k += binomial(m_m, i);
    }

    buildGeneratorMatrix();
}

/** @brief 构建生成矩阵 */
void ReedMullerCode::buildGeneratorMatrix()
{
    m_genMatrix.assign(m_k, QVector<int>(m_n, 0));

    /* RM(r,m)生成矩阵的行对应m个变量的所有阶数<=r的组合 */
    /* 使用变量索引: v_1, v_2, ..., v_m */
    /* 每个变量在2^m个码字上取值: v_j在第j位循环0/1 */

    /* 生成变量向量 */
    QVector<QVector<int>> vars(m_m, QVector<int>(m_n));
    for (int j = 0; j < m_m; ++j) {
        int blockSize = 1 << j;
        for (int i = 0; i < m_n; ++i) {
            vars[j][i] = ((i / blockSize) % 2 == 0) ? 1 : 0;
        }
    }

    /* 生成所有阶数<=r的行 */
    int rowIdx = 0;

    /* 第0行: 全1向量(阶数0) */
    for (int i = 0; i < m_n; ++i) {
        m_genMatrix[rowIdx][i] = 1;
    }
    ++rowIdx;

    /* 阶数1到r的行 */
    for (int order = 1; order <= m_r; ++order) {
        /* 生成所有从m个变量中选order个的组合 */
        QVector<int> indices(m_m);
        for (int i = 0; i < m_m; ++i) indices[i] = i;

        /* 使用递归生成组合 */
        QVector<QVector<int>> combos;
        QVector<int> current;
        generateCombinations(indices, order, 0, current, combos);

        for (const auto& combo : combos) {
            if (rowIdx >= m_k) break;
            for (int i = 0; i < m_n; ++i) {
                int val = 1;
                for (int idx : combo) {
                    val &= vars[idx][i];
                }
                m_genMatrix[rowIdx][i] = val;
            }
            ++rowIdx;
        }
    }
}

/** @brief 递归生成组合 @param indices 索引数组 @param k 选择数 @param start 起始位置 @param current 当前组合 @param result 所有组合 */
void ReedMullerCode::generateCombinations(const QVector<int>& indices, int k,
                                           int start, QVector<int>& current,
                                           QVector<QVector<int>>& result)
{
    if (static_cast<int>(current.size()) == k) {
        result.append(current);
        return;
    }
    for (int i = start; i < indices.size(); ++i) {
        current.append(indices[i]);
        generateCombinations(indices, k, i + 1, current, result);
        current.removeLast();
    }
}

/** @brief 编码数据 @param data 原始数据 @return 编码后数据 */
QByteArray ReedMullerCode::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 将输入数据转换为比特 */
    int inputBits = data.size() * 8;
    int numBlocks = inputBits / m_k;
    if (numBlocks == 0 && inputBits > 0) numBlocks = 1;

    QByteArray result;

    for (int block = 0; block < numBlocks; ++block) {
        /* 提取k个信息位 */
        QVector<int> msgBits(m_k, 0);
        for (int i = 0; i < m_k; ++i) {
            int bitPos = block * m_k + i;
            int byteIdx = bitPos / 8;
            int bitIdx = 7 - (bitPos % 8);
            if (byteIdx < data.size()) {
                msgBits[i] = (static_cast<quint8>(data[byteIdx]) >> bitIdx) & 1;
            }
        }

        /* 矩阵乘法: codeword = message * G (mod 2) */
        QVector<int> codeword(m_n, 0);
        for (int j = 0; j < m_n; ++j) {
            int sum = 0;
            for (int i = 0; i < m_k; ++i) {
                sum += msgBits[i] * m_genMatrix[i][j];
            }
            codeword[j] = sum % 2;
        }

        /* 将码字打包为字节 */
        for (int i = 0; i < m_n; i += 8) {
            quint8 byte = 0;
            for (int b = 0; b < 8 && (i + b) < m_n; ++b) {
                byte |= (static_cast<quint8>(codeword[i + b]) << (7 - b));
            }
            result.append(static_cast<char>(byte));
        }
    }

    /* 更新统计 */
    ++m_stats.totalEncoded;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

/** @brief 解码数据(多数逻辑解码) @param received 接收数据 @return 解码后数据 */
QByteArray ReedMullerCode::decode(const QByteArray& received)
{
    QElapsedTimer timer;
    timer.start();

    bool success = true;
    int outputBytes = (m_k + 7) / 8;
    QByteArray result;

    /* 将接收数据转换为比特 */
    QVector<int> recvBits = bytesToBits(received, m_n);

    if (recvBits.size() < m_n) {
        recvBits.resize(m_n, 0);
    }

    /* 多数逻辑解码: 从最高阶到最低阶依次解码 */
    QVector<int> decodedBits(m_k, 0);

    /* 计算每行对应的校验位置 */
    int rowIdx = 0;

    /* 阶数0: 全1行 */
    decodedBits[rowIdx] = majorityVote(recvBits, m_genMatrix[rowIdx]);
    ++rowIdx;

    /* 从已解码位中减去低阶贡献 */
    QVector<int> residual(recvBits);

    /* 阶数1到r: 依次解码 */
    for (int order = 1; order <= m_r; ++order) {
        /* 对每个阶为order的行进行多数逻辑判决 */
        int rowsInOrder = binomial(m_m, order);
        for (int r = 0; r < rowsInOrder && rowIdx < m_k; ++r) {
            /* 构造校验掩码并判决 */
            decodedBits[rowIdx] = majorityVote(residual, m_genMatrix[rowIdx]);

            /* 减去已解码行的贡献 */
            if (decodedBits[rowIdx] == 1) {
                for (int i = 0; i < m_n; ++i) {
                    residual[i] = (residual[i] + 2 - m_genMatrix[rowIdx][i]) % 2;
                }
            }
            ++rowIdx;
        }
    }

    /* 将解码比特打包为字节 */
    result = bitsToBytes(decodedBits);

    /* 更新统计 */
    ++m_stats.totalDecoded;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decodingCompleted(success);
    return result;
}

/** @brief 多数逻辑判决 @param received 接收比特 @param pattern 生成行 @return 判决结果(0或1) */
int ReedMullerCode::majorityVote(const QVector<int>& received,
                                  const QVector<int>& pattern) const
{
    int sum = 0;
    int count = 0;
    for (int i = 0; i < m_n; ++i) {
        if (pattern[i] == 1) {
            sum += received[i];
            ++count;
        }
    }
    if (count == 0) return 0;
    return (sum * 2 >= count) ? 1 : 0;
}

/** @brief 计算组合数C(n,k) @param n 总数 @param k 选择数 @return 组合数 */
int ReedMullerCode::binomial(int n, int k) const
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

/** @brief 字节转比特数组 @param data 字节数据 @param bitCount 所需比特数 @return 比特数组 */
QVector<int> ReedMullerCode::bytesToBits(const QByteArray& data,
                                          int bitCount) const
{
    QVector<int> bits(bitCount, 0);
    for (int i = 0; i < bitCount; ++i) {
        int byteIdx = i / 8;
        int bitIdx = 7 - (i % 8);
        if (byteIdx < data.size()) {
            bits[i] = (static_cast<quint8>(data[byteIdx]) >> bitIdx) & 1;
        }
    }
    return bits;
}

/** @brief 比特数组转字节 @param bits 比特数组 @return 字节数据 */
QByteArray ReedMullerCode::bitsToBytes(const QVector<int>& bits) const
{
    QByteArray result;
    int bytes = (bits.size() + 7) / 8;
    for (int i = 0; i < bytes; ++i) {
        quint8 byte = 0;
        for (int b = 0; b < 8; ++b) {
            int idx = i * 8 + b;
            if (idx < bits.size()) {
                byte |= (static_cast<quint8>(bits[idx]) << (7 - b));
            }
        }
        result.append(static_cast<char>(byte));
    }
    return result;
}

/** @brief 重置统计 */
void ReedMullerCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

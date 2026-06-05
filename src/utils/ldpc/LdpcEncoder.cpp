/**
 * @file LdpcEncoder.cpp
 * @brief LDPC码编码器/解码器实现
 */

#include "LdpcEncoder.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

LdpcEncoder::LdpcEncoder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_totalOps(0)
    , m_n(0)
    , m_k(0)
    , m_m(0)
    , m_matrixReady(false)
{
}

void LdpcEncoder::setParityMatrix(const QVector<QVector<int>>& H)
{
    m_H = H;
    m_m = H.size();
    if (m_m > 0)
        m_n = H[0].size();
    else
        m_n = 0;
    m_k = m_n - m_m;

    /* 计算生成矩阵 */
    m_matrixReady = computeGeneratorMatrix();
}

QByteArray LdpcEncoder::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_matrixReady || data.size() != m_k) {
        m_stats.totalEncoded++;
        m_timeSum += timer.elapsed();
        m_totalOps++;
        m_stats.avgProcessingTimeMs =
            (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
        return QByteArray();
    }

    /* 系统码编码: codeword = [data | parity]
     * parity_bits = data * G_parity (模2) */
    QByteArray codeword(m_n, 0);

    /* 信息位直接拷贝 */
    for (int i = 0; i < m_k; ++i)
        codeword[i] = data[i];

    /* 计算校验位: 对每个校验方程求模2和 */
    for (int i = 0; i < m_m; ++i) {
        int sum = 0;
        for (int j = 0; j < m_k; ++j) {
            if (m_G[i + m_k][j] != 0) {
                sum ^= (data[j] != 0) ? 1 : 0;
            }
        }
        codeword[m_k + i] = static_cast<char>(sum);
    }

    /* 统计更新 */
    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    return codeword;
}

QByteArray LdpcEncoder::decode(const QByteArray& received, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    if (m_H.isEmpty() || received.size() != m_n) {
        m_stats.totalDecoded++;
        m_timeSum += timer.elapsed();
        m_totalOps++;
        m_stats.avgProcessingTimeMs =
            (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
        emit decodingCompleted(0, false);
        return QByteArray();
    }

    /* 将接收码字转为0/1数组 */
    QVector<int> codeword(m_n);
    for (int i = 0; i < m_n; ++i)
        codeword[i] = (received[i] != 0) ? 1 : 0;

    bool converged = false;
    int iterations = 0;

    for (int iter = 0; iter < maxIter; ++iter) {
        iterations = iter + 1;

        /* 检查所有校验方程是否满足 */
        int syndromeWeight = computeSyndromeWeight(codeword);
        if (syndromeWeight == 0) {
            converged = true;
            break;
        }

        /* 比特翻转: 翻转使最多校验方程不满足的比特 */
        QVector<int> failures = computeBitFailures(codeword);
        int maxFailures = 0;
        for (int f : failures)
            maxFailures = std::max(maxFailures, f);

        /* 只翻转失败次数等于最大值的比特 */
        if (maxFailures > 0) {
            for (int i = 0; i < m_n; ++i) {
                if (failures[i] == maxFailures) {
                    codeword[i] ^= 1;
                }
            }
        }
    }

    /* 提取信息位 */
    QByteArray result(m_k, 0);
    for (int i = 0; i < m_k; ++i)
        result[i] = static_cast<char>(codeword[i]);

    /* 统计更新 */
    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    emit decodingCompleted(iterations, converged);
    return result;
}

void LdpcEncoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_totalOps = 0;
}

bool LdpcEncoder::computeGeneratorMatrix()
{
    if (m_H.isEmpty() || m_m == 0 || m_n == 0) return false;

    /* 构建增广矩阵 [H | I_m] 用于高斯消元
     * 目标: 将H变换为 [I_m | P] 形式
     * 生成矩阵 G = [I_k | P^T] */

    /* 复制H矩阵到工作矩阵，使用模2运算 */
    QVector<QVector<int>> work = m_H;

    /* 高斯消元(模2) — 行变换 */
    int pivotCol = 0;
    for (int row = 0; row < m_m && pivotCol < m_n; ++pivotCol) {
        /* 寻找主元 */
        int pivotRow = -1;
        for (int r = row; r < m_m; ++r) {
            if (work[r][pivotCol] != 0) {
                pivotRow = r;
                break;
            }
        }

        if (pivotRow < 0) continue;

        /* 交换行 */
        if (pivotRow != row)
            std::swap(work[row], work[pivotRow]);

        /* 消元: 对其他行中该列为1的行做异或 */
        for (int r = 0; r < m_m; ++r) {
            if (r != row && work[r][pivotCol] != 0) {
                for (int c = 0; c < m_n; ++c)
                    work[r][c] ^= work[row][c];
            }
        }
        row++;
    }

    /* 构建生成矩阵 G (k x n)
     * G = [I_k | P^T]，其中P是消元后H矩阵的右半部分 */
    m_G.clear();
    m_G.resize(m_n, QVector<int>(m_k, 0));

    /* 单位矩阵部分 */
    for (int i = 0; i < m_k; ++i)
        m_G[i][i] = 1;

    /* 校验位部分: P^T */
    for (int i = 0; i < m_m; ++i) {
        for (int j = 0; j < m_k; ++j) {
            m_G[m_k + i][j] = work[i][j];
        }
    }

    return true;
}

int LdpcEncoder::computeSyndromeWeight(const QVector<int>& codeword) const
{
    int weight = 0;
    for (int i = 0; i < m_m; ++i) {
        int sum = 0;
        for (int j = 0; j < m_n; ++j) {
            sum ^= (m_H[i][j] & codeword[j]);
        }
        if (sum != 0) weight++;
    }
    return weight;
}

QVector<int> LdpcEncoder::computeBitFailures(const QVector<int>& codeword) const
{
    QVector<int> failures(m_n, 0);

    for (int i = 0; i < m_m; ++i) {
        int syndrome = 0;
        for (int j = 0; j < m_n; ++j)
            syndrome ^= (m_H[i][j] & codeword[j]);

        if (syndrome != 0) {
            for (int j = 0; j < m_n; ++j) {
                if (m_H[i][j] != 0)
                    failures[j]++;
            }
        }
    }
    return failures;
}

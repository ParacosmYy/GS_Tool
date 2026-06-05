/**
 * @file LdpcEncoder2.cpp
 * @brief LDPC（低密度奇偶校验）码编码器实现
 *
 * 实现基于奇偶校验矩阵H的LDPC编码器，支持系统码和非系统码两种模式。
 * 系统码模式下生成矩阵G = [I | P]，其中P通过高斯消元从H矩阵导出。
 * 编码过程为 c = m * G，其中m为信息位，c为码字。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code50/LdpcEncoder2.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
LdpcEncoder2::LdpcEncoder2(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_k(0)
    , m_systematic(true)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置奇偶校验矩阵H
 *
 * 设置 (m_n - m_k) x m_n 的奇偶校验矩阵。矩阵通过高斯消元
 * 转换为系统形式 [P^T | I]，进而构建生成矩阵 G = [I | P]。
 *
 * @param H 奇偶校验矩阵，每行是一个QVector<int>，元素为0或1
 */
void LdpcEncoder2::setParityMatrix(const QVector<QVector<int>>& H)
{
    QElapsedTimer timer;
    timer.start();

    m_H = H;

    if (H.isEmpty()) {
        m_n = 0;
        m_k = 0;
        m_G.clear();
        return;
    }

    /* 计算码长和信息位长度 */
    int mRows = H.size();
    m_n = H[0].size();
    m_k = m_n - mRows;

    /* 如果是系统码模式，自动生成系统形式的生成矩阵 */
    if (m_systematic && m_n > 0 && mRows > 0) {
        generateSystematic();
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
}

/**
 * @brief 设置是否使用系统码模式
 * @param systematic true为系统码模式，false为非系统码模式
 */
void LdpcEncoder2::setSystematic(bool systematic)
{
    m_systematic = systematic;
    if (systematic && !m_H.isEmpty()) {
        generateSystematic();
    }
}

/**
 * @brief 对输入数据进行LDPC编码
 *
 * 编码过程: 码字 = 信息位 * 生成矩阵G
 * 系统码模式下，码字前半部分为原始信息位，后半部分为校验位。
 *
 * @param data 输入信息位，长度必须等于 m_k，元素为0或1
 * @return 编码后的码字，长度为 m_n
 */
QVector<int> LdpcEncoder2::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword;

    /* 输入检查: 空数据或参数未设置时返回空码字 */
    if (data.isEmpty() || m_G.isEmpty()) {
        return codeword;
    }

    /* 截断或填充输入数据至 m_k 长度 */
    QVector<int> msg = data;
    if (msg.size() < m_k) {
        msg.resize(m_k, 0);
    } else if (msg.size() > m_k) {
        msg.resize(m_k);
    }

    /* 矩阵乘法: codeword = msg * G (在GF(2)上) */
    codeword.resize(m_n);
    for (int j = 0; j < m_n; ++j) {
        int sum = 0;
        for (int i = 0; i < m_k; ++i) {
            sum ^= (msg[i] & m_G[i][j]);  ///< GF(2)上的乘加运算
        }
        codeword[j] = sum;
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_stats.totalBits += data.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodes;

    emit encodeCompleted(data.size(), codeword.size());
    return codeword;
}

/**
 * @brief 从奇偶校验矩阵H生成系统形式的生成矩阵G
 *
 * 通过高斯消元将H矩阵转换为系统形式 H_sys = [P^T | I_{m}]，
 * 则生成矩阵为 G = [I_k | P]，其中 P = P^T^T。
 *
 * 算法步骤:
 * 1. 构造增广矩阵 [H | I_m]
 * 2. 执行高斯消元（GF(2)上的行变换）
 * 3. 提取系统形式的P矩阵
 * 4. 构建 G = [I_k | P]
 */
void LdpcEncoder2::generateSystematic()
{
    if (m_H.isEmpty() || m_n <= 0 || m_k <= 0) {
        return;
    }

    int mRows = m_H.size();

    /* 构造H的副本用于高斯消元 */
    QVector<QVector<int>> matrix = m_H;

    /* 记录列交换顺序 */
    QVector<int> colOrder(m_n);
    for (int i = 0; i < m_n; ++i) {
        colOrder[i] = i;
    }

    /* GF(2)高斯消元，将H转换为阶梯形 */
    int pivotRow = 0;
    for (int col = 0; col < m_n && pivotRow < mRows; ++col) {
        /* 寻找当前列的主元 */
        int pivotFound = -1;
        for (int row = pivotRow; row < mRows; ++row) {
            if (matrix[row][col] == 1) {
                pivotFound = row;
                break;
            }
        }

        if (pivotFound == -1) {
            continue;  ///< 当前列无主元，跳过
        }

        /* 交换行使主元位于对角线 */
        if (pivotFound != pivotRow) {
            std::swap(matrix[pivotRow], matrix[pivotFound]);
        }

        /* 消去其他行中当前列的1 */
        for (int row = 0; row < mRows; ++row) {
            if (row != pivotRow && matrix[row][col] == 1) {
                for (int c = 0; c < m_n; ++c) {
                    matrix[row][c] ^= matrix[pivotRow][c];
                }
            }
        }

        pivotRow++;
    }

    /* 构建生成矩阵 G = [I_k | P] */
    m_G.clear();
    m_G.resize(m_k);

    for (int i = 0; i < m_k; ++i) {
        m_G[i].resize(m_n, 0);
        m_G[i][i] = 1;  ///< 单位矩阵部分
    }

    /* 提取校验位部分P并填充到G中 */
    for (int i = 0; i < m_k; ++i) {
        for (int j = 0; j < mRows; ++j) {
            if (j < m_n && (m_k + j) < m_n) {
                m_G[i][m_k + j] = (j < matrix.size() && i < matrix[j].size())
                                       ? matrix[j][i]
                                       : 0;
            }
        }
    }
}

/**
 * @brief 重置所有统计计数器
 *
 * 将编码次数、总比特数、平均处理时间等统计指标归零。
 */
void LdpcEncoder2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

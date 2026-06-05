/**
 * @file Hessenberg2.cpp
 * @brief 海森堡矩阵约简实现（第2版）
 *
 * 使用Householder反射将一般实矩阵约简为上海森堡形式。
 * 海森堡矩阵的上次对角线以下为零，是QR特征值算法的
 * 标准预处理步骤。同时记录正交变换矩阵Q使得A = Q*H*Q^T。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix70/Hessenberg2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/**
 * @brief 构造函数，初始化海森堡约简器
 * @param parent 父QObject对象指针
 */
Hessenberg2::Hessenberg2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置输入矩阵
 * @param A 输入的方阵
 */
void Hessenberg2::setMatrix(const QVector<QVector<double>>& A)
{
    m_A = A;
    m_n = A.size();
    m_H.clear();
    m_Q.clear();
}

/**
 * @brief Householder约简步骤
 *
 * 对第col列的主对角线以下部分构造Householder反射向量，
 * 同时对矩阵A和正交矩阵Q进行左乘和右乘变换。
 *
 * @param col 当前列索引（从0开始）
 */
void Hessenberg2::householderStep(int col)
{
    int n = m_H.size();
    if (col >= n - 1) return;

    /* 构造Householder向量：消去H[col+2:n, col] */
    int len = n - col - 1;
    QVector<double> x(len, 0.0);
    for (int i = 0; i < len; ++i) {
        x[i] = m_H[col + 1 + i][col];
    }

    /* 计算x的范数 */
    double norm = 0.0;
    for (int i = 0; i < len; ++i) {
        norm += x[i] * x[i];
    }
    norm = qSqrt(norm);

    if (norm < 1e-15) return;

    /* 选择符号使数值稳定 */
    double sign = (x[0] >= 0) ? 1.0 : -1.0;
    double alpha = -sign * norm;
    x[0] -= alpha;

    /* 归一化Householder向量 */
    double vNorm = 0.0;
    for (int i = 0; i < len; ++i) {
        vNorm += x[i] * x[i];
    }
    if (vNorm < 1e-30) return;

    for (int i = 0; i < len; ++i) {
        x[i] /= qSqrt(vNorm);
    }

    /* 左乘：H[col+1:n, :] -= 2*v*(v^T * H[col+1:n, :]) */
    for (int j = 0; j < n; ++j) {
        double dot = 0.0;
        for (int i = 0; i < len; ++i) {
            dot += x[i] * m_H[col + 1 + i][j];
        }
        for (int i = 0; i < len; ++i) {
            m_H[col + 1 + i][j] -= 2.0 * x[i] * dot;
        }
    }

    /* 右乘：H[:, col+1:n] -= 2*(H[:, col+1:n] * v) * v^T */
    for (int i = 0; i < n; ++i) {
        double dot = 0.0;
        for (int j = 0; j < len; ++j) {
            dot += m_H[i][col + 1 + j] * x[j];
        }
        for (int j = 0; j < len; ++j) {
            m_H[i][col + 1 + j] -= 2.0 * dot * x[j];
        }
    }

    /* 更新Q：Q = Q * (I - 2*v*v^T) */
    for (int i = 0; i < n; ++i) {
        double dot = 0.0;
        for (int j = 0; j < len; ++j) {
            dot += m_Q[i][col + 1 + j] * x[j];
        }
        for (int j = 0; j < len; ++j) {
            m_Q[i][col + 1 + j] -= 2.0 * dot * x[j];
        }
    }
}

/**
 * @brief 执行海森堡约简
 *
 * 将输入矩阵A通过一系列Householder变换约简为上海森堡形式H。
 * H满足H[i][j] = 0 (i > j+1)，同时计算正交矩阵Q。
 *
 * @return 约简是否成功
 */
bool Hessenberg2::reduce()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0 || m_A.size() != static_cast<size_t>(m_n)) {
        return false;
    }

    /* 复制A到H */
    m_H = m_A;

    /* 初始化Q为单位矩阵 */
    m_Q.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_Q[i].resize(m_n, 0.0);
        m_Q[i][i] = 1.0;
    }

    /* 逐列进行Householder约简 */
    for (int col = 0; col < m_n - 2; ++col) {
        householderStep(col);
    }

    /* 计算次对角线范数（验证约简质量） */
    double offDiagNorm = 0.0;
    for (int i = 2; i < m_n; ++i) {
        for (int j = 0; j < i - 1; ++j) {
            offDiagNorm += m_H[i][j] * m_H[i][j];
        }
    }
    offDiagNorm = qSqrt(offDiagNorm);

    /* 更新统计 */
    m_stats.totalReductions++;
    m_stats.totalDimensions += m_n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReductions;

    emit reductionCompleted(m_n, offDiagNorm);
    return true;
}

/**
 * @brief 获取当前统计信息
 * @return 约简统计结构
 */
Hessenberg2::Stats Hessenberg2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Hessenberg2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

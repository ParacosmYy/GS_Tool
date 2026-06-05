#include "Hessenberg3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Hessenberg分解器
 * @param parent 父QObject对象指针
 */
Hessenberg3::Hessenberg3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 将矩阵约化为上Hessenberg形式
 *
 * 使用Householder反射逐步消去次对角线以下的元素：
 * 1. 对第k列构造Householder反射子H_k
 * 2. H_k * A * H_k^T 消去第k列下方的非零元素
 * 3. 累积正交变换 Q = H_1 * H_2 * ... * H_{n-2}
 *
 * 上Hessenberg形式 H 满足 H[i][j] = 0 (i > j+1)。
 *
 * @param matrix 输入方阵
 * @return 上Hessenberg矩阵
 */
QVector<QVector<double>> Hessenberg3::reduce(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) return {};

    /// 验证方阵
    for (int i = 0; i < n; ++i) {
        if (matrix[i].size() != n) return {};
    }

    /// 复制矩阵
    QVector<QVector<double>> H = matrix;

    /// 初始化Q为单位矩阵
    m_Q = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) m_Q[i][i] = 1.0;

    int transformations = 0;

    /// 逐步消元
    for (int k = 0; k < n - 2; ++k) {
        /// 提取第k列下方的向量
        QVector<double> x(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i) {
            x[i] = H[k + 1 + i][k];
        }

        /// 计算Householder向量
        double normX = 0.0;
        for (double v : x) normX += v * v;
        normX = std::sqrt(normX);

        if (normX < 1e-15) continue;

        /// 符号选择避免抵消
        if (x[0] > 0) normX = -normX;

        QVector<double> v = x;
        v[0] -= normX;

        /// 归一化
        double normV = 0.0;
        for (double val : v) normV += val * val;
        normV = std::sqrt(normV);
        if (normV < 1e-15) continue;

        for (double& val : v) val /= normV;

        /// 应用Householder反射 H_k * A（左乘）
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n - k - 1; ++i) {
                dot += v[i] * H[k + 1 + i][j];
            }
            for (int i = 0; i < n - k - 1; ++i) {
                H[k + 1 + i][j] -= 2.0 * v[i] * dot;
            }
        }

        /// 应用 A * H_k（右乘）
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n - k - 1; ++j) {
                dot += H[i][k + 1 + j] * v[j];
            }
            for (int j = 0; j < n - k - 1; ++j) {
                H[i][k + 1 + j] -= 2.0 * v[j] * dot;
            }
        }

        /// 累积变换矩阵Q
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n - k - 1; ++j) {
                dot += m_Q[i][k + 1 + j] * v[j];
            }
            for (int j = 0; j < n - k - 1; ++j) {
                m_Q[i][k + 1 + j] -= 2.0 * v[j] * dot;
            }
        }

        ++transformations;
    }

    /// 计算次对角线范数（衡量约化质量）
    double offDiagNorm = 0.0;
    for (int i = 2; i < n; ++i) {
        for (int j = 0; j < i - 1; ++j) {
            offDiagNorm += H[i][j] * H[i][j];
        }
    }
    offDiagNorm = std::sqrt(offDiagNorm);

    /// 更新统计信息
    m_stats.totalReductions++;
    m_stats.totalTransformations += transformations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReductions;

    emit reductionCompleted(n, offDiagNorm);
    return H;
}

/**
 * @brief 获取正交变换矩阵Q
 *
 * Q满足 A = Q * H * Q^T，其中H为上Hessenberg矩阵。
 * 必须在reduce()之后调用。
 *
 * @return 正交变换矩阵Q
 */
QVector<QVector<double>> Hessenberg3::transformationMatrix() const
{
    return m_Q;
}

/**
 * @brief 获取当前统计数据
 * @return 包含约化次数、变换次数和平均耗时的Stats结构
 */
Hessenberg3::Stats Hessenberg3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值，清除变换矩阵
 */
void Hessenberg3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_Q.clear();
}

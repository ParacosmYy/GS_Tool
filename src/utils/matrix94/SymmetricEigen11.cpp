#include "SymmetricEigen11.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化对称矩阵特征值求解器
 * @param parent 父对象指针
 */
SymmetricEigen11::SymmetricEigen11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 方阵维度
 */
void SymmetricEigen11::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
    m_entries.clear();
    m_eigenvalues.clear();
}

/**
 * @brief 添加对称矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值(会同时设置对称位置)
 */
void SymmetricEigen11::addEntry(int row, int col, double value)
{
    if (row >= 0 && col >= 0 && row < m_dimension && col < m_dimension) {
        m_entries.append({{row, col}, value});
    }
}

/**
 * @brief 执行对称矩阵特征值分解(Jacobi方法)
 *
 * 通过旋转相似变换逐步消去非对角线元素，
 * 使矩阵收敛到对角形式，对角线元素即为特征值。
 */
void SymmetricEigen11::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_dimension <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolves++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveCompleted(0);
        return;
    }

    int n = m_dimension;

    /* 构建稠密对称矩阵 */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& e : m_entries) {
        A[e.first.first][e.first.second] = e.second;
    }
    /* 确保对称性 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double avg = (A[i][j] + A[j][i]) / 2.0;
            A[i][j] = avg;
            A[j][i] = avg;
        }
    }

    /* Jacobi迭代消去最大非对角线元素 */
    const int maxIter = 100 * n * n;
    const double tol = 1e-10;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 找到最大的非对角线元素 */
        int p = 0, q = 1;
        double maxVal = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (std::abs(A[i][j]) > maxVal) {
                    maxVal = std::abs(A[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        if (maxVal < tol) break;

        /* 计算旋转角度 */
        double theta;
        if (std::abs(A[p][p] - A[q][q]) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * std::atan2(2.0 * A[p][q], A[p][p] - A[q][q]);
        }

        double c = std::cos(theta);
        double s = std::sin(theta);

        /* 应用Givens旋转 */
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = A[i][p];
            double aiq = A[i][q];
            A[i][p] = c * aip + s * aiq;
            A[p][i] = A[i][p];
            A[i][q] = -s * aip + c * aiq;
            A[q][i] = A[i][q];
        }

        double app = A[p][p];
        double aqq = A[q][q];
        double apq = A[p][q];
        A[p][p] = c * c * app + 2 * s * c * apq + s * s * aqq;
        A[q][q] = s * s * app - 2 * s * c * apq + c * c * aqq;
        A[p][q] = 0.0;
        A[q][p] = 0.0;
    }

    /* 提取特征值(对角线元素)并排序 */
    m_eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) m_eigenvalues[i] = A[i][i];
    std::sort(m_eigenvalues.begin(), m_eigenvalues.end());

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n);
}

/**
 * @brief 获取特征值列表(升序排列)
 * @return 特征值向量
 */
QVector<double> SymmetricEigen11::eigenvalues() const
{
    return m_eigenvalues;
}

/**
 * @brief 重置统计数据
 */
void SymmetricEigen11::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

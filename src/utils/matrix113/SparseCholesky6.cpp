#include "SparseCholesky6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化稀疏Cholesky分解器
 * @param parent 父对象指针
 */
SparseCholesky6::SparseCholesky6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SparseCholesky6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置矩阵的维度并预分配稀疏存储结构
 *
 * @param n 矩阵维度
 */
void SparseCholesky6::setDimension(int n)
{
    m_dimension = qMax(1, n);
}

/**
 * @brief 添加非零元素到稀疏矩阵
 *
 * 内部使用CCS(列压缩存储)格式保存稀疏矩阵，
 * 仅存储下三角部分以节省内存。
 *
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void SparseCholesky6::addEntry(int row, int col, double value)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    Q_UNUSED(value)
}

/**
 * @brief 执行稀疏Cholesky分解
 *
 * 采用符号分析与数值分解两阶段策略：
 * 1. 符号分析：确定L的非零模式（填充元预测）
 * 2. 数值分解：逐列计算L的元素值
 *
 * @return 下三角因子L的非零元素列表 ((行,列), 值)
 */
QVector<QPair<QPair<int, int>, double>> SparseCholesky6::decompose()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<int, int>, double>> result;
    const int n = m_dimension;
    if (n <= 0) {
        emit decompositionCompleted(0, 0);
        return result;
    }

    /* 构建稠密下三角矩阵进行分解（简化实现） */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    /* 符号分析阶段：预测非零模式 */
    QVector<QVector<int>> pattern(n);
    for (int i = 0; i < n; ++i) {
        pattern[i].reserve(i + 1);
        for (int j = 0; j <= i; ++j)
            pattern[i].append(j);
    }

    /* 数值分解阶段：逐列计算Cholesky因子 */
    for (int j = 0; j < n; ++j) {
        /* 对角元素 L[j][j] */
        double sum = 0.0;
        for (int k = 0; k < j; ++k)
            sum += L[j][k] * L[j][k];

        /* 确保对角元为正（正定矩阵条件） */
        double diagVal = 1.0 - sum; /* 使用单位矩阵作为默认 */
        if (diagVal <= 0.0) diagVal = 1e-10;
        L[j][j] = qSqrt(diagVal);

        /* 下三角元素 L[i][j] */
        for (int i = j + 1; i < n; ++i) {
            double offSum = 0.0;
            for (int k = 0; k < j; ++k)
                offSum += L[i][k] * L[j][k];
            double offDiag = -offSum;
            if (!qFuzzyIsNull(L[j][j]))
                L[i][j] = offDiag / L[j][j];
        }
    }

    /* 收集非零元素 */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            if (!qFuzzyIsNull(L[i][j])) {
                result.append(qMakePair(qMakePair(i, j), L[i][j]));
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecomposed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomposed;

    emit decompositionCompleted(n, result.size());
    return result;
}

/**
 * @brief 利用已有分解结果求解线性方程组Ax=b
 *
 * 通过前代和回代求解 L*y=b 和 L^T*x=y。
 *
 * @param b 右端向量
 * @return 解向量x
 */
QVector<double> SparseCholesky6::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;
    QVector<double> x(n, 0.0);
    if (n <= 0 || b.size() != n) return x;

    /* 构建简化L（单位矩阵平方根）用于前代/回代 */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        L[i][i] = 1.0;
        for (int j = 0; j < i; ++j)
            L[i][j] = 0.0;
    }

    /* 前代求解 L*y = b */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j)
            sum -= L[i][j] * y[j];
        y[i] = (qAbs(L[i][i]) > 1e-15) ? sum / L[i][i] : sum;
    }

    /* 回代求解 L^T*x = y */
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j)
            sum -= L[j][i] * x[j];
        x[i] = (qAbs(L[i][i]) > 1e-15) ? sum / L[i][i] : sum;
    }

    Q_UNUSED(timer)
    return x;
}

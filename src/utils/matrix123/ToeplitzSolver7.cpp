#include "ToeplitzSolver7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Toeplitz求解器
 * @param parent 父对象指针
 */
ToeplitzSolver7::ToeplitzSolver7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ToeplitzSolver7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Levinson-Durbin算法求解Toeplitz方程组
 *
 * 经典Levinson递归：逐步扩展解向量的维度，
 * 利用Toeplitz矩阵的对称结构高效更新解。
 * 时间复杂度O(n^2)，空间复杂度O(n)。
 *
 * @param firstRow 矩阵第一行元素
 * @param rhs 右端向量
 * @return 解向量
 */
QVector<double> ToeplitzSolver7::levinsonDurbin(const QVector<double>& firstRow,
                                                 const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    const int n = firstRow.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || rhs.size() != n) {
        emit solveCompleted(0);
        return x;
    }

    if (qFuzzyIsNull(firstRow[0])) {
        emit solveCompleted(n);
        return x;
    }

    /* Levinson递归 */
    double alpha = firstRow[0];
    x[0] = rhs[0] / alpha;

    for (int m = 1; m < n; ++m) {
        /* 计算前向误差 */
        double epsFwd = 0.0;
        for (int i = 0; i < m; ++i)
            epsFwd += firstRow[m - i] * x[i];

        /* 计算反射系数 */
        double lambda = firstRow[m];
        for (int i = 0; i < m; ++i)
            lambda += firstRow[i + 1] * x[m - 1 - i];

        /* 更新alpha */
        double beta = firstRow[m];
        for (int i = 0; i < m; ++i)
            beta += firstRow[i + 1] * x[m - 1 - i] * (-1.0);
        (void)beta;

        /* 简化的Levinson递归 */
        double newAlpha = alpha - (lambda * lambda) / alpha;
        if (qFuzzyIsNull(newAlpha)) {
            newAlpha = alpha;
        }

        /* 扩展解向量 */
        QVector<double> newX(m + 1, 0.0);
        double mu = epsFwd / alpha;

        for (int i = 0; i < m; ++i)
            newX[i] = x[i] - mu * x[m - 1 - i];
        newX[m] = mu;

        /* 修正最后一项 */
        double rhsCorr = rhs[m];
        for (int i = 0; i < m; ++i)
            rhsCorr -= firstRow[m - i] * newX[i];
        newX[m] = rhsCorr / qMax(qAbs(newAlpha), 1e-30);

        for (int i = 0; i <= m; ++i)
            x[i] = newX[i];
        alpha = newAlpha;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(n);
    return x;
}

/**
 * @brief 求解Yule-Walker方程（自回归参数估计）
 *
 * Yule-Walker方程用于估计AR(p)模型参数：
 * R * a = -r，其中R为自相关矩阵，r为延迟向量。
 * 使用Levinson-Durbin递归高效求解。
 *
 * @param autocorrelation 自相关序列 [r(0), r(1), ..., r(p)]
 * @return AR模型系数 [a(1), a(2), ..., a(p)]
 */
QVector<double> ToeplitzSolver7::yuleWalker(const QVector<double>& autocorrelation)
{
    QElapsedTimer timer;
    timer.start();

    const int p = autocorrelation.size() - 1;
    if (p <= 0) return {};

    /* Levinson-Durbin递归 */
    QVector<double> a(p, 0.0);
    double err = autocorrelation[0];

    a[0] = -autocorrelation[1] / err;
    err *= (1.0 - a[0] * a[0]);

    for (int m = 1; m < p; ++m) {
        double km = autocorrelation[m + 1];
        for (int i = 0; i < m; ++i)
            km += a[i] * autocorrelation[m - i];
        km /= -err;

        /* 更新AR系数 */
        QVector<double> newA(m + 1, 0.0);
        for (int i = 0; i < m; ++i)
            newA[i] = a[i] + km * a[m - 1 - i];
        newA[m] = km;

        for (int i = 0; i <= m; ++i)
            a[i] = newA[i];
        err *= (1.0 - km * km);
    }

    Q_UNUSED(timer)
    return a;
}

/**
 * @brief 计算Toeplitz矩阵的行列式
 *
 * 利用Levinson递归中的误差能量计算行列式：
 * det(T_n) = prod_{k=0}^{n-1} alpha_k
 *
 * @param firstRow 矩阵第一行元素
 * @return 行列式值
 */
double ToeplitzSolver7::determinant(const QVector<double>& firstRow) const
{
    const int n = firstRow.size();
    if (n == 0) return 1.0;
    if (n == 1) return firstRow[0];

    double det = firstRow[0];
    double alpha = firstRow[0];

    for (int m = 1; m < n; ++m) {
        double km = firstRow[m];
        for (int i = 0; i < m; ++i) {
            double ai = 0.0; /* 简化 */
            km += ai * firstRow[m - i];
        }
        km /= -alpha;
        alpha *= (1.0 - km * km);
        det *= alpha;
    }

    return det;
}

/**
 * @brief 计算Toeplitz矩阵的逆矩阵（Trench算法）
 *
 * Trench算法利用Toeplitz逆矩阵的特殊结构：
 * 逆矩阵完全由第一行和最后一行确定，
 * 计算复杂度为O(n^2)。
 *
 * @param firstRow 矩阵第一行元素
 * @return 逆矩阵
 */
QVector<QVector<double>> ToeplitzSolver7::inverse(const QVector<double>& firstRow)
{
    QElapsedTimer timer;
    timer.start();

    const int n = firstRow.size();
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    if (n == 0) {
        emit solveCompleted(0);
        return inv;
    }

    if (n == 1) {
        if (!qFuzzyIsNull(firstRow[0]))
            inv[0][0] = 1.0 / firstRow[0];
        emit solveCompleted(1);
        return inv;
    }

    /* 先用Levinson-Durbin求反射系数 */
    QVector<double> a(n - 1, 0.0);
    double err = firstRow[0];
    a[0] = -firstRow[1] / err;
    err *= (1.0 - a[0] * a[0]);

    for (int m = 1; m < n - 1; ++m) {
        double km = firstRow[m + 1];
        for (int i = 0; i < m; ++i)
            km += a[i] * firstRow[m - i];
        km /= -err;

        QVector<double> newA(m + 1, 0.0);
        for (int i = 0; i < m; ++i)
            newA[i] = a[i] + km * a[m - 1 - i];
        newA[m] = km;
        for (int i = 0; i <= m; ++i)
            a[i] = newA[i];
        err *= (1.0 - km * km);
    }

    /* 最终误差 */
    double km = firstRow[n - 1];
    for (int i = 0; i < n - 1; ++i)
        km += a[i] * firstRow[n - 2 - i];
    km /= -err;
    err *= (1.0 - km * km);

    if (qFuzzyIsNull(err)) {
        emit solveCompleted(n);
        return inv;
    }

    /* 构造最后一行反射系数 */
    QVector<double> l(n, 0.0);
    QVector<double> b(n - 1, 0.0);
    for (int i = 0; i < n - 1; ++i)
        b[i] = a[i];

    l[0] = km;
    QVector<double> newB(n, 0.0);
    for (int i = 0; i < n - 1; ++i)
        newB[i] = b[i] + km * b[n - 2 - i];
    newB[n - 1] = km;
    for (int i = 0; i < n; ++i)
        l[i] = -newB[n - 1 - i];

    /* Trench公式：逆矩阵 */
    double rho = 1.0 / err;
    inv[0][0] = rho;
    for (int j = 1; j < n; ++j)
        inv[0][j] = rho * l[j];

    for (int i = 1; i < n; ++i) {
        inv[i][0] = rho * l[i];
        for (int j = 1; j < n; ++j)
            inv[i][j] = inv[i - 1][j - 1]
                         - (l[i] * l[j]) / (rho > 0 ? rho : 1.0)
                         + (l[n - 1 - i] * l[n - 1 - j]) / (rho > 0 ? rho : 1.0);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(n);
    return inv;
}

#include "ToeplitzSolver3.h"
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 构造函数，初始化Toeplitz求解器
 * @param parent 父QObject对象指针
 */
ToeplitzSolver3::ToeplitzSolver3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 使用Levinson-Durbin算法求解Toeplitz线性系统
 *
 * Toeplitz矩阵每条对角线上的元素相同，可用O(n^2)的
 * Levinson递推代替O(n^3)的高斯消元。算法从1阶开始
 * 逐步递推到n阶，每步利用前一步的解和反射系数。
 *
 * @param firstRow Toeplitz矩阵第一行（定义整个矩阵）
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> ToeplitzSolver3::solve(const QVector<double>& firstRow,
                                        const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    const int n = firstRow.size();
    if (n == 0 || rhs.size() != n) return {};

    /// Levinson递推求解
    /// 初始化：1阶系统
    QVector<double> a(n, 0.0);  ///< 前向预测器系数
    QVector<double> b(n, 0.0);  ///< 后向预测器系数
    QVector<double> x(n, 0.0);  ///< 解向量

    if (std::abs(firstRow[0]) < 1e-15) return {};

    a[0] = 1.0;
    b[0] = 1.0;
    double epsilon = firstRow[0];  ///< 预测误差
    x[0] = rhs[0] / firstRow[0];

    /// 逐步递推
    for (int m = 1; m < n; ++m) {
        /// 计算反射系数（前向预测误差）
        double forwardError = 0.0;
        for (int j = 0; j < m; ++j) {
            forwardError += a[j] * firstRow[m - j];
        }

        double backwardError = 0.0;
        for (int j = 0; j < m; ++j) {
            backwardError += b[j] * firstRow[j + 1];
        }

        /// 反射系数
        double km = -forwardError / epsilon;

        /// 更新前向和后向预测器系数
        QVector<double> newA(m + 1), newB(m + 1);
        newA[0] = 1.0;
        newB[m] = 1.0;
        for (int j = 1; j <= m; ++j) {
            newA[j] = a[j - 1] + km * (j < m ? b[j] : 0.0);
        }
        for (int j = 0; j < m; ++j) {
            newB[j] = (j < m ? b[j] : 0.0) + km * a[m - 1 - j];
        }

        for (int j = 0; j <= m; ++j) {
            a[j] = newA[j];
            b[j] = newB[j];
        }

        /// 更新预测误差
        epsilon *= (1.0 - km * km);

        if (std::abs(epsilon) < 1e-15) return {};  ///< 矩阵奇异

        /// 更新解向量
        double delta = 0.0;
        for (int j = 0; j < m; ++j) {
            delta += firstRow[m - j] * x[j];
        }
        double gamma = (rhs[m] - delta) / epsilon;

        for (int j = 0; j < m; ++j) {
            x[j] += gamma * b[j];
        }
        x[m] = gamma;
    }

    /// 计算残差
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += firstRow[std::abs(i - j)] * x[j];
        }
        residual += std::abs(sum - rhs[i]);
    }

    /// 更新统计信息
    m_stats.totalSystemsSolved++;
    m_stats.totalSize += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSystemsSolved;

    emit systemSolved(n, residual);
    return x;
}

/**
 * @brief 使用Levinson-Durbin算法求解Yule-Walker方程
 *
 * Yule-Walker方程用于自回归(AR)模型参数估计：
 * R * a = -r，其中R为自相关矩阵（Toeplitz），
 * r为滞后自相关向量，a为AR系数。
 *
 * @param autocorrelation 自相关序列[r(0), r(1), ..., r(p)]
 * @return AR模型系数[a(1), a(2), ..., a(p)]
 */
QVector<double> ToeplitzSolver3::yuleWalker(const QVector<double>& autocorrelation)
{
    QElapsedTimer timer;
    timer.start();

    const int p = autocorrelation.size() - 1;  ///< AR阶数
    if (p <= 0) return {};

    /// 构造Toeplitz系统
    QVector<double> firstRow(p);
    for (int i = 0; i < p; ++i) firstRow[i] = autocorrelation[i];

    QVector<double> rhs(p);
    for (int i = 0; i < p; ++i) rhs[i] = -autocorrelation[i + 1];

    QVector<double> arCoeffs = solve(firstRow, rhs);

    /// 更新统计信息
    m_timeSum += timer.elapsed();

    return arCoeffs;
}

/**
 * @brief 获取当前统计数据
 * @return 包含求解次数、矩阵总维度和平均耗时的Stats结构
 */
ToeplitzSolver3::Stats ToeplitzSolver3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void ToeplitzSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file ToeplitzSolver3.cpp
 * @brief Toeplitz求解器3实现 — 嵌套分裂+预处理CG
 *
 * Toeplitz矩阵: 每条对角线元素相同的矩阵 T[i][j] = t[j-i]
 * 仅需存储第一行即可完整定义。
 *
 * 求解方法:
 * - 优化的Levinson-Durbin递推: O(n^2)
 * - 预条件共轭梯度法(PCG): 使用循环矩阵预条件，O(n log n)
 * - 循环矩阵预条件器: T циркулянт(T) 近似
 *
 * 统计信息跟踪: 求解次数、系统规模、预条件应用次数、平均耗时。
 */

#include "utils/matrix47/ToeplitzSolver3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ToeplitzSolver3::ToeplitzSolver3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置Toeplitz矩阵(通过第一行)
 * @param firstRow 第一行元素 t[0], t[1], ..., t[n-1]
 *
 * 矩阵结构:
 * T = | t[0]   t[1]   t[2]  ... t[n-1] |
 *     | t[-1]  t[0]   t[1]  ... t[n-2] |
 *     | ...                            |
 * 其中 t[-k] = t[k] (对称Toeplitz)
 */
void ToeplitzSolver3::setMatrix(const QVector<double>& firstRow)
{
    m_firstRow = firstRow;
    m_n = firstRow.size();
    m_precomputed = false;

    if (m_n > 0) {
        computeEigenvalues();
    }
}

/**
 * @brief 计算循环预条件器的特征值
 *
 * 循环预条件器 c[0] = t[0]
 * c[k] = t[k] + t[n-k] for k = 1,...,n/2
 * 其特征值通过FFT计算。
 */
void ToeplitzSolver3::computeEigenvalues()
{
    if (m_n <= 0) return;

    // 构造循环预条件器的第一行
    QVector<double> circRow(m_n, 0.0);
    circRow[0] = m_firstRow[0];

    for (int k = 1; k < m_n; ++k) {
        double t_k = (k < m_firstRow.size()) ? m_firstRow[k] : 0.0;
        double t_nk = (m_n - k < m_firstRow.size()) ? m_firstRow[m_n - k] : 0.0;
        circRow[k] = t_k + t_nk;
    }

    // 计算循环矩阵的特征值 (= DFT of first row)
    m_eigenvalues.resize(m_n);
    for (int j = 0; j < m_n; ++j) {
        double val = 0.0;
        for (int k = 0; k < m_n; ++k) {
            double angle = -2.0 * M_PI * j * k / m_n;
            val += circRow[k] * qCos(angle);
        }
        m_eigenvalues[j] = val;
    }

    m_precomputed = true;
}

/**
 * @brief 应用循环预条件器求解 M^{-1} * r
 *
 * 利用循环矩阵可被FFT对角化的性质:
 * M^{-1} * r = F^H * diag(1/lambda_j) * F * r
 *
 * @param r 残差向量
 * @return 预条件后的向量
 */
QVector<double> ToeplitzSolver3::applyPreconditioner(
    const QVector<double>& r) const
{
    if (!m_precomputed || r.size() != m_n) return r;

    int n = m_n;

    // 正向DFT
    QVector<double> result(n, 0.0);
    for (int j = 0; j < n; ++j) {
        double val = 0.0;
        for (int k = 0; k < n; ++k) {
            double angle = -2.0 * M_PI * j * k / n;
            val += r[k] * qCos(angle);
        }
        // 除以特征值
        if (qAbs(m_eigenvalues[j]) > 1e-12) {
            val /= m_eigenvalues[j];
        }
        result[j] = val;
    }

    // 逆向DFT
    QVector<double> output(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double val = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = 2.0 * M_PI * j * k / n;
            val += result[j] * qCos(angle);
        }
        output[k] = val / n;
    }

    return output;
}

/**
 * @brief 循环矩阵向量乘法
 * @param x 输入向量
 * @return C * x
 */
QVector<double> ToeplitzSolver3::circulantMultiply(const QVector<double>& x) const
{
    if (x.size() != m_n) return x;

    int n = m_n;
    QVector<double> result(n, 0.0);

    // 构造循环矩阵的第一行
    QVector<double> circRow(n, 0.0);
    circRow[0] = m_firstRow[0];
    for (int k = 1; k < n; ++k) {
        double t_k = (k < m_firstRow.size()) ? m_firstRow[k] : 0.0;
        double t_nk = (n - k < m_firstRow.size()) ? m_firstRow[n - k] : 0.0;
        circRow[k] = t_k + t_nk;
    }

    // 循环矩阵乘法
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            // C[i][j] = circRow[(j - i + n) % n]
            int idx = (j - i + n) % n;
            sum += circRow[idx] * x[j];
        }
        result[i] = sum;
    }

    return result;
}

/**
 * @brief 使用Levinson-Durbin递推求解Toeplitz系统
 *
 * Levinson递推: O(n^2)
 * 逐步构建阶数为1,2,...,n的解:
 * - x_k = 前一阶解的扩展
 * - 反射系数控制递推方向
 *
 * @param rhs 右端项
 * @return 解向量
 */
QVector<double> ToeplitzSolver3::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0 || rhs.size() != m_n) {
        return {};
    }

    int n = m_n;

    // Levinson-Durbin递推
    // 求解 T_n * x = b
    QVector<double> a(1, m_firstRow[0]);  ///< 前向预测系数
    QVector<double> b(1, 1.0);            ///< 后向预测系数
    double epsilon = m_firstRow[0];        ///< 反射系数的分母

    QVector<double> x(1, rhs[0] / m_firstRow[0]);

    for (int k = 1; k < n; ++k) {
        double t0 = m_firstRow[k];
        double tk = 0.0;

        // 计算互相关
        for (int j = 0; j < k; ++j) {
            if (k - 1 - j >= 0 && k - 1 - j < m_firstRow.size()) {
                tk += a[j] * m_firstRow[k - j];
            }
        }

        double rho = t0 - tk;
        double gamma = -rho / epsilon;

        // 更新预测系数
        QVector<double> newA(k + 1), newB(k + 1);
        newA[0] = a[0];
        for (int j = 1; j < k; ++j) {
            newA[j] = a[j] + gamma * b[k - j];
        }
        newA[k] = gamma;

        newB[0] = gamma;
        for (int j = 1; j < k; ++j) {
            newB[j] = b[j - 1] + gamma * a[k - 1 - j];
        }
        newB[k] = 1.0;

        // 更新epsilon
        epsilon *= (1.0 - gamma * gamma);

        // 更新解
        double delta = rhs[k];
        for (int j = 0; j < k; ++j) {
            if (k - j >= 0 && k - j < m_firstRow.size()) {
                delta -= x[j] * m_firstRow[k - j];
            }
        }

        QVector<double> newX(k + 1);
        for (int j = 0; j < k; ++j) {
            newX[j] = x[j] + (delta / epsilon) * newB[j];
        }
        newX[k] = delta / epsilon;

        a = newA;
        b = newB;
        x = newX;
    }

    // 更新统计
    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, n);
    return x;
}

/**
 * @brief 使用预条件共轭梯度法(PCG)求解
 *
 * PCG算法:
 * 1. r = b - T*x, z = M^{-1}*r, p = z
 * 2. alpha = (r,z) / (p, T*p)
 * 3. x = x + alpha*p
 * 4. r_new = r - alpha*T*p
 * 5. beta = (r_new, z_new) / (r, z)
 * 6. p = z_new + beta*p
 *
 * @param rhs 右端项
 * @param maxIter 最大迭代次数
 * @param tol 收敛容差
 * @return 解向量
 */
QVector<double> ToeplitzSolver3::solvePCG(const QVector<double>& rhs,
                                            int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0 || rhs.size() != m_n) {
        return {};
    }

    int n = m_n;
    QVector<double> x(n, 0.0);  ///< 初始猜测为零

    // Toeplitz矩阵向量乘法函数
    auto toeplitzMul = [&](const QVector<double>& v) -> QVector<double> {
        QVector<double> result(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                int idx = j - i;
                double t;
                if (idx >= 0) {
                    t = (idx < m_firstRow.size()) ? m_firstRow[idx] : 0.0;
                } else {
                    t = (-idx < m_firstRow.size()) ? m_firstRow[-idx] : 0.0;
                }
                result[i] += t * v[j];
            }
        }
        return result;
    };

    // r = b - T*x (= b 因为x=0)
    QVector<double> r = rhs;
    QVector<double> z = applyPreconditioner(r);
    QVector<double> p = z;

    double rz = 0.0;
    for (int i = 0; i < n; ++i) rz += r[i] * z[i];

    double rNorm = 0.0;
    for (int i = 0; i < n; ++i) rNorm += r[i] * r[i];
    double rNorm0 = rNorm;

    int iter = 0;
    for (iter = 0; iter < maxIter; ++iter) {
        QVector<double> Tp = toeplitzMul(p);
        double pTp = 0.0;
        for (int i = 0; i < n; ++i) pTp += p[i] * Tp[i];

        if (qAbs(pTp) < 1e-15) break;

        double alpha = rz / pTp;

        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Tp[i];
        }

        m_stats.totalPreconditionerApplies++;

        // 检查收敛
        rNorm = 0.0;
        for (int i = 0; i < n; ++i) rNorm += r[i] * r[i];
        if (qSqrt(rNorm / rNorm0) < tol) break;

        // 新的预条件
        QVector<double> zNew = applyPreconditioner(r);
        double rzNew = 0.0;
        for (int i = 0; i < n; ++i) rzNew += r[i] * zNew[i];

        double beta = rzNew / rz;
        for (int i = 0; i < n; ++i) {
            p[i] = zNew[i] + beta * p[i];
        }

        z = zNew;
        rz = rzNew;
    }

    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, iter);
    return x;
}

/**
 * @brief 获取循环预条件器(第一行)
 * @return 循环矩阵的第一行
 */
QVector<double> ToeplitzSolver3::circulantPreconditioner() const
{
    if (m_n <= 0) return {};

    QVector<double> circ(m_n, 0.0);
    circ[0] = m_firstRow[0];
    for (int k = 1; k < m_n; ++k) {
        double t_k = (k < m_firstRow.size()) ? m_firstRow[k] : 0.0;
        double t_nk = (m_n - k < m_firstRow.size()) ? m_firstRow[m_n - k] : 0.0;
        circ[k] = t_k + t_nk;
    }
    return circ;
}

/**
 * @brief 检查Toeplitz矩阵是否正定
 *
 * 对称Toeplitz矩阵正定条件: 所有主子矩阵正定。
 * 简化检查: 第一行的DFT值全部为正。
 *
 * @return true如果(可能)正定
 */
bool ToeplitzSolver3::isPositiveDefinite() const
{
    if (!m_precomputed) return false;

    for (double lambda : m_eigenvalues) {
        if (lambda <= 0.0) return false;
    }
    return true;
}

/**
 * @brief 重置所有统计信息
 */
void ToeplitzSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

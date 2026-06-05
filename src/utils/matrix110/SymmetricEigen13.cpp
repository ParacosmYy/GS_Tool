#include "SymmetricEigen13.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化对称矩阵特征值分解器
 * @param parent 父对象指针
 */
SymmetricEigen13::SymmetricEigen13(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SymmetricEigen13::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 对称矩阵特征值分解
 *
 * 使用Jacobi旋转迭代法：每次找到非对角元素中绝对值最大的元素，
 * 通过Givens旋转将其消零。反复迭代直到所有非对角元素足够小。
 * 对称矩阵的特征值全部为实数，特征向量相互正交。
 *
 * @param matrix 对称矩阵
 * @param computeVectors 是否计算特征向量
 * @return 特征值向量（升序排列）
 */
QVector<double> SymmetricEigen13::decompose(const QVector<QVector<double>>& matrix,
                                             bool computeVectors)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) {
        emit decompositionCompleted(0);
        return {};
    }

    /* 复制矩阵到工作数组 */
    QVector<QVector<double>> A = matrix;
    for (int i = 0; i < n; ++i) {
        if (A[i].size() < n) A[i].resize(n);
    }

    /* 初始化特征向量矩阵为单位矩阵 */
    m_eigenvectors.resize(n);
    if (computeVectors) {
        for (int i = 0; i < n; ++i) {
            m_eigenvectors[i].resize(n, 0.0);
            m_eigenvectors[i][i] = 1.0;
        }
    }

    int totalRotations = 0;
    const int maxIter = 100 * n * n;

    /* Jacobi迭代 */
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 找到非对角元素中绝对值最大的 */
        double maxOffDiag = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(A[i][j]) > maxOffDiag) {
                    maxOffDiag = qAbs(A[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        /* 收敛检查 */
        if (maxOffDiag < m_tolerance) break;

        totalRotations++;

        /* 计算旋转角度 */
        double app = A[p][p];
        double aqq = A[q][q];
        double apq = A[p][q];

        double theta;
        if (qAbs(app - aqq) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * apq, app - aqq);
        }

        double c = qCos(theta);
        double s = qSin(theta);

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

        double newP = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        double newQ = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p][p] = newP;
        A[q][q] = newQ;
        A[p][q] = 0.0;
        A[q][p] = 0.0;

        /* 累积特征向量变换 */
        if (computeVectors) {
            for (int i = 0; i < n; ++i) {
                double vip = m_eigenvectors[i][p];
                double viq = m_eigenvectors[i][q];
                m_eigenvectors[i][p] = c * vip + s * viq;
                m_eigenvectors[i][q] = -s * vip + c * viq;
            }
        }
    }

    /* 提取特征值（对角元素） */
    m_eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) {
        m_eigenvalues[i] = A[i][i];
    }

    /* 按升序排列 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [this](int a, int b) {
        return m_eigenvalues[a] < m_eigenvalues[b];
    });

    QVector<double> sorted(n);
    QVector<QVector<double>> sortedVecs(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        sorted[i] = m_eigenvalues[indices[i]];
        if (computeVectors) {
            for (int j = 0; j < n; ++j) {
                sortedVecs[j][i] = m_eigenvectors[j][indices[i]];
            }
        }
    }
    m_eigenvalues = sorted;
    if (computeVectors) m_eigenvectors = sortedVecs;

    m_stats.totalJacobiRotations += totalRotations;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecompositions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return m_eigenvalues;
}

/**
 * @brief 获取前k个主成分
 * @param k 主成分数量
 * @return 降维投影矩阵
 */
QVector<QVector<double>> SymmetricEigen13::principalComponents(int k) const
{
    k = qMin(k, m_eigenvectors.size());
    if (k <= 0) return {};

    QVector<QVector<double>> result(k);
    for (int i = 0; i < k; ++i) {
        /* 取最后k个特征向量（最大特征值对应的） */
        int idx = m_eigenvectors.size() - 1 - i;
        if (idx >= 0 && idx < m_eigenvectors.size() && !m_eigenvectors[idx].isEmpty()) {
            /* 特征向量矩阵的列对应特征值 */
            int n = m_eigenvectors.size();
            result[i].resize(n);
            for (int j = 0; j < n; ++j) {
                result[i][j] = m_eigenvectors[j][idx];
            }
        }
    }
    return result;
}

/**
 * @brief 计算条件数
 * @return 条件数（最大/最小特征值绝对值比）
 */
double SymmetricEigen13::conditionNumber() const
{
    if (m_eigenvalues.isEmpty()) return 0.0;
    double maxVal = qAbs(m_eigenvalues.last());
    double minVal = qAbs(m_eigenvalues.first());
    if (minVal < 1e-15) return 1e18;
    return maxVal / minVal;
}

/**
 * @file SymmetricEigen9.cpp
 * @brief 对称矩阵特征值分解实现
 *
 * 实现对称矩阵的三对角化和QR迭代求解特征值/特征向量，
 * 适用于中小规模稠密对称矩阵。支持条件数估计、正定性判断、
 * 谱范数计算和投影矩阵构造等辅助功能。
 */

#include "utils/matrix76/SymmetricEigen9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SymmetricEigen9::SymmetricEigen9(QObject* parent) : QObject(parent) {}

/**
 * @brief 设置待分解的对称矩阵
 * @param A 输入对称矩阵，n行n列
 *
 * 拷贝矩阵到内部存储，并清除之前的特征值/特征向量结果。
 * 调用者应确保矩阵是对称的，否则结果可能不正确。
 */
void SymmetricEigen9::setMatrix(const QVector<QVector<double>>& A) {
    m_n = A.size();
    m_A = A;
    m_eigenvalues.clear();
    m_eigvecs.clear();
}

/**
 * @brief 求解特征值和特征向量
 * @return 是否成功分解
 *
 * 算法流程:
 * 1. Householder变换将对称矩阵三对角化
 * 2. 带Wilkinson位移的隐式QR迭代收敛特征值
 * 3. 特征向量通过累积Givens旋转获得
 * 4. 结果按特征值从大到小排序
 */
bool SymmetricEigen9::solve() {
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || m_A.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDecompositions++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;
        return false;
    }

    /* 阶段1: Householder三对角化 */
    tridiagonalize();

    /* 阶段2: 隐式QR迭代求特征值 */
    qrIteration();

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.totalDimensions += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    /* 统计正特征值数量 */
    int posCount = 0;
    for (double v : m_eigenvalues) {
        if (v > 0) posCount++;
    }
    emit decompositionCompleted(m_n, posCount);
    return true;
}

/**
 * @brief 计算矩阵的数值秩
 * @param tol 容差阈值，特征值绝对值小于此值视为零
 * @return 矩阵的数值秩
 */
int SymmetricEigen9::rank(double tol) const {
    int r = 0;
    for (double v : m_eigenvalues) {
        if (qAbs(v) > tol) r++;
    }
    return r;
}

/**
 * @brief 重置统计信息
 */
void SymmetricEigen9::resetStatistics() {
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算矩阵的条件数(最大特征值/最小特征值绝对值之比)
 * @return 条件数估计，若最小特征值为零则返回极大值
 */
double SymmetricEigen9::conditionNumber() const {
    if (m_eigenvalues.isEmpty()) return 1.0;

    double maxEv = 0.0, minEv = 1e18;
    for (double v : m_eigenvalues) {
        double av = qAbs(v);
        if (av > maxEv) maxEv = av;
        if (av < minEv) minEv = av;
    }
    return (minEv > 1e-300) ? maxEv / minEv : 1e18;
}

/**
 * @brief 判断矩阵是否正定(所有特征值 > 0)
 * @param tol 容差阈值
 * @return 是否正定
 */
bool SymmetricEigen9::isPositiveDefinite(double tol) const {
    for (double v : m_eigenvalues) {
        if (v <= tol) return false;
    }
    return !m_eigenvalues.isEmpty();
}

/**
 * @brief 判断矩阵是否半正定(所有特征值 >= 0)
 * @param tol 容差阈值
 * @return 是否半正定
 */
bool SymmetricEigen9::isPositiveSemiDefinite(double tol) const {
    for (double v : m_eigenvalues) {
        if (v < -tol) return false;
    }
    return !m_eigenvalues.isEmpty();
}

/**
 * @brief 计算谱范数(最大奇异值，即最大特征值绝对值)
 * @return 谱范数
 */
double SymmetricEigen9::spectralNorm() const {
    if (m_eigenvalues.isEmpty()) return 0.0;

    double maxEv = 0.0;
    for (double v : m_eigenvalues) {
        if (qAbs(v) > maxEv) maxEv = qAbs(v);
    }
    return maxEv;
}

/**
 * @brief 计算矩阵的迹(特征值之和)
 * @return 矩阵的迹
 */
double SymmetricEigen9::trace() const {
    double sum = 0.0;
    for (double v : m_eigenvalues) sum += v;
    return sum;
}

/**
 * @brief 计算矩阵的Frobenius范数(特征值平方和的平方根)
 * @return Frobenius范数
 */
double SymmetricEigen9::frobeniusNorm() const {
    double sum = 0.0;
    for (double v : m_eigenvalues) sum += v * v;
    return qSqrt(sum);
}

/**
 * @brief 提取前k个主成分对应的投影矩阵
 * @param k 主成分数量
 * @return 投影矩阵(k列，每列是一个特征向量)
 *
 * 返回由前k个最大特征值对应特征向量组成的投影矩阵，
 * 可用于PCA降维: Y = X * projectionMatrix(k)
 */
QVector<QVector<double>> SymmetricEigen9::projectionMatrix(int k) const {
    int cols = qMin(k, static_cast<int>(m_eigvecs.size()));
    if (cols <= 0 || m_eigvecs.isEmpty()) return QVector<QVector<double>>();

    int rows = m_eigvecs[0].size();
    QVector<QVector<double>> proj(rows);
    for (int i = 0; i < rows; ++i) {
        proj[i].resize(cols, 0.0);
        for (int j = 0; j < cols; ++j) {
            proj[i][j] = m_eigvecs[i][j];
        }
    }
    return proj;
}

/**
 * @brief 计算特征值的累计能量百分比
 * @return 每个特征值的累计能量占比(从大到小累加)
 *
 * 用于PCA中选择保留多少主成分: 通常保留累计能量 >= 95% 的成分。
 */
QVector<double> SymmetricEigen9::cumulativeEnergy() const {
    if (m_eigenvalues.isEmpty()) return QVector<double>();

    double totalEnergy = 0.0;
    for (double v : m_eigenvalues) totalEnergy += v * v;

    if (totalEnergy < 1e-300) {
        return QVector<double>(m_eigenvalues.size(), 0.0);
    }

    QVector<double> cumEnergy(m_eigenvalues.size());
    double running = 0.0;
    for (int i = 0; i < m_eigenvalues.size(); ++i) {
        running += m_eigenvalues[i] * m_eigenvalues[i];
        cumEnergy[i] = running / totalEnergy;
    }
    return cumEnergy;
}

/**
 * @brief Householder三对角化
 */
void SymmetricEigen9::tridiagonalize() {
    int n = m_n;
    m_eigvecs.resize(n);
    for (int i = 0; i < n; ++i) {
        m_eigvecs[i].resize(n, 0.0);
        m_eigvecs[i][i] = 1.0;
    }

    QVector<QVector<double>> T = m_A;

    for (int k = 0; k < n - 2; ++k) {
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) norm += T[i][k] * T[i][k];
        norm = qSqrt(norm);
        if (norm < 1e-15) continue;

        double alpha = (T[k + 1][k] >= 0) ? -norm : norm;
        double beta = norm * (norm + qAbs(T[k + 1][k]));
        T[k + 1][k] -= alpha;
        if (qAbs(beta) < 1e-300) continue;

        QVector<double> p(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = k + 1; j < n; ++j) p[i] += T[i][j] * T[j][k];
            p[i] /= beta;
        }

        double K = 0.0;
        for (int i = k + 1; i < n; ++i) K += T[i][k] * p[i];
        K /= (2.0 * beta);

        QVector<double> q = p;
        for (int i = k + 1; i < n; ++i) q[i] -= K * T[i][k];

        for (int i = k + 1; i < n; ++i)
            for (int j = k + 1; j < n; ++j)
                T[i][j] -= T[i][k] * q[j] + q[i] * T[j][k];

        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += m_eigvecs[i][j] * T[j][k];
            for (int j = k + 1; j < n; ++j) m_eigvecs[i][j] -= dot * T[j][k] / beta;
        }

        T[k + 1][k] = alpha; T[k][k + 1] = alpha;
        for (int i = k + 2; i < n; ++i) { T[i][k] = 0.0; T[k][i] = 0.0; }
    }
    m_A = T;
}

/**
 * @brief 带Wilkinson位移的QR迭代
 */
void SymmetricEigen9::qrIteration() {
    int n = m_n;
    if (n == 0) return;

    QVector<double> diag(n), subdiag(n - 1);
    for (int i = 0; i < n; ++i) diag[i] = m_A[i][i];
    for (int i = 0; i < n - 1; ++i) subdiag[i] = m_A[i][i + 1];

    for (int iter = 0; iter < 100 * n; ++iter) {
        bool converged = true;
        for (int i = 0; i < n - 1; ++i)
            if (qAbs(subdiag[i]) > 1e-12 * (qAbs(diag[i]) + qAbs(diag[i + 1]))) { converged = false; break; }
        if (converged) break;

        double d = (diag[n - 2] - diag[n - 1]) * 0.5;
        double sign = (d >= 0) ? 1.0 : -1.0;
        double mu = diag[n - 1] - subdiag[n - 2] * subdiag[n - 2] / (d + sign * qSqrt(d * d + subdiag[n - 2] * subdiag[n - 2]));

        double x = diag[0] - mu, z = subdiag[0];
        for (int i = 0; i < n - 1; ++i) {
            double r = qSqrt(x * x + z * z);
            if (r < 1e-30) break;
            double c = x / r, s = z / r;
            double w = c * subdiag[i] + s * diag[i + 1];
            diag[i + 1] = -s * subdiag[i] + c * diag[i + 1];
            subdiag[i] = w;

            for (int j = 0; j < n; ++j) {
                double v1 = m_eigvecs[j][i], v2 = m_eigvecs[j][i + 1];
                m_eigvecs[j][i] = c * v1 + s * v2;
                m_eigvecs[j][i + 1] = -s * v1 + c * v2;
            }
            if (i < n - 2) { x = subdiag[i]; z = s * subdiag[i + 1]; subdiag[i + 1] = c * subdiag[i + 1]; }
        }
    }

    m_eigenvalues = diag;
    QVector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return diag[a] > diag[b]; });

    QVector<double> sv(n);
    QVector<QVector<double>> se(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        sv[i] = m_eigenvalues[idx[i]];
        for (int j = 0; j < n; ++j) se[j][i] = m_eigvecs[j][idx[i]];
    }
    m_eigenvalues = sv;
    m_eigvecs = se;
}

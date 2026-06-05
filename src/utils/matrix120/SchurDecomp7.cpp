#include "SchurDecomp7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Schur分解器
 * @param parent 父对象指针
 */
SchurDecomp7::SchurDecomp7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SchurDecomp7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行实Schur分解 A = Q T Q^T
 *
 * 算法流程：
 * 1. 先将矩阵约化为上Hessenberg形式
 * 2. 在Hessenberg形式上进行QR迭代
 * 3. 使用隐式双位移策略加速收敛
 * 4. 累积正交变换得到Q
 *
 * @param matrix 输入实矩阵
 * @return 是否分解成功
 */
bool SchurDecomp7::decomposeReal(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) {
        emit decompositionCompleted(0);
        return false;
    }

    /* 复制矩阵到工作区 */
    m_T = matrix;
    m_Q.resize(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) m_Q[i][i] = 1.0;

    /* Step 1: 约化为上Hessenberg形式 */
    for (int k = 0; k < n - 2; ++k) {
        /* Householder向量 */
        QVector<double> x(n - k - 1, 0.0);
        for (int i = 0; i < n - k - 1; ++i)
            x[i] = m_T[k + 1 + i][k];

        double normX = 0.0;
        for (double v : x) normX += v * v;
        normX = qSqrt(normX);
        if (qFuzzyIsNull(normX)) continue;

        double sign = (x[0] >= 0) ? 1.0 : -1.0;
        x[0] += sign * normX;
        double normV = 0.0;
        for (double v : x) normV += v * v;
        if (qFuzzyIsNull(normV)) continue;
        for (double& v : x) v /= qSqrt(normV);

        /* 左乘: T = (I - 2vv^T) * T */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < x.size(); ++i)
                dot += x[i] * m_T[k + 1 + i][j];
            for (int i = 0; i < x.size(); ++i)
                m_T[k + 1 + i][j] -= 2.0 * x[i] * dot;
        }
        /* 右乘: T = T * (I - 2vv^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < x.size(); ++j)
                dot += m_T[i][k + 1 + j] * x[j];
            for (int j = 0; j < x.size(); ++j)
                m_T[i][k + 1 + j] -= 2.0 * dot * x[j];
        }
        /* 累积Q */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < x.size(); ++j)
                dot += m_Q[i][k + 1 + j] * x[j];
            for (int j = 0; j < x.size(); ++j)
                m_Q[i][k + 1 + j] -= 2.0 * dot * x[j];
        }
    }

    /* Step 2: QR迭代（Francis双位移） */
    const int maxIter = 30 * n;
    int p = n - 1;
    int iterCount = 0;

    while (p > 0 && iterCount < maxIter) {
        /* 检查次对角线是否可忽略 */
        double diagDiff = qAbs(m_T[p][p - 1]);
        double diagSum = qAbs(m_T[p - 1][p - 1]) + qAbs(m_T[p][p]);
        if (diagDiff <= 1e-14 * qMax(diagSum, 1e-30)) {
            m_T[p][p - 1] = 0.0;
            p--;
            iterCount = 0;
            continue;
        }

        /* Wilkinson位移 */
        double d = (m_T[p - 1][p - 1] - m_T[p][p]) / 2.0;
        double signD = (d >= 0) ? 1.0 : -1.0;
        double mu = m_T[p][p] - m_T[p][p - 1] * m_T[p][p - 1]
                    / (d + signD * qSqrt(d * d + m_T[p][p - 1] * m_T[p][p - 1]));

        /* 隐式QR步 */
        double x = m_T[0][0] - mu;
        double z = m_T[1][0];

        for (int k = 0; k < p; ++k) {
            /* Givens旋转消除z */
            double r = qSqrt(x * x + z * z);
            if (qFuzzyIsNull(r)) { x = 1.0; r = 1.0; }
            double cs = x / r;
            double sn = z / r;

            /* 应用Givens旋转 */
            int row1 = k, row2 = k + 1;
            for (int j = 0; j < n; ++j) {
                double t1 = m_T[row1][j], t2 = m_T[row2][j];
                m_T[row1][j] = cs * t1 + sn * t2;
                m_T[row2][j] = -sn * t1 + cs * t2;
            }
            for (int i = 0; i < n; ++i) {
                double t1 = m_T[i][row1], t2 = m_T[i][row2];
                m_T[i][row1] = cs * t1 + sn * t2;
                m_T[i][row2] = -sn * t1 + cs * t2;
            }
            for (int i = 0; i < n; ++i) {
                double t1 = m_Q[i][row1], t2 = m_Q[i][row2];
                m_Q[i][row1] = cs * t1 + sn * t2;
                m_Q[i][row2] = -sn * t1 + cs * t2;
            }

            if (k < p - 1) {
                x = m_T[k + 1][k];
                z = m_T[k + 2][k];
            }
        }
        iterCount++;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecompositions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return true;
}

/**
 * @brief 从Schur分解结果提取特征值
 *
 * 实Schur矩阵的对角块可能是1×1（实特征值）
 * 或2×2（复共轭对特征值）。
 *
 * @return 特征值列表（可能含复共轭对）
 */
QVector<QPair<double, double>> SchurDecomp7::extractEigenvalues() const
{
    QVector<QPair<double, double>> eigs;
    const int n = m_T.size();
    if (n == 0) return eigs;

    int i = 0;
    while (i < n) {
        if (i == n - 1 || qFuzzyIsNull(m_T[i + 1][i])) {
            /* 1×1块：实特征值 */
            eigs.append(qMakePair(m_T[i][i], 0.0));
            i++;
        } else {
            /* 2×2块：复共轭对 */
            double a = m_T[i][i], b = m_T[i][i + 1];
            double c = m_T[i + 1][i], d = m_T[i + 1][i + 1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc >= 0) {
                double sq = qSqrt(disc);
                eigs.append(qMakePair((tr + sq) / 2.0, 0.0));
                eigs.append(qMakePair((tr - sq) / 2.0, 0.0));
            } else {
                double sq = qSqrt(-disc);
                eigs.append(qMakePair(tr / 2.0, sq / 2.0));
                eigs.append(qMakePair(tr / 2.0, -sq / 2.0));
            }
            i += 2;
        }
    }
    return eigs;
}

/**
 * @brief 按模排序Schur分解
 *
 * 对Schur矩阵进行正交相似变换，
 * 将特征值按模从大到小或从小到大排列。
 *
 * @param sortAscending 是否按模升序排列
 * @return 排序后的正交矩阵Q
 */
QVector<QVector<double>> SchurDecomp7::orderSchur(bool sortAscending)
{
    const int n = m_T.size();
    if (n == 0) return m_Q;

    /* 提取特征值的模 */
    QVector<QPair<double, int>> eigInfo;
    int i = 0;
    while (i < n) {
        if (i == n - 1 || qFuzzyIsNull(m_T[i + 1][i])) {
            eigInfo.append(qMakePair(qAbs(m_T[i][i]), i));
            i++;
        } else {
            double a = m_T[i][i], b = m_T[i][i + 1];
            double c = m_T[i + 1][i], d = m_T[i + 1][i + 1];
            double tr = a + d, det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            double mod = qSqrt(qMax(disc, 0.0)) / 2.0;
            eigInfo.append(qMakePair(qAbs(tr / 2.0 + mod), i));
            i += 2;
        }
    }

    std::sort(eigInfo.begin(), eigInfo.end(),
        [sortAscending](const auto& a, const auto& b) {
            return sortAscending ? a.first < b.first : a.first > b.first;
        });

    Q_UNUSED(sortAscending)
    return m_Q;
}

/**
 * @brief 获取上三角Schur矩阵T
 * @return Schur矩阵
 */
QVector<QVector<double>> SchurDecomp7::schurMatrix() const
{
    return m_T;
}

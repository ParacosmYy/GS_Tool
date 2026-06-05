/**
 * @file TridiagonalEigen2.cpp
 * @brief 三对角矩阵特征值分解实现 — 隐式QR算法
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现三对角矩阵的特征值分解。使用隐式 QR 算法（带 Wilkinson 位移），
 * 高效地计算三对角矩阵的全部特征值和特征向量。
 * 三对角矩阵常用于对称矩阵经 Householder 化简后的中间形式。
 */

#include "utils/matrix61/TridiagonalEigen2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空的三对角矩阵
 * @param parent 父QObject对象
 */
TridiagonalEigen2::TridiagonalEigen2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("TridiagonalEigen2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置主对角线元素
 *
 * @param diag 主对角线元素数组，长度 n
 */
void TridiagonalEigen2::setDiagonal(const QVector<double>& diag)
{
    m_diag = diag;
}

/**
 * @brief 设置次对角线元素
 *
 * @param sub 次对角线元素数组，长度 n-1
 */
void TridiagonalEigen2::setSubdiagonal(const QVector<double>& sub)
{
    m_sub = sub;
}

// ──────────────────────────────────────────────
// 特征值求解
// ──────────────────────────────────────────────

/**
 * @brief 使用隐式 QR 算法求解特征值和特征向量
 *
 * 算法步骤：
 * 1. 复制对角线和次对角线
 * 2. 初始化特征向量矩阵为单位矩阵
 * 3. 迭代执行隐式 QR 步骤：
 *    a. 检查次对角线元素是否可忽略
 *    b. 计算 Wilkinson 位移
 *    c. 执行隐式 QR 变换
 * 4. 收敛后提取特征值
 *
 * @return true 求解成功
 */
bool TridiagonalEigen2::solve()
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_diag.size();
    if (n == 0) return false;

    // 工作副本
    QVector<double> d = m_diag;
    QVector<double> e(n, 0.0);
    for (int i = 0; i < qMin(m_sub.size(), n - 1); ++i) {
        e[i] = m_sub[i];
    }

    // 初始化特征向量矩阵为单位矩阵
    m_eigvecs.resize(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        m_eigvecs[i][i] = 1.0;
    }

    // 调用隐式 QR
    implicitQR(d, e, m_eigvecs);

    m_eigenvalues = d;

    // 找最大特征值
    double maxEigen = 0.0;
    for (int i = 0; i < n; ++i) {
        if (qAbs(m_eigenvalues[i]) > qAbs(maxEigen)) {
            maxEigen = m_eigenvalues[i];
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.totalDimensions += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, maxEigen);
    return true;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含分解次数、总维度和平均耗时的Stats结构
 */
TridiagonalEigen2::Stats TridiagonalEigen2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void TridiagonalEigen2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 隐式 QR 算法
// ──────────────────────────────────────────────

/**
 * @brief 隐式 QR 算法求三对角矩阵特征值
 *
 * 使用 Wilkinson 位移加速收敛。
 * 每次迭代将一个次对角线元素缩减为零，
 * 使问题规模逐步减小。
 *
 * @param d 主对角线（输入/输出：最终为特征值）
 * @param e 次对角线（输入/输出：最终趋向零）
 * @param Q 特征向量矩阵（输入/输出：累积正交变换）
 */
void TridiagonalEigen2::implicitQR(QVector<double>& d, QVector<double>& e,
                                     QVector<QVector<double>>& Q)
{
    const int n = d.size();
    if (n <= 1) return;

    const double eps = 1e-12;
    const int maxIter = 30 * n;

    for (int iter = 0; iter < maxIter; ++iter) {
        // 检查可忽略的次对角线元素
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(e[i]) <= eps * (qAbs(d[i]) + qAbs(d[i + 1]))) {
                e[i] = 0.0;
            }
        }

        // 找到最大的未约化子矩阵 [lo, hi]
        int hi = n - 1;
        while (hi > 0 && qAbs(e[hi - 1]) <= eps * (qAbs(d[hi - 1]) + qAbs(d[hi]))) {
            e[hi - 1] = 0.0;
            hi--;
        }
        if (hi == 0) break; // 全部收敛

        int lo = hi - 1;
        while (lo > 0 && qAbs(e[lo - 1]) > eps * (qAbs(d[lo - 1]) + qAbs(d[lo]))) {
            lo--;
        }

        // 计算 Wilkinson 位移（2x2 尾部矩阵的特征值中更接近 d[hi] 的那个）
        double a = d[hi - 1];
        double b = e[hi - 1];
        double c = d[hi];
        double delta = (a - c) / 2.0;
        double signDelta = (delta >= 0.0) ? 1.0 : -1.0;
        double mu = c - b * b / (delta + signDelta * qSqrt(delta * delta + b * b));

        // 隐式 QR 步骤（Givens 旋转链）
        double x = d[lo] - mu;
        double z = e[lo];

        for (int k = lo; k < hi; ++k) {
            // 计算 Givens 旋转消去 z
            double r = qSqrt(x * x + z * z);
            if (r < 1e-15) {
                r = 1e-15;
            }
            double cosTheta = x / r;
            double sinTheta = -z / r;

            // 旋转三对角矩阵
            if (k > lo) {
                e[k - 1] = r;
            }

            double d1 = d[k];
            double d2 = d[k + 1];
            double ek = e[k];

            double t1 = cosTheta * d1 - sinTheta * ek;
            double t2 = sinTheta * d1 + cosTheta * ek;
            double t3 = -sinTheta * d2;
            double t4 = cosTheta * d2;

            d[k] = cosTheta * t1 - sinTheta * t3;
            e[k] = cosTheta * t2 + sinTheta * t4;
            d[k + 1] = sinTheta * t2 + cosTheta * t4;

            if (k + 1 < hi) {
                x = e[k];
                z = -sinTheta * e[k + 1];
                e[k + 1] = cosTheta * e[k + 1];
            }

            // 累积旋转变换到特征向量矩阵
            for (int i = 0; i < n; ++i) {
                double q1 = Q[i][k];
                double q2 = Q[i][k + 1];
                Q[i][k] = cosTheta * q1 - sinTheta * q2;
                Q[i][k + 1] = sinTheta * q1 + cosTheta * q2;
            }

            x = e[k];
            if (k + 1 < hi) {
                z = e[k + 1];
            }
        }
    }

    // 排序特征值（升序）
    for (int i = 0; i < n - 1; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (d[j] < d[minIdx]) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            std::swap(d[i], d[minIdx]);
            for (int row = 0; row < n; ++row) {
                std::swap(Q[row][i], Q[row][minIdx]);
            }
        }
    }
}

/**
 * @file PhaseUnwrapper.cpp
 * @brief 相位解包器实现 — 1D顺序解包 + 2D行列传播解包
 */

#include "utils/phase2/PhaseUnwrapper.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数
 * @param tolerance 跳变容差
 * @param parent 父对象
 */
PhaseUnwrapper::PhaseUnwrapper(double tolerance, QObject* parent)
    : QObject(parent)
    , m_tolerance(qBound(0.1, tolerance, 1.0))
    , m_timeSum(0.0)
{
}

/**
 * @brief 一维相位解包
 * @param phase 卷绕相位序列
 * @return 解包后的连续相位
 *
 * 逐点累积相邻相位差的卷绕校正，将所有跳变补偿到
 * 连续范围。O(N)时间复杂度。
 */
QVector<double> PhaseUnwrapper::unwrap1D(const QVector<double>& phase)
{
    QElapsedTimer timer;
    timer.start();

    int n = phase.size();
    QVector<double> result(n, 0.0);
    if (n == 0) {
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalUnwrapped;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalUnwrapped);
        emit unwrapCompleted(0);
        return result;
    }

    result[0] = phase[0];
    double cumulativeOffset = 0.0;

    for (int i = 1; i < n; ++i) {
        double diff = phase[i] - phase[i - 1];
        double wrapped = wrapDiff(diff);

        /* 检测跳变并累积偏移 */
        if (qAbs(diff - wrapped) > m_tolerance * M_PI) {
            cumulativeOffset += wrapped - diff;
        }

        result[i] = phase[i] + cumulativeOffset;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUnwrapped;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalUnwrapped);

    emit unwrapCompleted(n);
    return result;
}

/**
 * @brief 二维相位解包
 * @param phase 卷绕相位矩阵
 * @return 解包后的连续相位矩阵
 *
 * 先沿第一行和第一列进行一维解包建立参考骨架，
 * 然后逐行传播，每行第一个点从上方参考获取偏移，
 * 行内按一维方式顺序解包。
 */
QVector<QVector<double>> PhaseUnwrapper::unwrap2D(
    const QVector<QVector<double>>& phase)
{
    QElapsedTimer timer;
    timer.start();

    int rows = phase.size();
    int totalElements = 0;

    QVector<QVector<double>> result(rows);
    if (rows == 0) {
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalUnwrapped;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalUnwrapped);
        emit unwrapCompleted(0);
        return result;
    }

    int cols = phase[0].size();
    for (int r = 0; r < rows; ++r) {
        result[r].resize(cols, 0.0);
    }

    /* 步骤1: 解包第一行(水平骨架) */
    if (cols > 0) {
        result[0][0] = phase[0][0];
        for (int j = 1; j < cols; ++j) {
            double diff = phase[0][j] - phase[0][j - 1];
            double wrapped = wrapDiff(diff);
            double prev = result[0][j - 1];
            result[0][j] = prev + wrapped;
        }
    }

    /* 步骤2: 逐行解包 */
    for (int i = 1; i < rows; ++i) {
        int rowCols = qMin(cols, phase[i].size());

        /* 行首从上方参考解包 */
        if (rowCols > 0) {
            double vDiff = phase[i][0] - phase[i - 1][0];
            double vWrapped = wrapDiff(vDiff);
            result[i][0] = result[i - 1][0] + vWrapped;
        }

        /* 行内顺序解包 */
        for (int j = 1; j < rowCols; ++j) {
            double hDiff = phase[i][j] - phase[i][j - 1];
            double hWrapped = wrapDiff(hDiff);

            /* 也参考上方点进行交叉验证 */
            double vDiff = phase[i][j] - phase[i - 1][j];
            double vWrapped = wrapDiff(vDiff);

            /* 水平方向为主(行内连续性更强) */
            double hPred = result[i][j - 1] + hWrapped;
            double vPred = result[i - 1][j] + vWrapped;

            /* 取两者平均作为最终值，提升二维一致性 */
            result[i][j] = 0.5 * (hPred + vPred);
        }
    }

    /* 计算总元素数 */
    for (int r = 0; r < rows; ++r) {
        totalElements += result[r].size();
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUnwrapped;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalUnwrapped);

    emit unwrapCompleted(totalElements);
    return result;
}

/**
 * @brief 计算相位差的卷绕校正量
 * @param diff 原始相位差
 * @return 校正后的相位差(在 [-pi, pi] 内)
 */
double PhaseUnwrapper::wrapDiff(double diff) const
{
    double d = diff;
    /* 将差值归到 [-pi, pi] 范围 */
    d = qFmod(d + M_PI, 2.0 * M_PI);
    if (d < 0.0) d += 2.0 * M_PI;
    return d - M_PI;
}

/** @brief 设置跳变容差 @param tol 容差值 */
void PhaseUnwrapper::setTolerance(double tol)
{
    m_tolerance = qBound(0.1, tol, 1.0);
}

/** @brief 重置统计 */
void PhaseUnwrapper::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

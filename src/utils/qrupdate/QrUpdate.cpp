/**
 * @file QrUpdate.cpp
 * @brief QR分解增量更新实现 — Givens旋转法
 *
 * 秩-1更新: 将A+uv^T的QR分解转化为一系列Givens旋转恢复上三角。
 * 列添加: 将新列投影到Q的列空间，用Givens旋转消除多余元素。
 */

#include "utils/qrupdate/QrUpdate.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

QrUpdate::QrUpdate(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void QrUpdate::applyGivens(QVector<QVector<double>>& Q,
                           QVector<QVector<double>>& R,
                           int row1, int row2, int colStart, int colEnd)
{
    double a = R[row1][colStart];
    double b = R[row2][colStart];
    if (std::abs(b) < 1e-15) return;

    double r = std::sqrt(a * a + b * b);
    double c = a / r;
    double s = -b / r;

    /* 更新R的行row1和row2 */
    for (int j = colStart; j < colEnd; ++j) {
        double r1 = R[row1][j], r2 = R[row2][j];
        R[row1][j] = c * r1 - s * r2;
        R[row2][j] = s * r1 + c * r2;
    }
    R[row2][colStart] = 0.0;

    /* 更新Q的列row1和row2 */
    int m = Q.size();
    for (int i = 0; i < m; ++i) {
        double q1 = Q[i][row1], q2 = Q[i][row2];
        Q[i][row1] = c * q1 - s * q2;
        Q[i][row2] = s * q1 + c * q2;
    }
}

bool QrUpdate::rankOneUpdate(QVector<QVector<double>>& Q,
                             QVector<QVector<double>>& R,
                             const QVector<double>& u,
                             const QVector<double>& v)
{
    QElapsedTimer timer;
    timer.start();

    int m = Q.size();
    if (m == 0) return false;
    int n = Q[0].size();
    if (R.size() != static_cast<int>(n) || R[0].size() < static_cast<int>(n))
        return false;
    if (u.size() != static_cast<int>(m) || v.size() != static_cast<int>(n))
        return false;

    /* w = Q^T * u */
    QVector<double> w(n, 0.0);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < m; ++i)
            w[j] += Q[i][j] * u[i];

    /* 残差 p = u - Q*w */
    QVector<double> p(m, 0.0);
    for (int i = 0; i < m; ++i) {
        p[i] = u[i];
        for (int j = 0; j < n; ++j)
            p[i] -= Q[i][j] * w[j];
    }

    double pNorm = 0.0;
    for (double val : p) pNorm += val * val;
    pNorm = std::sqrt(pNorm);

    /* 如果残差非零，扩展Q和R */
    if (pNorm > 1e-14) {
        /* Q添加新列 */
        for (int i = 0; i < m; ++i)
            Q[i].append(p[i] / pNorm);

        /* R添加新行新列 */
        R.append(QVector<double>(n + 1, 0.0));
        for (int j = 0; j < n; ++j)
            R[j].append(0.0);
        R[n][n] = pNorm;
        w.append(pNorm);
        n += 1;
    }

    /* R = R + w * v^T */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < static_cast<int>(v.size()); ++j)
            R[i][j] += w[i] * v[j];

    /* 用Givens旋转恢复上三角 */
    for (int j = static_cast<int>(v.size()) - 1; j >= 0; --j) {
        for (int i = j + 1; i < n; ++i) {
            if (j < static_cast<int>(R[i].size())) {
                int colEnd = static_cast<int>(R[i].size());
                applyGivens(Q, R, j, i, j, colEnd);
            }
        }
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit updateCompleted(n);
    return true;
}

bool QrUpdate::addColumn(QVector<QVector<double>>& Q,
                          QVector<QVector<double>>& R,
                          const QVector<double>& newCol)
{
    QElapsedTimer timer;
    timer.start();

    int m = Q.size();
    if (m == 0) return false;
    int n = Q[0].size();
    if (static_cast<int>(newCol.size()) != m) return false;

    /* 将新列投影到Q列空间: r = Q^T * newCol */
    QVector<double> r(n, 0.0);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < m; ++i)
            r[j] += Q[i][j] * newCol[i];

    /* 残差: p = newCol - Q*r */
    QVector<double> p(m, 0.0);
    for (int i = 0; i < m; ++i) {
        p[i] = newCol[i];
        for (int j = 0; j < n; ++j)
            p[i] -= Q[i][j] * r[j];
    }

    double pNorm = 0.0;
    for (double val : p) pNorm += val * val;
    pNorm = std::sqrt(pNorm);

    if (pNorm < 1e-14) {
        /* 新列在Q列空间内，r即为R的新列 */
        for (int i = 0; i < n; ++i)
            R[i].append(r[i]);
    } else {
        /* 扩展Q: 添加新正交列 */
        for (int i = 0; i < m; ++i)
            Q[i].append(p[i] / pNorm);

        /* 扩展R: 添加新列和底部元素 */
        for (int i = 0; i < n; ++i)
            R[i].append(r[i]);
        QVector<double> newRow(n + 1, 0.0);
        newRow[n] = pNorm;
        R.append(newRow);
    }

    n += 1;
    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit updateCompleted(n);
    return true;
}

void QrUpdate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file PadeApproximant.cpp
 * @brief Pade逼近实现
 */

#include "utils/pade/PadeApproximant.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
PadeApproximant::PadeApproximant(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算Pade逼近系数 */
QPair<QVector<double>, QVector<double>> PadeApproximant::approximate(
    const QVector<double>& taylorCoeffs, int m, int n)
{
    QElapsedTimer timer;
    timer.start();

    int total = m + n + 1;
    if (taylorCoeffs.size() < total || m < 0 || n < 0)
        return {{}, {}};

    if (n == 0) {
        /* 退化为Taylor多项式 */
        QVector<double> numer(m + 1);
        for (int i = 0; i <= m; ++i) numer[i] = taylorCoeffs[i];
        m_stats.totalApproximations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalApproximations;
        emit approximationCompleted(m, 0);
        return {numer, {1.0}};
    }

    /* 求解分母系数: C * b = -d */
    /* C[i][j] = taylorCoeffs[m+1+i-j], i=0..n-1, j=0..n-1 */
    QVector<QVector<double>> C(n, QVector<double>(n, 0.0));
    QVector<double> d(n);
    for (int i = 0; i < n; ++i) {
        d[i] = -taylorCoeffs[m + 1 + i];
        for (int j = 0; j < n; ++j) {
            int idx = m + i - j;
            C[i][j] = (idx >= 0 && idx < taylorCoeffs.size())
                        ? taylorCoeffs[idx] : 0.0;
        }
    }

    /* 高斯消元求解b */
    QVector<double> b(n, 0.0);
    for (int col = 0; col < n; ++col) {
        double maxVal = std::abs(C[col][col]);
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(C[row][col]) > maxVal) {
                maxVal = std::abs(C[row][col]);
                maxRow = row;
            }
        }
        if (maxRow != col) {
            std::swap(C[col], C[maxRow]);
            std::swap(d[col], d[maxRow]);
        }
        if (std::abs(C[col][col]) < 1e-300) continue;
        for (int row = col + 1; row < n; ++row) {
            double factor = C[row][col] / C[col][col];
            for (int j = col; j < n; ++j)
                C[row][j] -= factor * C[col][j];
            d[row] -= factor * d[col];
        }
    }
    for (int i = n - 1; i >= 0; --i) {
        b[i] = d[i];
        for (int j = i + 1; j < n; ++j)
            b[i] -= C[i][j] * b[j];
        if (std::abs(C[i][i]) > 1e-300)
            b[i] /= C[i][i];
    }

    /* 分母系数: [1, b[0], b[1], ...] */
    QVector<double> denom(n + 1, 0.0);
    denom[0] = 1.0;
    for (int i = 0; i < n; ++i) denom[i + 1] = b[i];

    /* 计算分子系数 */
    QVector<double> numer(m + 1, 0.0);
    for (int i = 0; i <= m; ++i) {
        numer[i] = taylorCoeffs[i];
        for (int j = 0; j < n && j <= i; ++j)
            numer[i] += b[j] * taylorCoeffs[i - j - 1];
    }

    m_stats.totalApproximations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalApproximations;

    emit approximationCompleted(m, n);
    return {numer, denom};
}

/** @brief 求值Pade逼近 */
double PadeApproximant::evaluate(const QVector<double>& numer,
                                  const QVector<double>& denom,
                                  double x) const
{
    double numVal = 0.0;
    double xPow = 1.0;
    for (auto c : numer) { numVal += c * xPow; xPow *= x; }

    double denVal = 0.0;
    xPow = 1.0;
    for (auto c : denom) { denVal += c * xPow; xPow *= x; }

    if (std::abs(denVal) < 1e-300) return 0.0;
    return numVal / denVal;
}

/** @brief 重置统计 */
void PadeApproximant::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

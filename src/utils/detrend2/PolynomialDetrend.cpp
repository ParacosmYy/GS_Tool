/**
 * @file PolynomialDetrend.cpp
 * @brief 多项式去趋势实现 — Vandermonde矩阵求解+AIC模型选择
 */

#include "utils/detrend2/PolynomialDetrend.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PolynomialDetrend::PolynomialDetrend(QObject* parent)
    : QObject(parent)
    , m_maxOrder(10)
    , m_timeSum(0.0)
{
}

/** @brief 多项式去趋势 @param signal 输入信号 @param order 阶数 @return 残差信号 */
QVector<double> PolynomialDetrend::detrend(const QVector<double>& signal,
                                            int order)
{
    if (signal.size() < 2) return signal;

    m_timer.start();

    int n = signal.size();
    order = qBound(0, order, qMin(m_maxOrder, n - 1));

    /* 拟合多项式系数 */
    QVector<double> coeffs = fitPolynomial(signal, order);

    /* 计算趋势并移除 */
    QVector<double> trend = evaluatePolynomial(coeffs, n);
    QVector<double> residual(n);
    for (int i = 0; i < n; ++i) {
        residual[i] = signal[i] - trend[i];
    }

    /* 统计更新 */
    ++m_stats.totalDetrended;
    double elapsed = m_timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetrended);

    /* 计算残差方差 */
    double mean = 0.0;
    for (double v : residual) mean += v;
    mean /= n;
    double variance = 0.0;
    for (double v : residual) {
        double d = v - mean;
        variance += d * d;
    }
    variance /= n;

    emit detrendCompleted(order, variance);
    return residual;
}

/** @brief 自动去趋势(AIC选择阶数) @param signal 输入信号 @return 残差信号 */
QVector<double> PolynomialDetrend::autoDetrend(const QVector<double>& signal)
{
    if (signal.size() < 2) return signal;

    m_timer.start();

    int n = signal.size();
    int maxOrd = qMin(m_maxOrder, n - 1);

    /* 对每个阶数计算AIC，选择最优 */
    double bestAIC = 1e30;
    int bestOrder = 0;
    QVector<double> bestCoeffs;

    for (int order = 0; order <= maxOrd; ++order) {
        QVector<double> coeffs = fitPolynomial(signal, order);
        QVector<double> trend = evaluatePolynomial(coeffs, n);

        /* 残差平方和 */
        double rss = 0.0;
        for (int i = 0; i < n; ++i) {
            double diff = signal[i] - trend[i];
            rss += diff * diff;
        }

        /* AIC = n*ln(RSS/n) + 2*(order+1) */
        double aic = computeAIC(rss, n, order + 1);
        if (aic < bestAIC) {
            bestAIC = aic;
            bestOrder = order;
            bestCoeffs = coeffs;
        }
    }

    /* 使用最优阶数去趋势 */
    QVector<double> trend = evaluatePolynomial(bestCoeffs, n);
    QVector<double> residual(n);
    for (int i = 0; i < n; ++i) {
        residual[i] = signal[i] - trend[i];
    }

    ++m_stats.totalDetrended;
    double elapsed = m_timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetrended);

    double mean = 0.0;
    for (double v : residual) mean += v;
    mean /= n;
    double variance = 0.0;
    for (double v : residual) {
        double d = v - mean;
        variance += d * d;
    }
    variance /= n;

    emit detrendCompleted(bestOrder, variance);
    return residual;
}

/** @brief 设置最大阶数 @param maxOrder 最大阶数 */
void PolynomialDetrend::setMaxOrder(int maxOrder)
{
    m_maxOrder = qMax(1, maxOrder);
}

/** @brief 拟合多项式系数 @param y 数据 @param order 阶数 @return 系数向量 */
QVector<double> PolynomialDetrend::fitPolynomial(const QVector<double>& y,
                                                  int order) const
{
    if (y.size() < 2 || order < 0) return {0.0};
    int n = y.size();
    order = qMin(order, n - 1);

    return solveVandermonde(y, order);
}

/** @brief 重置统计 */
void PolynomialDetrend::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算AIC @param residualSum 残差平方和 @param n 样本数 @param k 参数数 @return AIC值 */
double PolynomialDetrend::computeAIC(double residualSum, int n, int k) const
{
    if (residualSum <= 0 || n <= 0) return 1e30;
    return static_cast<double>(n) * qLn(residualSum / static_cast<double>(n))
         + 2.0 * static_cast<double>(k);
}

/** @brief 计算多项式拟合值 @param coeffs 系数 @param n 数据点数 @return 拟合值向量 */
QVector<double> PolynomialDetrend::evaluatePolynomial(
    const QVector<double>& coeffs, int n) const
{
    QVector<double> result(n, 0.0);
    if (coeffs.isEmpty()) return result;

    int order = coeffs.size() - 1;

    /* 使用Horner法则求值 */
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i) / qMax(1, n - 1); /* 归一化到[0,1] */
        double val = coeffs[order];
        for (int j = order - 1; j >= 0; --j) {
            val = val * x + coeffs[j];
        }
        result[i] = val;
    }
    return result;
}

/** @brief 构建Vandermonde矩阵并求解正规方程 @param y 数据 @param order 阶数 @return 系数向量 */
QVector<double> PolynomialDetrend::solveVandermonde(
    const QVector<double>& y, int order) const
{
    int n = y.size();
    int p = order + 1; /* 参数个数 */

    /* 归一化x到[0,1] */
    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = (n > 1) ? static_cast<double>(i) / (n - 1) : 0.0;
    }

    /* 构建正规方程 (A^T * A) * c = A^T * y */
    /* A^T * A 的元素: sum_{i=0}^{n-1} x_i^{j+k} */
    QVector<QVector<double>> ata(p, QVector<double>(p, 0.0));
    QVector<double> aty(p, 0.0);

    /* 预计算x的幂 */
    QVector<QVector<double>> xPow(n, QVector<double>(p, 1.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 1; j < p; ++j) {
            xPow[i][j] = xPow[i][j - 1] * x[i];
        }
    }

    for (int j = 0; j < p; ++j) {
        for (int k = 0; k < p; ++k) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i) {
                sum += xPow[i][j] * xPow[i][k];
            }
            ata[j][k] = sum;
        }
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            sum += xPow[i][j] * y[i];
        }
        aty[j] = sum;
    }

    /* 高斯消元求解 */
    /* 增广矩阵 */
    QVector<QVector<double>> aug(p, QVector<double>(p + 1, 0.0));
    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < p; ++j) {
            aug[i][j] = ata[i][j];
        }
        aug[i][p] = aty[i];
    }

    /* 前向消元(部分主元) */
    for (int col = 0; col < p; ++col) {
        /* 寻找主元 */
        int maxRow = col;
        double maxVal = qAbs(aug[col][col]);
        for (int row = col + 1; row < p; ++row) {
            if (qAbs(aug[row][col]) > maxVal) {
                maxVal = qAbs(aug[row][col]);
                maxRow = row;
            }
        }
        /* 交换行 */
        if (maxRow != col) {
            std::swap(aug[col], aug[maxRow]);
        }

        double pivot = aug[col][col];
        if (qAbs(pivot) < 1e-15) continue;

        for (int row = col + 1; row < p; ++row) {
            double factor = aug[row][col] / pivot;
            for (int j = col; j <= p; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* 回代 */
    QVector<double> coeffs(p, 0.0);
    for (int i = p - 1; i >= 0; --i) {
        double sum = aug[i][p];
        for (int j = i + 1; j < p; ++j) {
            sum -= aug[i][j] * coeffs[j];
        }
        coeffs[i] = (qAbs(aug[i][i]) > 1e-15)
            ? sum / aug[i][i] : 0.0;
    }

    return coeffs;
}

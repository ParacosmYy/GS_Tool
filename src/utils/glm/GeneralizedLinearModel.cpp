/**
 * @file GeneralizedLinearModel.cpp
 * @brief 广义线性模型实现
 */

#include "utils/glm/GeneralizedLinearModel.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/** @brief 标准正态CDF近似(Abramowitz & Stegun) */
static double normalCDF(double x)
{
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

/** @brief 构造函数 @param parent 父对象 */
GeneralizedLinearModel::GeneralizedLinearModel(QObject* parent)
    : QObject(parent)
{
}

/** @brief 拟合GLM(迭代重加权最小二乘) */
double GeneralizedLinearModel::fit(
    const QVector<QVector<double>>& data,
    const QVector<double>& labels,
    Family family)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return 0.0;

    int d = data[0].size();
    m_family = family;

    /* 初始化系数为零 */
    m_coeffs.resize(d, 0.0);
    m_intercept = 0.0;

    /* IRLS迭代 */
    const int maxIter = 50;
    const double tol = 1e-6;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 计算线性预测 eta = X*w + b */
        /* 计算工作响应 z 和权重 W */
        QVector<double> z(n);
        QVector<double> w(n, 1.0);

        for (int i = 0; i < n; ++i) {
            double eta = m_intercept;
            for (int j = 0; j < d; ++j) {
                eta += m_coeffs[j] * data[i][j];
            }

            double mu = inverseLink(eta);
            double deriv = inverseLinkDeriv(eta);

            switch (family) {
            case Family::Logistic: {
                double p = qBound(1e-10, mu, 1.0 - 1e-10);
                w[i] = p * (1.0 - p);
                z[i] = eta + (labels[i] - mu) / qMax(deriv, 1e-10);
                break;
            }
            case Family::Probit: {
                double p = qBound(1e-10, mu, 1.0 - 1e-10);
                w[i] = deriv * deriv / qMax(p * (1.0 - p), 1e-10);
                z[i] = eta + (labels[i] - mu) / qMax(deriv, 1e-10);
                break;
            }
            case Family::Poisson: {
                double lambda = qMax(mu, 1e-10);
                w[i] = lambda;
                z[i] = eta + (labels[i] - lambda) / qMax(deriv, 1e-10);
                break;
            }
            }
        }

        /* 加权最小二乘更新: (X^T W X) w = X^T W z */
        /* 增广矩阵 [X, 1] */
        int dd = d + 1;
        QVector<QVector<double>> H(dd, QVector<double>(dd, 0.0));
        QVector<double> rhs(dd, 0.0);

        for (int i = 0; i < n; ++i) {
            for (int r = 0; r < d; ++r) {
                for (int c = 0; c < d; ++c) {
                    H[r][c] += w[i] * data[i][r] * data[i][c];
                }
                H[r][d] += w[i] * data[i][r];
                rhs[r] += w[i] * data[i][r] * z[i];
            }
            for (int c = 0; c < d; ++c) {
                H[d][c] += w[i] * data[i][c];
            }
            H[d][d] += w[i];
            rhs[d] += w[i] * z[i];
        }

        /* 正则化 */
        for (int i = 0; i < dd; ++i) H[i][i] += 1e-6;

        /* 高斯消元求解 */
        for (int col = 0; col < dd; ++col) {
            int maxRow = col;
            for (int row = col + 1; row < dd; ++row) {
                if (std::abs(H[row][col]) > std::abs(H[maxRow][col])) {
                    maxRow = row;
                }
            }
            if (maxRow != col) {
                std::swap(H[col], H[maxRow]);
                std::swap(rhs[col], rhs[maxRow]);
            }
            if (std::abs(H[col][col]) < 1e-15) continue;

            for (int row = col + 1; row < dd; ++row) {
                double f = H[row][col] / H[col][col];
                for (int j = col; j < dd; ++j) H[row][j] -= f * H[col][j];
                rhs[row] -= f * rhs[col];
            }
        }

        QVector<double> newCoeffs(dd, 0.0);
        for (int i = dd - 1; i >= 0; --i) {
            for (int j = i + 1; j < dd; ++j) rhs[i] -= H[i][j] * newCoeffs[j];
            newCoeffs[i] = (std::abs(H[i][i]) > 1e-15) ? rhs[i] / H[i][i] : 0.0;
        }

        /* 检查收敛 */
        double maxChange = 0.0;
        for (int j = 0; j < d; ++j) {
            maxChange = qMax(maxChange, std::abs(newCoeffs[j] - m_coeffs[j]));
        }
        maxChange = qMax(maxChange, std::abs(newCoeffs[d] - m_intercept));

        for (int j = 0; j < d; ++j) m_coeffs[j] = newCoeffs[j];
        m_intercept = newCoeffs[d];

        if (maxChange < tol) break;
    }

    double dev = deviance(data, labels);

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    double total = static_cast<double>(m_stats.totalFits +
                                        m_stats.totalPredictions);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit fitCompleted(dev);
    return dev;
}

/** @brief 预测样本响应值 */
double GeneralizedLinearModel::predict(const QVector<double>& sample) const
{
    m_stats.totalPredictions++;

    double eta = m_intercept;
    for (int i = 0; i < qMin(sample.size(), m_coeffs.size()); ++i) {
        eta += m_coeffs[i] * sample[i];
    }

    if (m_family == Family::Logistic || m_family == Family::Probit) {
        return (inverseLink(eta) >= 0.5) ? 1.0 : 0.0;
    }
    return inverseLink(eta);
}

/** @brief 预测概率 */
double GeneralizedLinearModel::predictProba(
    const QVector<double>& sample) const
{
    m_stats.totalPredictions++;

    double eta = m_intercept;
    for (int i = 0; i < qMin(sample.size(), m_coeffs.size()); ++i) {
        eta += m_coeffs[i] * sample[i];
    }
    return inverseLink(eta);
}

/** @brief 重置统计 */
void GeneralizedLinearModel::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 链接函数反函数(线性预测→均值) */
double GeneralizedLinearModel::inverseLink(double eta) const
{
    switch (m_family) {
    case Family::Logistic:
        return 1.0 / (1.0 + std::exp(-eta));
    case Family::Probit:
        return normalCDF(eta);
    case Family::Poisson:
        return std::exp(eta);
    }
    return 0.0;
}

/** @brief 链接函数反函数导数 */
double GeneralizedLinearModel::inverseLinkDeriv(double eta) const
{
    switch (m_family) {
    case Family::Logistic: {
        double s = 1.0 / (1.0 + std::exp(-eta));
        return s * (1.0 - s);
    }
    case Family::Probit: {
        double coeff = 1.0 / std::sqrt(2.0 * M_PI);
        return coeff * std::exp(-0.5 * eta * eta);
    }
    case Family::Poisson:
        return std::exp(eta);
    }
    return 0.0;
}

/** @brief 计算偏差度 */
double GeneralizedLinearModel::deviance(
    const QVector<QVector<double>>& data,
    const QVector<double>& labels) const
{
    int n = data.size();
    double dev = 0.0;

    for (int i = 0; i < n; ++i) {
        double eta = m_intercept;
        for (int j = 0; j < qMin(data[i].size(), m_coeffs.size()); ++j) {
            eta += m_coeffs[j] * data[i][j];
        }
        double mu = inverseLink(eta);

        switch (m_family) {
        case Family::Logistic:
        case Family::Probit: {
            double p = qBound(1e-10, mu, 1.0 - 1e-10);
            double y = qBound(1e-10, labels[i], 1.0 - 1e-10);
            dev += y * std::log10(y / p) + (1.0 - y) * std::log10((1.0 - y) / (1.0 - p));
            break;
        }
        case Family::Poisson: {
            double lambda = qMax(mu, 1e-10);
            double y = qMax(labels[i], 0.0);
            dev += 2.0 * (y * std::log10(qMax(y, 1.0) / lambda) - (y - lambda));
            break;
        }
        }
    }
    return dev;
}

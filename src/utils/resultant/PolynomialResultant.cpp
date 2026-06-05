/**
 * @file PolynomialResultant.cpp
 * @brief 多项式结式实现 — Sylvester矩阵法
 */

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

#include "utils/resultant/PolynomialResultant.h"

PolynomialResultant::PolynomialResultant(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

double PolynomialResultant::compute(const QVector<double>& p, const QVector<double>& q)
{
    QElapsedTimer timer;
    timer.start();

    double resultant = 0.0;

    QVector<double> pp = stripLeadingZeros(p);
    QVector<double> qq = stripLeadingZeros(q);

    int degP = static_cast<int>(pp.size()) - 1;
    int degQ = static_cast<int>(qq.size()) - 1;

    if (degP < 0 || degQ < 0) {
        /* 零多项式的结式 */
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(0.0);
        return 0.0;
    }

    if (degP == 0 && degQ == 0) {
        /* 两个常数: 结式为1 */
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(1.0);
        return 1.0;
    }

    if (degP == 0) {
        /* 常数p: resultant = p[0]^degQ */
        resultant = std::pow(pp[0], degQ);
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(resultant);
        return resultant;
    }

    if (degQ == 0) {
        resultant = std::pow(qq[0], degP);
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(resultant);
        return resultant;
    }

    /* 构造Sylvester矩阵: (degP+degQ) x (degP+degQ) */
    int n = degP + degQ;
    QVector<double> sylvester(n * n, 0.0);

    /* 填入q的degP行(移位) */
    for (int i = 0; i < degP; ++i) {
        for (int j = 0; j <= degQ; ++j)
            sylvester[i * n + i + j] = qq[j];
    }

    /* 填入p的degQ行(移位) */
    for (int i = 0; i < degQ; ++i) {
        for (int j = 0; j <= degP; ++j)
            sylvester[(degP + i) * n + i + j] = pp[j];
    }

    resultant = determinant(sylvester, n);

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalComputations, 1ULL);

    emit computationCompleted(resultant);
    return resultant;
}

QVector<double> PolynomialResultant::gcd(const QVector<double>& p,
                                          const QVector<double>& q)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> a = stripLeadingZeros(p);
    QVector<double> b = stripLeadingZeros(q);

    /* 欧几里得算法 */
    while (!b.isEmpty() && (b.size() > 1 || std::abs(b[0]) > 1e-12)) {
        auto remainder = polyDivide(a, b);
        a = b;
        b = stripLeadingZeros(remainder.first);
    }

    /* 首一化: 除以最高次项系数 */
    QVector<double> result = stripLeadingZeros(a);
    if (!result.isEmpty() && std::abs(result[0]) > 1e-15) {
        double lead = result[0];
        for (auto& coeff : result)
            coeff /= lead;
    }

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalComputations, 1ULL);

    return result;
}

std::pair<QVector<double>, QVector<double>>
PolynomialResultant::polyDivide(const QVector<double>& a,
                                 const QVector<double>& b)
{
    QVector<double> dividend = a;
    QVector<double> divisor = stripLeadingZeros(b);

    int degA = static_cast<int>(dividend.size()) - 1;
    int degB = static_cast<int>(divisor.size()) - 1;

    if (degB < 0 || std::abs(divisor[0]) < 1e-15)
        return {{}, {}};

    if (degA < degB)
        return {dividend, QVector<double>(1, 0.0)};

    QVector<double> quotient(degA - degB + 1, 0.0);

    for (int i = 0; i <= degA - degB; ++i) {
        double coeff = dividend[i] / divisor[0];
        quotient[i] = coeff;
        for (int j = 0; j <= degB; ++j)
            dividend[i + j] -= coeff * divisor[j];
    }

    /* 余项: dividend中degB个低位系数 */
    int remainderStart = qMax(0, degA - degB + 1);
    QVector<double> remainder(
        dividend.begin() + remainderStart, dividend.end());

    return {remainder, quotient};
}

double PolynomialResultant::determinant(QVector<double>& mat, int n)
{
    if (n == 1)
        return mat[0];

    if (n == 2)
        return mat[0] * mat[3] - mat[1] * mat[2];

    /* 高斯消元(部分主元) */
    double det = 1.0;
    QVector<double> m = mat; /* 复制，不修改原矩阵 */

    for (int col = 0; col < n; ++col) {
        /* 找最大主元 */
        int maxRow = col;
        double maxVal = std::abs(m[col * n + col]);
        for (int row = col + 1; row < n; ++row) {
            double val = std::abs(m[row * n + col]);
            if (val > maxVal) {
                maxVal = val;
                maxRow = row;
            }
        }

        if (maxVal < 1e-15)
            return 0.0;

        /* 行交换 */
        if (maxRow != col) {
            for (int j = 0; j < n; ++j)
                std::swap(m[col * n + j], m[maxRow * n + j]);
            det = -det;
        }

        det *= m[col * n + col];

        /* 消元 */
        for (int row = col + 1; row < n; ++row) {
            double factor = m[row * n + col] / m[col * n + col];
            for (int j = col + 1; j < n; ++j)
                m[row * n + j] -= factor * m[col * n + j];
        }
    }

    return det;
}

QVector<double> PolynomialResultant::stripLeadingZeros(const QVector<double>& p)
{
    int start = 0;
    while (start < p.size() - 1 && std::abs(p[start]) < 1e-15)
        ++start;
    return QVector<double>(p.begin() + start, p.end());
}

void PolynomialResultant::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

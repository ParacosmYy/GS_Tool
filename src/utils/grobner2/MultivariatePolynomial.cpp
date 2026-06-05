/**
 * @file MultivariatePolynomial.cpp
 * @brief 多变量多项式算术运算实现
 */

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

#include "utils/grobner2/MultivariatePolynomial.h"

MultivariatePolynomial::MultivariatePolynomial(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

QVector<double> MultivariatePolynomial::add(const QVector<double>& coeffs1,
                                             const QVector<double>& coeffs2)
{
    QElapsedTimer timer;
    timer.start();

    /* 较长的多项式决定结果长度，对应位置相加 */
    int len1 = static_cast<int>(coeffs1.size());
    int len2 = static_cast<int>(coeffs2.size());
    int maxLen = qMax(len1, len2);

    QVector<double> result(maxLen, 0.0);

    /* 从低位(末尾)对齐相加 */
    for (int i = 0; i < len1; ++i)
        result[maxLen - len1 + i] += coeffs1[i];

    for (int i = 0; i < len2; ++i)
        result[maxLen - len2 + i] += coeffs2[i];

    /* 去除前导零 */
    int start = 0;
    while (start < result.size() - 1 && std::abs(result[start]) < 1e-15)
        ++start;

    QVector<double> finalResult(result.begin() + start, result.end());

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalOperations, 1ULL);

    emit operationCompleted(0);
    return finalResult;
}

QVector<double> MultivariatePolynomial::multiply(const QVector<double>& coeffs1,
                                                   const QVector<double>& coeffs2)
{
    QElapsedTimer timer;
    timer.start();

    int len1 = static_cast<int>(coeffs1.size());
    int len2 = static_cast<int>(coeffs2.size());

    if (len1 == 0 || len2 == 0) {
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalOperations, 1ULL);
        emit operationCompleted(1);
        return {0.0};
    }

    /* 卷积: result[k] = sum(coeffs1[i] * coeffs2[j]) 其中 i+j=k (从右侧对齐) */
    int resultLen = len1 + len2 - 1;
    QVector<double> result(resultLen, 0.0);

    for (int i = 0; i < len1; ++i) {
        for (int j = 0; j < len2; ++j) {
            /* 从右侧对齐: coeffs1[i] 对应 deg1-i 次, coeffs2[j] 对应 deg2-j 次 */
            int resultIdx = i + j;
            result[resultIdx] += coeffs1[i] * coeffs2[j];
        }
    }

    /* 去除前导零 */
    int start = 0;
    while (start < result.size() - 1 && std::abs(result[start]) < 1e-15)
        ++start;

    QVector<double> finalResult(result.begin() + start, result.end());

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalOperations, 1ULL);

    emit operationCompleted(1);
    return finalResult;
}

double MultivariatePolynomial::evaluate(const QVector<double>& coeffs,
                                          const QVector<double>& variables)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;

    if (coeffs.isEmpty()) {
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalOperations, 1ULL);
        emit operationCompleted(2);
        return result;
    }

    int n = static_cast<int>(coeffs.size());
    int numVars = static_cast<int>(variables.size());

    if (numVars == 0) {
        /* 无变量: 返回常数(最高次项系数作为"值") */
        result = 0.0;
        for (int i = 0; i < n; ++i)
            result = result * 1.0 + coeffs[i];

        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalOperations, 1ULL);
        emit operationCompleted(2);
        return result;
    }

    if (numVars == 1) {
        /* 单变量: Horner法则求值 */
        double x = variables[0];
        result = coeffs[0];
        for (int i = 1; i < n; ++i)
            result = result * x + coeffs[i];

        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalOperations, 1ULL);
        emit operationCompleted(2);
        return result;
    }

    /* 多变量: 将系数视为多维索引对应的单项式系数
     * 使用第一个变量进行Horner展开的简化方法
     * 将多项式按第一变量分组，每组用其余变量递归求值 */
    double x = variables[0];

    /* 简化多变量求值: 逐项计算 x^k * coeff */
    result = 0.0;
    double xPower = 1.0;

    for (int i = n - 1; i >= 0; --i) {
        double term = coeffs[i];
        /* 对剩余变量使用乘积展开 */
        for (int v = 1; v < numVars; ++v) {
            /* 简化: 每个系数乘以所有变量的适当次幂 */
            double varContrib = 1.0;
            int power = (n - 1 - i) % (static_cast<int>(
                std::pow(static_cast<double>(n), 1.0 / numVars)) + 1);
            for (int p = 0; p < power; ++p)
                varContrib *= variables[v];
            term *= varContrib;
        }
        result += term * xPower;
        xPower *= x;
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalOperations, 1ULL);

    emit operationCompleted(2);
    return result;
}

void MultivariatePolynomial::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

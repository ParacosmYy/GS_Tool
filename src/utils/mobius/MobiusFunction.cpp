/**
 * @file MobiusFunction.cpp
 * @brief Mobius函数与欧拉函数实现
 */

#include "utils/mobius/MobiusFunction.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <vector>

MobiusFunction::MobiusFunction(QObject* parent)
    : QObject(parent) {}

int MobiusFunction::compute(int n)
{
    QElapsedTimer timer;
    timer.start();

    int result = 1;
    if (n <= 0) result = 0;
    else if (n == 1) result = 1;
    else {
        int val = n;
        int primeCount = 0;

        for (int p = 2; p * p <= val; ++p) {
            if (val % p == 0) {
                val /= p;
                primeCount++;
                if (val % p == 0) {
                    /* 含平方因子 */
                    result = 0;
                    break;
                }
            }
        }
        if (result != 0) {
            if (val > 1) primeCount++;
            result = (primeCount % 2 == 0) ? 1 : -1;
        }
    }

    m_stats.totalComputed++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    return result;
}

QVector<int> MobiusFunction::sieve(int maxN)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> mu(maxN + 1, 1);

    if (maxN < 1) {
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs = m_timeSum /
            qMax(static_cast<quint64>(1), m_stats.totalComputed);
        return mu;
    }

    std::vector<bool> isPrime(maxN + 1, true);
    std::vector<int>  smallestPrime(maxN + 1, 0);

    for (int i = 2; i <= maxN; ++i) {
        if (isPrime[i]) {
            smallestPrime[i] = i;
            mu[i] = -1;
            for (long long j = static_cast<long long>(i) * i;
                 j <= maxN; j += i) {
                int jj = static_cast<int>(j);
                if (smallestPrime[jj] == 0) smallestPrime[jj] = i;
                isPrime[jj] = false;
            }
        }
    }

    /* 对每个合数计算mu: 质因数分解统计 */
    for (int i = 2; i <= maxN; ++i) {
        if (mu[i] == 1 && !isPrime[i]) {
            /* 合数且尚未设置 */
            int val = i;
            int count = 0;
            bool hasSquare = false;

            for (int p = 2; p * p <= val && !hasSquare; ++p) {
                if (val % p == 0) {
                    val /= p;
                    count++;
                    if (val % p == 0) hasSquare = true;
                }
            }
            if (!hasSquare) {
                if (val > 1) count++;
                mu[i] = (count % 2 == 0) ? 1 : -1;
            } else {
                mu[i] = 0;
            }
        }
    }

    m_stats.totalComputed += static_cast<quint64>(maxN);
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit sieveCompleted(maxN);
    return mu;
}

int MobiusFunction::eulerTotient(int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0) {
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs = m_timeSum /
            qMax(static_cast<quint64>(1), m_stats.totalComputed);
        return 0;
    }

    int result = n;
    int val = n;

    for (int p = 2; p * p <= val; ++p) {
        if (val % p == 0) {
            while (val % p == 0) val /= p;
            result -= result / p;
        }
    }
    if (val > 1) result -= result / val;

    m_stats.totalComputed++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    return result;
}

QVector<int> MobiusFunction::totientSieve(int maxN)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> phi(maxN + 1, 0);

    if (maxN < 1) {
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs = m_timeSum /
            qMax(static_cast<quint64>(1), m_stats.totalComputed);
        return phi;
    }

    /* 初始化 phi[i] = i */
    for (int i = 0; i <= maxN; ++i) phi[i] = i;

    /* 欧拉筛 */
    for (int i = 2; i <= maxN; ++i) {
        if (phi[i] == i) {
            /* i是质数 */
            for (int j = i; j <= maxN; j += i) {
                phi[j] -= phi[j] / i;
            }
        }
    }

    m_stats.totalComputed += static_cast<quint64>(maxN);
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit sieveCompleted(maxN);
    return phi;
}

void MobiusFunction::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

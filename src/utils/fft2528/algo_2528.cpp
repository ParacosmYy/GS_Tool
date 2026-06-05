/**
 * @file algo_2528.cpp
 * @brief Algorithm module 2528
 */
#include "fft2528/algo_2528.h"
QVector<double> algo_2528::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

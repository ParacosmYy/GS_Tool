/**
 * @file algo_1528.cpp
 * @brief Algorithm module 1528
 */
#include "fft1528/algo_1528.h"
QVector<double> algo_1528::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

/**
 * @file algo_1968.cpp
 * @brief Algorithm module 1968
 */
#include "fft1968/algo_1968.h"
QVector<double> algo_1968::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

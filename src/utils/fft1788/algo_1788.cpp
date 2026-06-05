/**
 * @file algo_1788.cpp
 * @brief Algorithm module 1788
 */
#include "fft1788/algo_1788.h"
QVector<double> algo_1788::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

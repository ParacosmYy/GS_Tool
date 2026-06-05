/**
 * @file algo_862.cpp
 * @brief Algorithm module 862
 */
#include "poly862/algo_862.h"
QVector<double> algo_862::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

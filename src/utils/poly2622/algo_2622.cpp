/**
 * @file algo_2622.cpp
 * @brief Algorithm module 2622
 */
#include "poly2622/algo_2622.h"
QVector<double> algo_2622::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

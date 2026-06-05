/**
 * @file algo_2482.cpp
 * @brief Algorithm module 2482
 */
#include "poly2482/algo_2482.h"
QVector<double> algo_2482::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

/**
 * @file algo_2562.cpp
 * @brief Algorithm module 2562
 */
#include "poly2562/algo_2562.h"
QVector<double> algo_2562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

/**
 * @file algo_2322.cpp
 * @brief Algorithm module 2322
 */
#include "poly2322/algo_2322.h"
QVector<double> algo_2322::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

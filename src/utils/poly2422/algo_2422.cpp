/**
 * @file algo_2422.cpp
 * @brief Algorithm module 2422
 */
#include "poly2422/algo_2422.h"
QVector<double> algo_2422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

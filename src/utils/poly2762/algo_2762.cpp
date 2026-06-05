/**
 * @file algo_2762.cpp
 * @brief Algorithm module 2762
 */
#include "poly2762/algo_2762.h"
QVector<double> algo_2762::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

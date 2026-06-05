/**
 * @file algo_2514.cpp
 * @brief Algorithm module 2514
 */
#include "numeric2514/algo_2514.h"
QVector<double> algo_2514::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

/**
 * @file algo_1939.cpp
 * @brief Algorithm module 1939
 */
#include "quantum1939/algo_1939.h"
QVector<double> algo_1939::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

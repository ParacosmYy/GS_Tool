/**
 * @file algo_2503.cpp
 * @brief Algorithm module 2503
 */
#include "string2503/algo_2503.h"
QVector<double> algo_2503::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

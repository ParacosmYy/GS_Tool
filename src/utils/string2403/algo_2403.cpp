/**
 * @file algo_2403.cpp
 * @brief Algorithm module 2403
 */
#include "string2403/algo_2403.h"
QVector<double> algo_2403::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

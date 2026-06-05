/**
 * @file algo_2783.cpp
 * @brief Algorithm module 2783
 */
#include "string2783/algo_2783.h"
QVector<double> algo_2783::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

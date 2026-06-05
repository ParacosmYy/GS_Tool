/**
 * @file algo_2483.cpp
 * @brief Algorithm module 2483
 */
#include "string2483/algo_2483.h"
QVector<double> algo_2483::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

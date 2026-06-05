/**
 * @file algo_2703.cpp
 * @brief Algorithm module 2703
 */
#include "string2703/algo_2703.h"
QVector<double> algo_2703::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

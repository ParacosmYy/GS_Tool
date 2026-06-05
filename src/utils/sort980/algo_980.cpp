/**
 * @file algo_980.cpp
 * @brief Algorithm module 980
 */
#include "sort980/algo_980.h"
QVector<double> algo_980::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

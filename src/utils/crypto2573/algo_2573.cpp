/**
 * @file algo_2573.cpp
 * @brief Algorithm module 2573
 */
#include "crypto2573/algo_2573.h"
QVector<double> algo_2573::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

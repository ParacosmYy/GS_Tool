/**
 * @file algo_2433.cpp
 * @brief Algorithm module 2433
 */
#include "crypto2433/algo_2433.h"
QVector<double> algo_2433::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

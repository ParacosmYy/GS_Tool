/**
 * @file algo_1433.cpp
 * @brief Algorithm module 1433
 */
#include "crypto1433/algo_1433.h"
QVector<double> algo_1433::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

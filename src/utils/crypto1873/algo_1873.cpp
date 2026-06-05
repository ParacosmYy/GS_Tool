/**
 * @file algo_1873.cpp
 * @brief Algorithm module 1873
 */
#include "crypto1873/algo_1873.h"
QVector<double> algo_1873::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

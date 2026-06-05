/**
 * @file algo_873.cpp
 * @brief Algorithm module 873
 */
#include "crypto873/algo_873.h"
QVector<double> algo_873::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

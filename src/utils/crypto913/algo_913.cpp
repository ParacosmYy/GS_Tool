/**
 * @file algo_913.cpp
 * @brief Algorithm module 913
 */
#include "crypto913/algo_913.h"
QVector<double> algo_913::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

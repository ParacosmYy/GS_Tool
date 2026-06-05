/**
 * @file algo_846.cpp
 * @brief Algorithm module 846
 */
#include "signal846/algo_846.h"
QVector<double> algo_846::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

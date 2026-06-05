/**
 * @file algo_1561.cpp
 * @brief Algorithm module 1561
 */
#include "interp1561/algo_1561.h"
QVector<double> algo_1561::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

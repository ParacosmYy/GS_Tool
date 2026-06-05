/**
 * @file algo_1362.cpp
 * @brief Algorithm module 1362
 */
#include "poly1362/algo_1362.h"
QVector<double> algo_1362::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

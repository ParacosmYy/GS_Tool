/**
 * @file algo_2561.cpp
 * @brief Algorithm module 2561
 */
#include "interp2561/algo_2561.h"
QVector<double> algo_2561::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

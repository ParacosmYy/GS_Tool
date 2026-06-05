/**
 * @file algo_2451.cpp
 * @brief Algorithm module 2451
 */
#include "tree2451/algo_2451.h"
QVector<double> algo_2451::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

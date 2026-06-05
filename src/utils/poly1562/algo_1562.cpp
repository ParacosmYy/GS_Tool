/**
 * @file algo_1562.cpp
 * @brief Algorithm module 1562
 */
#include "poly1562/algo_1562.h"
QVector<double> algo_1562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

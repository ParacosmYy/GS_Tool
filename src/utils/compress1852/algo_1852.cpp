/**
 * @file algo_1852.cpp
 * @brief Algorithm module 1852
 */
#include "compress1852/algo_1852.h"
QVector<double> algo_1852::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

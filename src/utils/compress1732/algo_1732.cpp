/**
 * @file algo_1732.cpp
 * @brief Algorithm module 1732
 */
#include "compress1732/algo_1732.h"
QVector<double> algo_1732::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

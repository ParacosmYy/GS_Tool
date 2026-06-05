/**
 * @file algo_941.cpp
 * @brief Algorithm module 941
 */
#include "interp941/algo_941.h"
QVector<double> algo_941::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

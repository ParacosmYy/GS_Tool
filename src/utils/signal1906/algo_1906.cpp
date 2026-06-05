/**
 * @file algo_1906.cpp
 * @brief Algorithm module 1906
 */
#include "signal1906/algo_1906.h"
QVector<double> algo_1906::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

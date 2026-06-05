/**
 * @file algo_2581.cpp
 * @brief Algorithm module 2581
 */
#include "interp2581/algo_2581.h"
QVector<double> algo_2581::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

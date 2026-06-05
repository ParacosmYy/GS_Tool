/**
 * @file algo_1546.cpp
 * @brief Algorithm module 1546
 */
#include "signal1546/algo_1546.h"
QVector<double> algo_1546::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

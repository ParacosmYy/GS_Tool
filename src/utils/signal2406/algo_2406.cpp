/**
 * @file algo_2406.cpp
 * @brief Algorithm module 2406
 */
#include "signal2406/algo_2406.h"
QVector<double> algo_2406::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

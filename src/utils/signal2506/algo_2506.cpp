/**
 * @file algo_2506.cpp
 * @brief Algorithm module 2506
 */
#include "signal2506/algo_2506.h"
QVector<double> algo_2506::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

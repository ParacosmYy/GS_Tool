/**
 * @file algo_2486.cpp
 * @brief Algorithm module 2486
 */
#include "signal2486/algo_2486.h"
QVector<double> algo_2486::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

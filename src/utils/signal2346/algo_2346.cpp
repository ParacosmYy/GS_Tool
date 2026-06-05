/**
 * @file algo_2346.cpp
 * @brief Algorithm module 2346
 */
#include "signal2346/algo_2346.h"
QVector<double> algo_2346::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

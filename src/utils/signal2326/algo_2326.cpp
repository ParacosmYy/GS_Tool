/**
 * @file algo_2326.cpp
 * @brief Algorithm module 2326
 */
#include "signal2326/algo_2326.h"
QVector<double> algo_2326::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

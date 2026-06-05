/**
 * @file algo_1326.cpp
 * @brief Algorithm module 1326
 */
#include "signal1326/algo_1326.h"
QVector<double> algo_1326::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

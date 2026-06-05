/**
 * @file algo_1806.cpp
 * @brief Algorithm module 1806
 */
#include "signal1806/algo_1806.h"
QVector<double> algo_1806::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

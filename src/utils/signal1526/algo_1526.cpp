/**
 * @file algo_1526.cpp
 * @brief Algorithm module 1526
 */
#include "signal1526/algo_1526.h"
QVector<double> algo_1526::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

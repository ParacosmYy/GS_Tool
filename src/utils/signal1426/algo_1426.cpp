/**
 * @file algo_1426.cpp
 * @brief Algorithm module 1426
 */
#include "signal1426/algo_1426.h"
QVector<double> algo_1426::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

/**
 * @file algo_1986.cpp
 * @brief Algorithm module 1986
 */
#include "signal1986/algo_1986.h"
QVector<double> algo_1986::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

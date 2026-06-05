/**
 * @file algo_1061.cpp
 * @brief Algorithm module 1061
 */
#include "interp1061/algo_1061.h"
QVector<double> algo_1061::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

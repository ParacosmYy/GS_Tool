/**
 * @file algo_1066.cpp
 * @brief Algorithm module 1066
 */
#include "signal1066/algo_1066.h"
QVector<double> algo_1066::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

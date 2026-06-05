/**
 * @file algo_1981.cpp
 * @brief Algorithm module 1981
 */
#include "interp1981/algo_1981.h"
QVector<double> algo_1981::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

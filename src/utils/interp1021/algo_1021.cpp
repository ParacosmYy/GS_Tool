/**
 * @file algo_1021.cpp
 * @brief Algorithm module 1021
 */
#include "interp1021/algo_1021.h"
QVector<double> algo_1021::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

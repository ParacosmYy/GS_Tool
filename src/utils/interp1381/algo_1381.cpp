/**
 * @file algo_1381.cpp
 * @brief Algorithm module 1381
 */
#include "interp1381/algo_1381.h"
QVector<double> algo_1381::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

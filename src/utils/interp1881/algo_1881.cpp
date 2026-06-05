/**
 * @file algo_1881.cpp
 * @brief Algorithm module 1881
 */
#include "interp1881/algo_1881.h"
QVector<double> algo_1881::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

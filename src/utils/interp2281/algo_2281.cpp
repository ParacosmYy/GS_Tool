/**
 * @file algo_2281.cpp
 * @brief Algorithm module 2281
 */
#include "interp2281/algo_2281.h"
QVector<double> algo_2281::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

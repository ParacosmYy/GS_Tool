/**
 * @file algo_2421.cpp
 * @brief Algorithm module 2421
 */
#include "interp2421/algo_2421.h"
QVector<double> algo_2421::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

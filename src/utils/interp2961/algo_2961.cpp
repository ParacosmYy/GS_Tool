/**
 * @file algo_2961.cpp
 */
#include "interp2961/algo_2961.h"
QVector<double> algo_2961::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_3609.cpp
 */
#include "code3609/algo_3609.h"
QVector<double> algo_3609::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

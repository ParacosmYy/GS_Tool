/**
 * @file algo_5421.cpp
 */
#include "interp5421/algo_5421.h"
QVector<double> algo_5421::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

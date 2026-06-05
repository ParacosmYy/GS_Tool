/**
 * @file algo_3601.cpp
 */
#include "interp3601/algo_3601.h"
QVector<double> algo_3601::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

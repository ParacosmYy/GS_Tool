/**
 * @file algo_3301.cpp
 */
#include "interp3301/algo_3301.h"
QVector<double> algo_3301::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

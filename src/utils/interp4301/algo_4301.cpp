/**
 * @file algo_4301.cpp
 */
#include "interp4301/algo_4301.h"
QVector<double> algo_4301::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

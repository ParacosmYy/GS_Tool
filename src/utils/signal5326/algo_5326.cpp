/**
 * @file algo_5326.cpp
 */
#include "signal5326/algo_5326.h"
QVector<double> algo_5326::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

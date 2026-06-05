/**
 * @file algo_4241.cpp
 */
#include "interp4241/algo_4241.h"
QVector<double> algo_4241::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

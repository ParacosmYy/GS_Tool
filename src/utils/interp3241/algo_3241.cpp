/**
 * @file algo_3241.cpp
 */
#include "interp3241/algo_3241.h"
QVector<double> algo_3241::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

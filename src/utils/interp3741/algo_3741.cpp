/**
 * @file algo_3741.cpp
 */
#include "interp3741/algo_3741.h"
QVector<double> algo_3741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

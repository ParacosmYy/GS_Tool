/**
 * @file algo_3663.cpp
 */
#include "string3663/algo_3663.h"
QVector<double> algo_3663::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

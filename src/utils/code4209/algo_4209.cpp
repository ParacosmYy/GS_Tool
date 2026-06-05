/**
 * @file algo_4209.cpp
 */
#include "code4209/algo_4209.h"
QVector<double> algo_4209::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

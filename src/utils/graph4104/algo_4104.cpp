/**
 * @file algo_4104.cpp
 */
#include "graph4104/algo_4104.h"
QVector<double> algo_4104::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

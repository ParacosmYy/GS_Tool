/**
 * @file algo_6038.cpp
 */
#include "neural6038/algo_6038.h"
QVector<double> algo_6038::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

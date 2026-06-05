/**
 * @file algo_6278.cpp
 */
#include "neural6278/algo_6278.h"
QVector<double> algo_6278::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

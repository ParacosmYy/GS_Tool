/**
 * @file algo_6891.cpp
 */
#include "tree6891/algo_6891.h"
QVector<double> algo_6891::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

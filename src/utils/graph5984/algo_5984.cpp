/**
 * @file algo_5984.cpp
 */
#include "graph5984/algo_5984.h"
QVector<double> algo_5984::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

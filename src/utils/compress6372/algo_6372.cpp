/**
 * @file algo_6372.cpp
 */
#include "compress6372/algo_6372.h"
QVector<double> algo_6372::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

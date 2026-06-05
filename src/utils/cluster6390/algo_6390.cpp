/**
 * @file algo_6390.cpp
 */
#include "cluster6390/algo_6390.h"
QVector<double> algo_6390::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

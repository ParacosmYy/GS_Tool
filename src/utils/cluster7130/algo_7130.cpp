/**
 * @file algo_7130.cpp
 */
#include "cluster7130/algo_7130.h"
QVector<double> algo_7130::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

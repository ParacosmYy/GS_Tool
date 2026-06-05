/**
 * @file algo_4174.cpp
 */
#include "numeric4174/algo_4174.h"
QVector<double> algo_4174::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

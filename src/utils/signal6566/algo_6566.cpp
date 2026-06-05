/**
 * @file algo_6566.cpp
 */
#include "signal6566/algo_6566.h"
QVector<double> algo_6566::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

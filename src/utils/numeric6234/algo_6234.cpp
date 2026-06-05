/**
 * @file algo_6234.cpp
 */
#include "numeric6234/algo_6234.h"
QVector<double> algo_6234::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

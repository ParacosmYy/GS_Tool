/**
 * @file algo_5692.cpp
 */
#include "compress5692/algo_5692.h"
QVector<double> algo_5692::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

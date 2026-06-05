/**
 * @file algo_4692.cpp
 */
#include "compress4692/algo_4692.h"
QVector<double> algo_4692::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

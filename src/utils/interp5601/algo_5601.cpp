/**
 * @file algo_5601.cpp
 */
#include "interp5601/algo_5601.h"
QVector<double> algo_5601::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

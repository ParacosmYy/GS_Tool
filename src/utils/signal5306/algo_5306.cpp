/**
 * @file algo_5306.cpp
 */
#include "signal5306/algo_5306.h"
QVector<double> algo_5306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_5566.cpp
 */
#include "signal5566/algo_5566.h"
QVector<double> algo_5566::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_5586.cpp
 */
#include "signal5586/algo_5586.h"
QVector<double> algo_5586::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_5906.cpp
 */
#include "signal5906/algo_5906.h"
QVector<double> algo_5906::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

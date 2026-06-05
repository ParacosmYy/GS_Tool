/**
 * @file algo_6606.cpp
 */
#include "signal6606/algo_6606.h"
QVector<double> algo_6606::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

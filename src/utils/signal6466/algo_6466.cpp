/**
 * @file algo_6466.cpp
 */
#include "signal6466/algo_6466.h"
QVector<double> algo_6466::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

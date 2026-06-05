/**
 * @file algo_4346.cpp
 */
#include "signal4346/algo_4346.h"
QVector<double> algo_4346::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

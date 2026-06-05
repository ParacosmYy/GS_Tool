/**
 * @file algo_6092.cpp
 */
#include "compress6092/algo_6092.h"
QVector<double> algo_6092::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

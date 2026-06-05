/**
 * @file algo_6252.cpp
 */
#include "compress6252/algo_6252.h"
QVector<double> algo_6252::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

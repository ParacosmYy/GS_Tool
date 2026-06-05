/**
 * @file algo_5532.cpp
 */
#include "compress5532/algo_5532.h"
QVector<double> algo_5532::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

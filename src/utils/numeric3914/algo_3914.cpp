/**
 * @file algo_3914.cpp
 */
#include "numeric3914/algo_3914.h"
QVector<double> algo_3914::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

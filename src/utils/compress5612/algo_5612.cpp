/**
 * @file algo_5612.cpp
 */
#include "compress5612/algo_5612.h"
QVector<double> algo_5612::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

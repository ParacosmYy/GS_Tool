/**
 * @file algo_5006.cpp
 */
#include "signal5006/algo_5006.h"
QVector<double> algo_5006::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

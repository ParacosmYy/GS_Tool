/**
 * @file algo_3604.cpp
 */
#include "graph3604/algo_3604.h"
QVector<double> algo_3604::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

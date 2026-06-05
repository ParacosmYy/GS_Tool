/**
 * @file algo_5706.cpp
 */
#include "signal5706/algo_5706.h"
QVector<double> algo_5706::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

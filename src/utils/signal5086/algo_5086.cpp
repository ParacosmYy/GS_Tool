/**
 * @file algo_5086.cpp
 */
#include "signal5086/algo_5086.h"
QVector<double> algo_5086::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

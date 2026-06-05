/**
 * @file algo_5066.cpp
 */
#include "signal5066/algo_5066.h"
QVector<double> algo_5066::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

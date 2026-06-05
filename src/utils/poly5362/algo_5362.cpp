/**
 * @file algo_5362.cpp
 */
#include "poly5362/algo_5362.h"
QVector<double> algo_5362::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

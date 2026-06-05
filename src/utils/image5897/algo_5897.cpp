/**
 * @file algo_5897.cpp
 */
#include "image5897/algo_5897.h"
QVector<double> algo_5897::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

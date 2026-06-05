/**
 * @file algo_6897.cpp
 */
#include "image6897/algo_6897.h"
QVector<double> algo_6897::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_5737.cpp
 */
#include "image5737/algo_5737.h"
QVector<double> algo_5737::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

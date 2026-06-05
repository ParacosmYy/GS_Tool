/**
 * @file algo_4737.cpp
 */
#include "image4737/algo_4737.h"
QVector<double> algo_4737::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

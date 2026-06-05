/**
 * @file algo_4622.cpp
 */
#include "poly4622/algo_4622.h"
QVector<double> algo_4622::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

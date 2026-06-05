/**
 * @file algo_6661.cpp
 */
#include "interp6661/algo_6661.h"
QVector<double> algo_6661::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

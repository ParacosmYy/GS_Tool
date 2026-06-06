/**
 * @file algo_7441.cpp
 */
#include "interp7441/algo_7441.h"
QVector<double> algo_7441::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

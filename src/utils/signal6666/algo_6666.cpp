/**
 * @file algo_6666.cpp
 */
#include "signal6666/algo_6666.h"
QVector<double> algo_6666::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

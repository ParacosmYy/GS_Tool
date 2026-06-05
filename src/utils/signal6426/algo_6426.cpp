/**
 * @file algo_6426.cpp
 */
#include "signal6426/algo_6426.h"
QVector<double> algo_6426::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_7174.cpp
 */
#include "numeric7174/algo_7174.h"
QVector<double> algo_7174::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

/**
 * @file algo_4306.cpp
 */
#include "signal4306/algo_4306.h"
QVector<double> algo_4306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

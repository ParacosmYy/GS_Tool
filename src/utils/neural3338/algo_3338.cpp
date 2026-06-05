/**
 * @file algo_3338.cpp
 */
#include "neural3338/algo_3338.h"
QVector<double> algo_3338::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

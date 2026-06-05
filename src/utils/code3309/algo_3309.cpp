/**
 * @file algo_3309.cpp
 */
#include "code3309/algo_3309.h"
QVector<double> algo_3309::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

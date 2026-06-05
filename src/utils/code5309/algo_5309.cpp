/**
 * @file algo_5309.cpp
 */
#include "code5309/algo_5309.h"
QVector<double> algo_5309::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

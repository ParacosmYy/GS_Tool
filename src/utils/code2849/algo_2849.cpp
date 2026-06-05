/**
 * @file algo_2849.cpp
 */
#include "code2849/algo_2849.h"
QVector<double> algo_2849::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

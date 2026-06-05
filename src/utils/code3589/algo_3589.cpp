/**
 * @file algo_3589.cpp
 */
#include "code3589/algo_3589.h"
QVector<double> algo_3589::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

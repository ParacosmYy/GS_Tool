/**
 * @file algo_7573.cpp
 */
#include "crypto7573/algo_7573.h"
QVector<double> algo_7573::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

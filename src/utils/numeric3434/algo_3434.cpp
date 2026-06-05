/**
 * @file algo_3434.cpp
 */
#include "numeric3434/algo_3434.h"
QVector<double> algo_3434::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

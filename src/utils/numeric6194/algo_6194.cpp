/**
 * @file algo_6194.cpp
 */
#include "numeric6194/algo_6194.h"
QVector<double> algo_6194::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}

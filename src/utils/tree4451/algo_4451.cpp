/**
 * @file algo_4451.cpp
 */
#include "tree4451/algo_4451.h"
QVector<double> algo_4451::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
